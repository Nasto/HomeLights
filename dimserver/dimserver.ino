#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const int LEDpin = 5;
int brightness = 0;
int toggleBrightness = 0;
int shiftDelay = 512;
const char* ssid = "Phalice";
const char* password = "DWi100%pfuerAS&SD";
const char* site = "<!DOCTYPE html>\n<html lang=en> <head><meta name=viewport content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\"><title>Lamp Controller</title><style type=text/css>body{background-color:#f3f3f3}.container{padding:15px;margin-right:auto;margin-left:auto}@media(min-width:768px){.container{width:750px}}@media(min-width:992px){.container{width:970px}}@media(min-width:1200px){.container{width:1170px}}.slidercontainer{width:100%}.slider{-webkit-appearance:none;appearance:none;width:100%;height:15px;outline:0;opacity:.7;-webkit-transition:.2s;transition:opacity .2s}.slider:hover{opacity:1}.slider::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:25px;height:25px;border-radius:20%;background:#00bbdf;cursor:pointer}.slider::-moz-range-thumb{width:25px;height:25px;border-radius:20%;background:#00bbdf;cursor:pointer}body,.button,.numberInput{font-family:Verdana,Geneva,sans-serif;font-weight:500;font-style:normal;font-variant:normal}.numberInput{font-size:16px;line-height:20px;border-radius:5px;border:1px solid #dedede;padding:12px 20px;margin:8px 0;width:2em}body{font-size:24px;line-height:26.4px}.button{font-size:16px;line-height:20px;border:1px solid #dedede;border-radius:5px;padding:12px 20px;margin:8px 0;background:#00bbdf;color:#f7f7f7}input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}input[type=number]{-moz-appearance:textfield}</style></head><body onload=refresh()><div class=container> <label for=absoluteBrightness>Brightness</label> <input type=number name=absoluteBrightness id=absoluteBrightness min=0 max=100 step=1 class=numberInput> <button class=button id=brightnessSubmit>Set</button> <div class=sliderContainer> <input type=range min=1 max=100 value=50 class=slider name=brightnessSlider id=brightnessSlider> </div> </div> <script type=text/javascript>request=new XMLHttpRequest();factor=1023/100;function refresh(){if(request.readyState==0||request.readyState==4){request.onreadystatechange=handleResponse;request.open('GET','brightness',true);request.send();}}function handleResponse(){if(request.readyState==4&&request.status==200){var xmlDoc=request.responseXML;var brightness=xmlDoc.getElementsByTagName('brightness')[0].firstChild.nodeValue;var value=Math.floor(brightness/factor);document.getElementById('absoluteBrightness').value=Number(value);document.getElementById('brightnessSlider').value=Number(value);}}function setBrightness(event){var rawVal=event.target.value;if(event.target.id==='brightnessSubmit'){rawVal=Number(document.getElementById('absoluteBrightness').value);}var sliderVal=Math.floor(rawVal*factor);document.getElementById('absoluteBrightness').value=Number(rawVal);document.getElementById('brightnessSlider').value=Number(rawVal);if(request.readyState==0||request.readyState==4){request.open('PUT','setBrightness?val='+sliderVal,true);request.send(null);}}function toggle(){request.open('GET', 'toggle', true);request.send();}document.getElementById('brightnessSlider').addEventListener('change',setBrightness,true);document.getElementById('absoluteBrightness').addEventListener('change',setBrightness,true);document.getElementById('brightnessSubmit').addEventListener('click',toggle,true);</script></body></html>";
boolean off = false;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LEDpin, OUTPUT);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wifi is connected:");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/brightness", []() {
    sendBrightness();
  });
  server.on("/setBrightness", handleSetBrightness);
  server.on("/toggle", toggle);
  server.begin();
  Serial.println("Server started.");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  Serial.println("Root-request received.");
  server.send(200, "text/html", site);
}

void handleSetBrightness() {
  Serial.print("Set-request received. Value: ");
  int value = server.arg("val").toInt();
  Serial.println(value);
  shiftBrightness(value);
  off = false;
  server.send(200, "text/plain", "");
}

void shiftBrightness(int value){
  Serial.println("Computing shift...");
  int diff = value - brightness;
  int dir = 1;
  if(diff < 0){
    dir = -1;
  }
  int wait = max(1, (int) ((shiftDelay / (abs(diff) / 2)) + 0.5));
  Serial.print("Shifting: ");
  Serial.print(diff);
  Serial.print(" | Wait: ");
  Serial.println(wait);
  while(brightness != value){
    brightness += dir;
    analogWrite(LEDpin, brightness);
    delay(wait);
  }
  Serial.println("Done shifting.");
}

void sendBrightness() {
  Serial.println("Sending brightness response.");
  String response = "<?xml version='1.0'?><xml><brightness>";
  response += brightness;
  response += "</brightness></xml>";
  server.send(200, "text/xml", response);
}

void toggle() {
  Serial.println("Toggle");
  if(!off){
    toggleBrightness = brightness;
    shiftBrightness(0);
  } else {
    brightness = 0;
    shiftBrightness(toggleBrightness);
  }
  off = !off;
  server.send(200, "text/plain", "");
}

