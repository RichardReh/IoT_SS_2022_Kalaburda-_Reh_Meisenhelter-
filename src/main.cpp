#include <Arduino.h>
#include <WiFi.h>
#include <Stepper.h>
#include "ESPAsyncWebServer.h" 
#include <DHT.h>
#include <iostream>
#include <string>

//Motor
int steps = 2048;
Stepper Schrittmotor(steps,14,27,26,25);

//Reed Schalter

int schalter = 22;

//Hall Sensor

int hall = 14;


////////////////////////////////

DHT my_sensor(18,DHT11);  //  <--- Für das initialisieren des Temperatursensors
float temperature;        //  <--- aktuelle Temperaturvariable


//Wi-fi Einstellungen und Port
char ssid[] = "AndroidAPF78A";
char pass[] = "12345678";
int status = WL_IDLE_STATUS;

AsyncWebServer server(80);

//Lagern des HTTP requests
String header;

//html
String html = "<!doctype html><html lang='en' style='height: 100vh; font-family: Arial,serif'><head><meta charset='UTF-8'><meta name='viewport'content='width=device-width, user-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0'><meta http-equiv='X-UA-Compatible' content='ie=edge'><title>Grill App</title></head><body style='height: 100vh; display: flex; align-items: center; justify-content: center'><div style='display: flex; align-items: center; justify-content: center; flex-direction: column; background: lightgray; padding: 20px; border-radius: 5px'><h1>Grill App</h1><div><form style='display: flex; flex-direction: column'><span><b style='margin-right: 5px'>Aktuelle Grilltemperatur:</b> <i id='grill-temperature'>30</i><i>&nbsp;Grad</i></span><label style='margin-top: 10px; display: flex; align-items: center'><b style='margin-right: 5px'>Grilltemperatur:</b><input type='range' min='30' max='70' step='5' value='30' onchange='sendAjaxGetRequest(`/setTemperature?temp=${this.value}`)' oninput='setOutput(`${this.value} Grad`)' style='margin-right: 5px'><span id='output'>30 Grad</span></label></form></div></div></body></html><script>function setOutput(value) {document.getElementById('output').innerHTML = value;};function sendAjaxGetRequest(url, callback) { let xhr = new XMLHttpRequest();xhr.open('GET', url, true);xhr.onload = function() {if (this.status === 200) { if (callback) {callback(this.responseText);}}};xhr.send();};setInterval(function() {sendAjaxGetRequest('/temperature', function(response) {document.getElementById('grill-temperature').innerHTML = response;});}, 3000);</script>";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


//lock-variable
bool lock = false;

//Die vom Client gewünschte Temperatureinstellung:
float sollTemp = 30.00; // kleinstmögliche TempStufe ist am Anfang eingestellt
float currentTemp = 0.00;

void setup() {
  Serial.begin(9600);
  Schrittmotor.setSpeed(5);
  WiFi.begin(ssid, pass);
  my_sensor.begin();
  pinMode(schalter, INPUT);
  pinMode(hall, INPUT);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", html);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(currentTemp,2));
  });

  server.on("/setTemperature", HTTP_GET, [](AsyncWebServerRequest *request){
    const AsyncWebParameter* param = request->getParam(0);
    sollTemp = param->value().toFloat();
    request->send(200, "text/plain", "Hello World");
  });

  server.begin();
}

void loop() {

  if (digitalRead(hall)==LOW){
    temperature = my_sensor.readTemperature();
    currentTemp = temperature;

    Serial.println(temperature);
    Serial.println(sollTemp);

    delay(300);
    
    
    if ((temperature >= sollTemp && temperature <= (sollTemp+3.00)) && lock){
        Schrittmotor.step(512);
        lock = false;
        delay(500);
    } else if ((temperature < sollTemp) && lock == false){
        Schrittmotor.step(512);
        lock = true;
    }
  } 
  
  
           
}
