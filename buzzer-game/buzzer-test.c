//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
#define BUZZER_PIN  22
#define LED_PIN 23
#define BUZZER_CHANNEL 0

void setup() {
  // put your setup code here, to run once:
  // Serial.begin(115200);
  // Serial.println("Hello, ESP32!");
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); 
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
}

void loop() {
  // put your main code here, to run repeatedly:
  ledcWriteNote(BUZZER_CHANNEL, NOTE_D, 4);
  digitalWrite(LED_PIN, HIGH); 
  delay(500); // this speeds up the simulation
  digitalWrite(LED_PIN, LOW);
  delay(500);
} 
