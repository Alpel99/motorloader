#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "wifi_credentials.h" 
#include <Servo.h>

#define LED1 2
#define LED2 16
// top view, right side at antenna (D1)
#define SERVOPIN 5

ESP8266WebServer server(80);
Servo servo;

const char* hostname1 = "motorloader";
const int MAX_VALUES = 500;
float csvValues[MAX_VALUES];
int numValues = 0;
bool runloop = false;

void handleRoot() {
  String html = "<form action='/submit' method='POST'>";
  html += "CSV Data: <input type='text' name='csv_data'><br>";
  html += "<input type='submit' value='Submit'></form>";

  html += "<form method='POST' action='/stop'>";
  html += "<input type='submit' value='Stop Loop'></form>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, LOW);

  String csvData = server.arg("csv_data");
  // Parse CSV data and store in an array
  // Provide feedback to the user
  String t = "CSV Data received, processing...";
  // server.send(200, "text/html ", t);
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
      } else
        err = true;
    } else
      err = true;
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
      } else
        err = true;
    } else
      err = true;
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
    t += "<input type='submit' value='Start Simulation'></form>";
    server.send(200, "text/html", t);
  }
  digitalWrite(LED2, LOW);
}

bool isFloat(const String& str) {
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isdigit(c) || c == '.' || c == '-' || c == '+') {
      return true;
    }
  }
  return false;
}

bool isInt(const String& str) {
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

void handleStop() {
  runloop = false;
  String t = "Stop Button was pressed.";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  server.send(200, "text/html", t);
  digitalWrite(LED1, HIGH);
}

void handleStart() {
  String dTime = server.arg("timestep");
  int timestep;
  if (isInt(dTime)) {
    timestep = dTime.toInt();
  } else {
    String t = "Error in timestep value!";
    t += "<form action='/'><input type='submit' value='Back to Form'></form>";
    server.send(200, "text/html", t);
    digitalWrite(LED1, HIGH);
    return;
  }

  String t = "Running Simulation now";
  t += "<form action='/'><input type='submit' value='Back to Form'></form>";
  t += "<form action='/stop'><input type='submit' value='Stop Run'></form>";
  server.send(200, "text/html", t);

  runloop = true;
  int ctr = 0;
  while(runloop && ctr < numValues) {
    digitalWrite(LED2, digitalRead(LED2) ^ HIGH);
    //servo write csvValues[ctr];    
    int val = map(csvValues[ctr],0, 1, 1100, 1900); // maps potentiometer values to PWM value.
    if(val>1900) val = 1900;
    if(val<1100) val = 1100;
      
    servo.writeMicroseconds(val);
    Serial.print(csvValues[ctr]);
    Serial.print("->");
    Serial.println(val);

    server.handleClient();
    delay(timestep);
    ++ctr;
  }
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, HIGH);
  Serial.println("done");
  while(runloop) { // ok in morse
    server.handleClient();
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED1, HIGH);
    delay(300);
    server.handleClient();
    digitalWrite(LED1, LOW);
    delay(300);
    digitalWrite(LED1, HIGH);
    delay(300);
    server.handleClient();
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED1, HIGH);
    delay(300);
    server.handleClient();
    delay(700);
  }
}

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  servo.attach(SERVOPIN);
  servo.writeMicroseconds(1500); // send "stop" signal to ESC. Also necessary to arm the ESC.
  
  delay(1000);
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
}

/*
#include <Servo.h>

byte servoPin = 9; // signal pin for the ESC.
byte potentiometerPin = A0; // analog input pin for the potentiometer.
Servo servo;

void setup() {
    servo.attach(servoPin);
    servo.writeMicroseconds(1500); // send "stop" signal to ESC. Also necessary to arm the ESC.

    delay(7000); // delay to allow the ESC to recognize the stopped signal.
}

void loop() {

    int potVal = analogRead(potentiometerPin); // read input from potentiometer.

    int pwmVal = map(potVal,0, 1023, 1100, 1900); // maps potentiometer values to PWM value.

    servo.writeMicroseconds(pwmVal); // Send signal to ESC.

*/
