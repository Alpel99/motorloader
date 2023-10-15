#ifndef WEBSOCKET_IMPL
#define WEBSOCKET_IMPL

#include <WebSocketsServer_Generic.h>

WebSocketsServer webSocket = WebSocketsServer(81);

String generateWebSocketHtml (bool append) {
  String html = "<div id='container' style='width: 70vw; word-wrap: break-word; padding: 10px; font-family: Arial; font-size: 16px; line-height: 1.5;'></div>"; //border: 1px solid #ccc; display: block; font-family: Arial, Helvetica, sans-serif; font-size: 40px;
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
  String m = minutes > 9 ? String(static_cast<int>(minutes)) : "0" + String(static_cast<int>(minutes));
  String s = seconds > 9 ? String(static_cast<int>(seconds)) : "0" + String(static_cast<int>(seconds));
  String r = m + ":" + s;
  return r;
}

String generateProgressBar(unsigned long start, unsigned long max) {
  int width = 20;
  unsigned long progress = millis()-start;
  double ratio = static_cast<double>(progress)/max;
  ratio = ratio > 1.00 ? 1 : ratio;
  int stride = ratio*width;
  String res = "|";
  for(int i = 0; i < width; i++) {
    if(i <= stride) {
      res += "▓"; //#,▓
    } else {
      res += "░"; //_░ 
    }
  }
  String t = max > progress ? generateTimeStr(max-progress) : "00:00"; // est time left
  res += "|  " + String(ratio*100) + "%" + " eta: " + t + " (mm:ss)";
  return res;
}

void customProgressInfo(String stepInfo, unsigned long startTime, unsigned long maxTime) {
  String bar = generateProgressBar(startTime, maxTime);
  // String step = (String(val) + " for " + String(timestep) + "ms");
  String step = stepInfo;
  webSocket.broadcastTXT(bar + "<br>" + step + "<br>"); // Send the updated value to all connected clients
}

#endif