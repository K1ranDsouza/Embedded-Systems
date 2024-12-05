//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 

//
// for esp32-cam
//

#define LED 18
#define BUTTON 19

volatile bool isOn;
volatile int  lastInterruptTime;
volatile int  thisInterruptTime;

#define MIN_INTERRUPT_INTERVAL 50000



// interrupt handler: flip the led from on to off and vice-versa.
void IRAM_ATTR myfunc(){
  thisInterruptTime = esp_timer_get_time();
  int interval;
  interval = thisInterruptTime - lastInterruptTime;
  lastInterruptTime = thisInterruptTime;
  if (interval < MIN_INTERRUPT_INTERVAL) return; // do nothing if last interrupt was recent
  isOn = !isOn;
}


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  
  // initialize digital pin LED as an output.
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLDOWN);
  attachInterrupt(BUTTON, myfunc, FALLING);
  digitalWrite(LED, LOW); //on initialize
  isOn = true;
  lastInterruptTime = 0;
}




// the loop function iterates forever
void loop() {
 if(isOn){
    digitalWrite(LED, LOW);
  }
  else{
    digitalWrite(LED, HIGH); 
  }
}
