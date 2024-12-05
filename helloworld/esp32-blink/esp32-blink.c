//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
// Simple ESP32 program to pipeclean a setup.

// Simply connect ESP32 board via USB to your
// PC and download the program to make the 
// on-board LED blink. 

// Also connect a terminal like Teraterm (on PC)
// or just the built-in terminal on MAC to view
// the debug output sent to Serial.

// ESP32 built-in LED is on pin 2
#define LED 2

bool isLow = false;

void setup() {

  Serial.begin(115200);
  Serial.println("Starting up ...");

  pinMode(LED, OUTPUT);
  
}

void loop() {

  Serial.print("LED=");
  Serial.println(isLow);
  
  if (isLow) { 
    digitalWrite(LED, LOW);
  } else { 
    digitalWrite(LED, HIGH);
  }

  isLow = !isLow;
  delay(100);
}
