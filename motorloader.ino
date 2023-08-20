#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "wifi_credentials.h" 

ESP8266WebServer server(80);

void handleRoot() {
  String html = "<form action='/submit' method='POST'>";
  html += "CSV Data: <input type='text' name='csv_data'><br>";
  html += "<input type='submit' value='Submit'></form>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  String csvData = server.arg("csv_data");
  // Parse CSV data and store in an array
  // Provide feedback to the user
  server.send(200, "text/plain", "CSV data received and processed.");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);

  server.begin();
}

void loop() {
  server.handleClient();
}