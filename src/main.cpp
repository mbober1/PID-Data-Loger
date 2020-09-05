#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

#define ONE_WIRE_BUS D3
#define PIN_OUTPUT D5

const char* ssid = "Bober.NET";
const char* password = "UP56&xE&n+!wtdxv";

const char* PARAM_INPUT_1 = "P";
const char* PARAM_INPUT_2 = "I";
const char* PARAM_INPUT_3 = "D";
const char* PARAM_INPUT_4 = "Setpoint";

double Setpoint = 0, Input, Output;
double Kp = 20;
double Ki = 0, Kd = 1;


AsyncWebServer server(80);
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  sensors.begin();
  myPID.SetMode(AUTOMATIC);
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Input));
  });

  server.on("/setpoint", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Setpoint));
  });

  server.on("/output", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Output));
  });
  

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      Kp = inputMessage.toInt();
      Serial.printf("Kp = %f \n", Kp);
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      Ki = inputMessage.toInt();
      Serial.printf("Ki = %f \n", Ki);
    }
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      Kd = inputMessage.toInt();
      Serial.printf("Kd = %f \n", Kd);
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      Setpoint = inputMessage.toInt();
      Serial.printf("Setpoint = %f \n", Setpoint);
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    
    request->send(200, "text/html", "<meta http-equiv='refresh' content='1; URL=\"/\"' />Ustawiono (" 
                                     + inputParam + ") na: " + inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  sensors.requestTemperatures();
  Input = sensors.getTempCByIndex(0);

  if(Input>0) {
    myPID.Compute();
    analogWrite(PIN_OUTPUT, Output);
  }

  Serial.print(Input);
  Serial.print("\t");
  Serial.print(Setpoint);
  Serial.print("\t");
  Serial.println(Output);
}