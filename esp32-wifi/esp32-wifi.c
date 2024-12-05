//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
//
// Based on code obtained online, also contributions from Berde/Fantini.
//
// You should add your SSID name and password below for the code to work.
//

#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>

#define SSID_name "XXXXX"
#define SSID_pwd  "XXXXX"

const char* ssid = SSID_name;
const char* password = SSID_pwd;
 
AsyncWebServer server(80);
 
IPAddress localIP(10,10,10,1);
IPAddress mainServerIP(10,10,10,0);
IPAddress gatewayIP(10,10,10,100);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(10,10,10,100);
IPAddress secondaryDNS(10,10,10,100);
 
//Your Domain name with URL path or IP address with path
String serverName = "http://10.10.10.1";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

String payload;

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to Wi-Fi as "); Serial.println(localIP);
  int waitCount=0;
  if (!WiFi.config(localIP, gatewayIP, subnet, primaryDNS, secondaryDNS)) { Serial.println("STA Failed to configure"); }
  WiFi.mode(WIFI_STA); WiFi.begin(SSID_name,SSID_pwd); while (WiFi.status() != WL_CONNECTED){ Serial.print("."); delay(200); if(waitCount++>300)ESP.restart(); } Serial.println("");
  Serial.print("Wi-Fi connected.   IP Address: "); Serial.print(WiFi.localIP());  Serial.print("   Gateway Address: "); Serial.println(WiFi.gatewayIP());
 
  server.on("/Data", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web Page '/' requested.");
    String message ="Hi\n";
    request->send(200, "text/html", message );
    });
 
  server.onNotFound( [](AsyncWebServerRequest *request){request->send(404,"text/plain","Not found");} );
 
  server.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
 
void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    for(int i = 1; i < 23; i++) {
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      char a[2];
      String str;
      str=String(i);
      str.toCharArray(a,2);

      String serverPath = "http://10.10.10." + str + "/Data";
    
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
        Serial.println("Data from http://10.10.10." + str + "/Data");
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();

      if (httpResponseCode>0) {
        String Berde = "http://20.106.144.20/Upload/?localIP=10.10.10.1&localName=Mike_Fantini&remoteIP=10.10.10." + str + "&remoteInfo=" + payload;
        http.begin(Berde.c_str());
        int httpResponseCode = http.GET();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    }
    lastTime = millis();
  }
}
