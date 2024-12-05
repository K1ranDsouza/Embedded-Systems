//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 

#define BUTTON_LED_1 7
#define SHARING_CONTROL 10
#define BUTTON_PRESS_LED 6

int button_pressed;

void setup() {


  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  button_pressed = 0;

  pinMode(BUTTON_LED_1, INPUT);
  pinMode(BUTTON_PRESS_LED, OUTPUT);

  digitalWrite(BUTTON_PRESS_LED, LOW);
  delay(500);
}


void loop() {
  // put your main code here, to run repeatedly:

#if 0
  // Serial.print("READ="); Serial.println(state);
  if (!digitalRead(BUTTON_LED_1)) {
    button_pressed = 1;
    Serial.println("BUTTON_1 PRESSED");
  } else { 
    button_pressed = 0;
  }
 

  button_pressed = 1;
  if (0) {
    digitalWrite(BUTTON_PRESS_LED, HIGH);
    delay(500);
    digitalWrite(BUTTON_PRESS_LED, LOW);
    delay(500);   
  } else {
    digitalWrite(BUTTON_PRESS_LED, LOW);
  }
#endif 

  digitalWrite(BUTTON_PRESS_LED, HIGH);
  delay(500);
  digitalWrite(BUTTON_PRESS_LED, LOW);
  delay(500);
}
