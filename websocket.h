#ifndef WEBSOCKET_IMPLEMENTAITON
#define WEBSOCKET_IMPLEMENTAITON

#include <WebSocketsServer_Generic.h>

WebSocketsServer webSocket = WebSocketsServer(81);

String generateWebSocketHtml (bool append) {
  String html = "<div id='container' style='width: 70vw; word-wrap: break-word; padding: 10px;'>"; //border: 1px solid #ccc; display: block;
  html += "<script>";
  html += "var socket = new WebSocket('ws://' + window.location.hostname + ':81/');";
  html += "socket.onmessage = function(event) {";
  String add = append ? " + container.innerHTML" : "";
  html += "  document.getElementById('container').innerHTML = event.data" + add;
  html += "};";
  html += "</script>";
  return html;
}

String generateTimeStr(unsigned long duration) {
  unsigned long currentMillis = duration;
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  String r = String(static_cast<int>(minutes)) + ":" + String(static_cast<int>(seconds));
  return r;
}

String generateProgressBar(unsigned long start, unsigned long max) {
  int width = 20;
  unsigned long progress = millis()-start;
  double ratio = static_cast<double>(progress)/max;
  int stride = ratio*width;
  String res = "|";
  for(int i = 0; i < width; i++) {
    if(i <= stride) {
      res += "▓"; //#,▓
    } else {
      res += "░"; //_░ 
    }
  }
  String t = max > progress ? generateTimeStr(max-progress) : "0:00"; // est time left
  res += "|  " + String(ratio*100) + "%" + " eta: " + t;
  return res;
}

void customProgressInfo(String stepInfo, unsigned long startTime, unsigned long maxTime) {
  String bar = generateProgressBar(startTime, maxTime);
  // String step = (String(val) + " for " + String(timestep) + "ms");
  String step = stepInfo;
  webSocket.broadcastTXT(bar + "<br>" + step + "<br>"); // Send the updated value to all connected clients
}

#endif