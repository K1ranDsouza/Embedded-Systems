//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
// Code derived from example at: 
//     https://circuitdigest.com/microcontroller-projects/interfacing-neo6m-gps-module-with-esp32
//
// Main change is to not error out if GPS doesn't update quickly enough. Also added DEBUG macro.
//
// Read data from GPS using TinyGPUPlus library
// Data between GPS to ESP32 uses Serial2
// Data between ESP32 and PC uses Serial
// 
// You should add your SSID name and password below for the code to work.
//

#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>


// Set DEBUG to 1 to just dump raw traffic from GPS
#define DEBUG 0

#include <TinyGPSPlus.h>

TinyGPSPlus gps;


#define SSID_name "XXXX"
#define SSID_pwd  "XXXX"

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

unsigned long gpsLastTime = 0; // tracking for gps

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1);  // talk to GPS on Serial2
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

  lastTime = 0;  // tracking for wifi
  gpsLastTime = 0; // tracking for gps
  delay(3000);  // additional 3-second delay to allow gps to get going 
}


void loop() {

  processGpsData();
  
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


// Use the TinyGPSPlus object to interpret the data that the GPS is sending back 
// to the ESP32
void displayInfo()
{
  Serial.print(F("Kiran Location: "));
  if (gps.location.isValid()){
    Serial.print("Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print("Lng: ");
    Serial.print(gps.location.lng(), 6);
    Serial.println();
  }  else  {
      Serial.print(F("INVALID"));
  }
}


// Debug utility: Simply dump whatever the GPS is sending back to the ESP32 to Serial.
void updateSerial()
{
  delay(500);
  Serial.write("Reading Serial2 ...");
  while (Serial2.available()) {
      Serial.write(Serial2.read());//Forward what Software Serial received to Serial Port
  }
}


void processGpsData() {
  unsigned long nowTime = millis();
  
  if (DEBUG) { 
    Serial.println(F("KD debug:"));
    delay(500);
  } else {
    delay(100);
  }
  
  if (DEBUG) { 
    updateSerial();
  } else {
    while (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        displayInfo();
        gpsLastTime = nowTime;
      }
    }

    // warn if we don't hear from GPS for 5s
    if (nowTime > (5000 + gpsLastTime) && gps.charsProcessed() < 10) {
      Serial.print(F("No GPS update for 5s. Chars processed="));
      Serial.println(gps.charsProcessed());
      delay(10000);  // wait 10s for GPS to generate data
      gpsLastTime = nowTime;
    }
  } 
}
