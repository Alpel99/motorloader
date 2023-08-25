#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "wifi_credentials.h" 
#include <Servo.h>

/* wifi credentials boilerplate

*/

#define LED1 2
#define LED2 16
// top view, right side at antenna, 2nd pin from top (D1)
#define SERVOPIN 5

ESP8266WebServer server(80);
// using this for the aerostar rvs 80a ESC
// 0/stop/arm = 1000, max=1900 (?)
Servo servo;
// stop/signal to arm
#define SERVOMIN 1000
// max full load, > than SERVOMIN
#define SERVOMAX 1900

const char* hostname1 = "motorloader";
// 12000: Variables and constants in RAM (global, static), used 77592 / 80192 bytes (96%)
// doesn't run like that, needs more space
// ATTENTION: there is some limit on transferred data with the post requests, too precise float numbers lead to not all values being read
const int MAX_VALUES = 1200;
float csvValues[MAX_VALUES];
int numValues = 0;
bool runloop = false;
int ctr = 0;
int timestep = 1000;
bool readStep = false;
unsigned long startTime;
unsigned long runTime;
unsigned long ledTime;

void handleRoot() {
  String html = "<form action='/submit' method='POST'>";
  html += "CSV Data: <input type='text' name='csv_data'><br>";
  html += "<input type='submit' value='Submit'></form>";

  html += "<form method='GET' action='/stop'>";
  html += "<input type='submit' value='Stop Loop'></form>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  ctr = -1;
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, LOW);

  String csvData = server.arg("csv_data");
  // Serial.println("sent: " + csvData);

  // Parse CSV data and store in array
  int startIndex = 0;
  int endIndex = csvData.indexOf(',');
  int valueIndex = 0;

  bool err = false;

  while (endIndex != -1 && valueIndex < MAX_VALUES && !err) {
    String csvValue = csvData.substring(startIndex, endIndex);
    // Serial.println(csvValue);
    // Serial.println(csvValue.length() > 0);
    // Serial.println(isFloat(csvValue));

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
    String t = "Error processing CSV data...";
    t += "<form action='/'><input type='submit' value='Back to Form'></form>";
    server.send(200, "text/html", t);
    digitalWrite(LED1, HIGH);
    return;
  }

  // Store the last value
  if (valueIndex < MAX_VALUES) {
    String csvValue = csvData.substring(startIndex);
    if (csvValue.length() > 0) {
      if(isFloat(csvValue)) {
        csvValues[valueIndex] = csvValue.toFloat();
      } else err = true;
    } else err = true;
    valueIndex++;
  } else {
    String t = "Max number of values exceeded";
    t += "<form action='/'><input type='submit' value='Back to Form'></form>";
    server.send(200, "text/html", t);
    digitalWrite(LED1, HIGH);
    return;
  }

  numValues = valueIndex;

  if (err) {
    String t = "Error processing CSV data...";
    t += "<form action='/'><input type='submit' value='Back to Form'></form>";
    server.send(200, "text/html", t);
    digitalWrite(LED1, HIGH);
  } else {
    String t = "Done processing CSV data...<br>";
    t += "Number of values received: " + String(numValues) + "<br>";
    t += "<form t action='/start' method='POST'>";
    t += "Timestep(ms): <input type='text' name='timestep'><br>";
    t += "ReadStep (every 2nd value as delay (int-ms)): <input type='checkbox' name='stepread'><br>"; // Checkbox for stepread
    t += "<input type='submit' value='Start Simulation'></form>";
    server.send(200, "text/html", t);
  }
  digitalWrite(LED2, LOW);
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
  String t = "Stop Button was pressed.";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  server.send(200, "text/html", t);
  digitalWrite(LED1, HIGH);
}

void handleStart() {
  String dTime = server.arg("timestep");
  readStep = server.arg("stepread") == "on"; // "on" when checkbox is checked
  if(!readStep) {
    if (isInt(dTime)) {
      timestep = dTime.toInt();
    } else {
      String t = "Error in timestep value!";
      t += "<form action='/'><input type='submit' value='Back to Form'></form>";
      server.send(200, "text/html", t);
      digitalWrite(LED1, HIGH);
      return;
    }
  }

  runloop = true;
  ctr = 0;
  runTime = millis();
  ledTime = millis();

  String t = "Running Simulation now";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  t += "<form action='/stop'><input type='submit' value='Stop Run'></form>";
  server.send(200, "text/html", t);
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
    startTime = millis();
    while (millis() - startTime < waitTime) {
      server.handleClient();
    }
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

  digitalWrite(LED1, LOW);
  server.begin();
}

void loop() {
  server.handleClient();

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
    Serial.println(String(csvValues[ctr],4) + "->" + String(val));
    ctr++;
    if(readStep) {
      timestep = static_cast<int>(csvValues[ctr++]);
    }

    startTime = millis();
    while (millis() - startTime < timestep) {
      server.handleClient();
      if(millis() - ledTime > 1000) {
        ledTime = millis();
        digitalWrite(LED1, digitalRead(LED1) ^ HIGH);
      }
    }
  }
  // work done
  if (ctr >= numValues) {
    if(runloop) {
      runloop = false;
      digitalWrite(LED2, LOW);
      digitalWrite(LED1, HIGH);
      Serial.println("done in " + String((millis()-runTime)/1000.0) + "s/ " + String((millis()-runTime)/60000.0) + "min");
    }
    blink_r();
  }
}
