#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "wifi_credentials.h" 
#include <Servo.h>

// uncomment for use of websocket communication
// used to send additional info (while running) to browser, needs stable internet connection
#define WEBSOCKET

#define LED1 2
#define LED2 16
// top view, right side at antenna, 2nd pin from top (D1)
#define SERVOPIN 5

// using this for the aerostar rvs 80a ESC
// 0/stop/arm = 1000, max=1900 (?)
Servo servo;
// stop/signal to arm
#define SERVOMIN 1000
// max full load, > than SERVOMIN
#define SERVOMAX 1900

ESP8266WebServer server(80);
#ifdef WEBSOCKET
#include "websocket.h"
#else
String generateWebSocketHtml(bool append) {return "";}
#endif

const char* hostname1 = "motorloader";
// 12000: Variables and constants in RAM (global, static), used 77592 / 80192 bytes (96%)
// doesn't run like that, needs more space
// ATTENTION: there is some limit on transferred data with the post requests, too precise float numbers lead to not all values being read
const int MAX_VALUES = 4000;
float csvValues[MAX_VALUES];
int numValues = 0;
bool runloop = false;
int ctr = 0;
int timestep = 1000;
bool readStep = false;
unsigned long startTime;
unsigned long runTime;
unsigned long ledTime;
unsigned long maxTime = 0;

void handleRoot() {
  String html = genDarkmodeCss() + "<form action='/submit' method='POST'>";
  html += "CSV Data:<br><input type='text' name='csv_data'><br>";
  html += "<input type='submit' value='Submit'></form>";
  html += "Upload a file:<br><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='file'><br><input type='submit' value='Upload'></form>";
  html += "<form method='GET' action='/stop'>";
  html += "<input type='submit' value='Stop Run'></form>";
  html += generateWebSocketHtml(false);
  server.send(200, "text/html", html);
}

void handleSubmit() {
  ctr = -1;
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, LOW);

  // Serial.println("Server.args():");
  // Serial.println(server.args());
  String csvData = server.arg("csv_data");
  // Serial.println("sent: " + csvData);
  // processCSVData(csvData);
  processBatchCSVData(csvData, 0, true);
}

bool isFloat(const String& str) {
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!isdigit(c) && c != '.' && c != '-' && c != '+')
      return false;
  }
  return true;
}

bool isInt(const String& str) {
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!isdigit(c))
      return false;
  }
  return true;
}

void handleStop() {
  servo.writeMicroseconds(SERVOMIN); // stop
  runloop = false;
  String t = genDarkmodeCss() + "Stop Button was pressed.";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  server.send(200, "text/html", t);
  digitalWrite(LED1, HIGH);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  static String uploadBuffer;
  static int batchValueIndex;
  static int accSize = 0;
  customPrint(String(accSize) + "/" + String(upload.totalSize));

  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("UPLOAD_FILE_START");
    uploadBuffer = "";
    batchValueIndex = 0;
    accSize = 0;

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.println("UPLOAD_FILE_WRITE " + String(upload.currentSize));
    uploadBuffer += String((const char*)upload.buf).substring(0, upload.currentSize);
    accSize += upload.currentSize;
    Serial.println(uploadBuffer);
    batchValueIndex = processBatchCSVData(uploadBuffer, batchValueIndex, false);
    int lastIndex = uploadBuffer.lastIndexOf(',');
    if (lastIndex != -1) {
      uploadBuffer = uploadBuffer.substring(lastIndex+1);
    } else {
      uploadBuffer = "";
    }
    Serial.println("Restof: " + uploadBuffer);

  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.println("UPLOAD_FILE_END: " + String(upload.totalSize));
    uploadBuffer += String((const char*)upload.buf).substring(0, upload.currentSize);
    Serial.println(uploadBuffer);
    // Check if the last character is a comma and remove it if true
    if (!uploadBuffer.isEmpty() && uploadBuffer[uploadBuffer.length() - 1] == ',') {
      uploadBuffer.remove(uploadBuffer.length() - 1); // Remove the last character
    }
    batchValueIndex = processBatchCSVData(uploadBuffer, batchValueIndex, true);
  }
}

int processBatchCSVData(String csvData, int valueIndex, bool last) {
  // Parse CSV data and store in array
  int startIndex = 0;
  int endIndex = csvData.indexOf(',');
  bool err = false;
  String csvValue;

  while (endIndex != -1 && valueIndex < MAX_VALUES && !err) {
    csvValue = csvData.substring(startIndex, endIndex);
    if (csvValue.length() > 0) {
      if(isFloat(csvValue)) {
        csvValues[valueIndex] = csvValue.toFloat();
      } else err = true;
    } else err = true;
    startIndex = endIndex + 1;
    endIndex = csvData.indexOf(',', startIndex);
    valueIndex++;
  }

  if (err) {
    String t = genDarkmodeCss() + "Error processing CSV data at index " + String(valueIndex-1) + "<br>Entry: " + csvValue + "<br>";
    t += "<form action='/'><input type='submit' value='Back to Form'></form>";
    server.send(200, "text/html", t);
    digitalWrite(LED1, HIGH);
    return valueIndex;
  }

  // only need to do all of this, if it is the last batch
  if(last) {
    // Store the last value
    if (valueIndex < MAX_VALUES) {
      csvValue = csvData.substring(startIndex);
      if (csvValue.length() > 0) {
        if(isFloat(csvValue)) {
          csvValues[valueIndex] = csvValue.toFloat();
        } else err = true;
      } else err = true;
      valueIndex++;
    } else {
      String t = genDarkmodeCss() + "Max number of values exceeded or error processing CSV data at index " + String(valueIndex-1) + "<br>Entry: " + csvValue + "<br>";
      t += "<form action='/'><input type='submit' value='Back to Form'></form>";
      server.send(200, "text/html", t);
      digitalWrite(LED1, HIGH);
      return valueIndex;
    }

    numValues = valueIndex;

    if (err) {
      String t = genDarkmodeCss() + "Something went wrong processing the csv data last index: " + String(valueIndex-1) + "<br>Entry: " + csvValue + "<br>";
      t += "<form action='/'><input type='submit' value='Back to Form'></form>";
      server.send(200, "text/html", t);
      digitalWrite(LED1, HIGH);
    } else {
      String t = genDarkmodeCss() + "Done processing CSV data...<br>";
      t += "Number of values received: " + String(numValues) + "<br>";
      t += "Last entry: " + csvValue + "<br>";
      t += "<form t action='/start' method='POST'>";
      t += "Timestep(ms): <input type='text' name='timestep'><br>";
      t += "ReadStep (every 2nd value as delay (cast to int-ms)): <input type='checkbox' name='stepread'><br>"; // Checkbox for stepread
      t += "<input type='submit' value='Start Simulation'></form>";
      t += "<form action='/'><input type='submit' value='Back to Form'></form>";
      server.send(200, "text/html", t);
    }
    digitalWrite(LED2, LOW);
  }
  return valueIndex;
}

void handleStart() {
  String dTime = server.arg("timestep");
  readStep = server.arg("stepread") == "on"; // "on" when checkbox is checked
  if(!readStep) {
    if (isInt(dTime)) {
      timestep = dTime.toInt();
      maxTime = numValues*timestep;
    } else {
      String t = genDarkmodeCss() + "Error in timestep value!";
      t += "<form action='/'><input type='submit' value='Back to Form'></form>";
      server.send(200, "text/html", t);
      digitalWrite(LED1, HIGH);
      return;
    }
  } else {
    maxTime = 0;
    for(int i = 1; i <=numValues-1; i+=2) {
      maxTime += csvValues[i];
    }
  }

  runloop = true;
  ctr = 0;
  runTime = millis();
  ledTime = millis();

  String t = genDarkmodeCss() + "Running Simulation now";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  t += "<form action='/stop'><input type='submit' value='Stop Run'></form>";
  t += generateWebSocketHtml(false);
  server.send(200, "text/html", t);
}

//generate responsive darkmode css
String genDarkmodeCss() {
  return "<style>body{background-color:white;color:black}@media(prefers-color-scheme:dark){body{background-color:#1d1e22;color:white}}</style>";
}

// r in morse = ok
void blink_r() {
    digitalWrite(LED1, HIGH);
    customDelay(700);
    digitalWrite(LED1, LOW);
    customDelay(100);
    digitalWrite(LED1, HIGH);
    customDelay(300);
    digitalWrite(LED1, LOW);
    customDelay(300);
    digitalWrite(LED1, HIGH);
    customDelay(300);
    digitalWrite(LED1, LOW);
    customDelay(100);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  // Ensure that x is within the input range
  x = constrain(x, in_min, in_max);
  
  // Calculate the proportion of x within the input range
  float proportion = (x - in_min) / (in_max - in_min);
  
  // Map the proportion to the output range and return the result
  return out_min + proportion * (out_max - out_min);
}

void customDelay(int waitTime) {
    int initTime = millis();
    while (millis() - initTime < waitTime) {
      handleLoop();
      if(runloop) break;
    }
}

void customPrint(String info) {
  Serial.println(info);
#ifdef WEBSOCKET
  // unsigned long startWait = millis();
  while(webSocket.connectedClients() == 0 && ctr < 2) {
    customDelay(25);
    // runTime = millis();
  }
  if(runloop) {
    customProgressInfo(info, runTime, maxTime);
  } else {
    webSocket.broadcastTXT(info + "<br>");
  }
#endif
}

void handleLoop() {
  server.handleClient();
#ifdef WEBSOCKET
  webSocket.loop();
#endif
}

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  servo.attach(SERVOPIN);  
  delay(1000);
  servo.writeMicroseconds(SERVOMIN); // send "stop" signal to ESC. Also necessary to arm the ESC.

  Serial.begin(115200);
  WiFi.hostname(hostname1);
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED1, digitalRead(LED1) ^ HIGH);
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/start", HTTP_POST, handleStart);
  server.on("/upload", HTTP_POST,[](){ server.send(200);}, handleUpload);
  server.onNotFound(handleRoot); // Serve the root page for any other URL

  digitalWrite(LED1, LOW);
  server.begin();
#ifdef WEBSOCKET
  webSocket.begin();
  // don't need any events from client to server (all done in html)
  // webSocket.onEvent(handleLoop());
#endif
}

void loop() {
  handleLoop();

  // stop servo if nothing is happening
  if(!runloop) {
    servo.writeMicroseconds(SERVOMIN); // stop/arm
  }
  // work loop
  if(runloop && ctr < numValues) {
    digitalWrite(LED2, digitalRead(LED2) ^ HIGH);
    //servo write csvValues[ctr];
    int val = static_cast<int>(mapFloat(csvValues[ctr],0.0, 1.0, SERVOMIN, SERVOMAX)); // maps potentiometer values to PWM value.
    servo.writeMicroseconds(val);
    startTime = millis();
    // customPrint(String(csvValues[ctr],4) + "->" + String(val));
    ctr++;
    if(readStep) {
      timestep = static_cast<int>(csvValues[ctr++]);
    }
    customPrint(String(val) + " for " + String(timestep) + "ms");

    while (millis() - startTime < timestep) {
      handleLoop();
      if(millis() - ledTime > 1000) {
        ledTime = millis();
        digitalWrite(LED1, digitalRead(LED1) ^ HIGH);
        // customPrint(String(val) + " for " + String(timestep-(millis() - startTime)) + "ms");
      }
    }
  }
  // work done
  if (ctr >= numValues) {
    if(runloop) {
      digitalWrite(LED2, LOW);
      digitalWrite(LED1, HIGH);
      customPrint("done in " + String((millis()-runTime)/1000.0) + "s/" + String((millis()-runTime)/60000.0) + "min");
      runloop = false;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      WiFi.begin(ssid, password);
      digitalWrite(LED1, digitalRead(LED1) ^ HIGH);
      customDelay(500);
    } else {
      blink_r();
    }
  }
}
