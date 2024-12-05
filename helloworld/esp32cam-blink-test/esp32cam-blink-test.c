//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
// Variant of esp32-blink but for the ESP32-CAM
// board instead of the regular ESP32-DEV board.

// For ESP32-CAM we cannot use pin 2 for the 
// builtin LED as it links to the flash. 
// Instead we have to use pin 33.

// Also, ESP32-CAM seems to connect to COM4 
// intead of COM3 as for ESP32-DEV


// For Serial output, use the Terminal in the Arduino IDE
// instead of Teraterm on Windows, because of an issue with DTR/RTS 
// which puts the device in flash mode when Teraterm is connected.

// Finally, use the ESP32-DEV module in the 
// Arduino IDE. The ESP32-CAM-USB doesn't work.

#define LED 33


bool isLow = false;

void setup() {
 
  Serial.begin(115200);
  Serial.println("Starting program");

  pinMode(LED, OUTPUT);
  
}

void loop() {

  Serial.print("LEDCAM=");
  Serial.println(isLow);
  
  if (isLow) { 
    digitalWrite(LED, LOW);
  } else { 
    digitalWrite(LED, HIGH);
  }

  isLow = !isLow;
  delay(100);
}
