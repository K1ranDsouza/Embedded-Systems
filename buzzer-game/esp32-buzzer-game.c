//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
//  buzzer game code for esp32-cam
//
//  full light guessing game, need to refine the play. 
// 
//  only uses 3 game buttons as-is, update BL_COUNT and button_or_led for 4 buttons.
// 
//  NEWGAMELED and NEWGAMEBUTTON are active low.
//

#define BUILTIN_LED 33

// GPIO to control whether to read Button or write Led
#define BL_CONTROL 16

// number of Button-or-Led pairs
#define BL_COUNT 4

// GPIO to control indivitual Button-or-Led pairs
int button_or_led[] = {12,13,15,14};


// buzzer  - need to make sure we use a PWM-capable pin
#define BUZZER_PIN 2
#define BUZZER_CHANNEL 4


// button to start a new game (this is the flash GPIO, but we'll use it for input)
#define NEWGAMEBUTTON 1

// LED to tell the user the system is waiting for a new game
#define NEWGAMELED 3


// Global game state
int num_leds_flashed;   // how many led's were flashed
int led_mem[] = {0, 0, 0, 0};  // the sequence we flashed led's
int current_led;        // which led in the sequence the user is guessing
bool awaiting_new_game; 

// the code in setup function runs only one time when ESP32 starts
void setup() { 
  Serial.begin(115200);

  // Initialize GPIO
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(NEWGAMEBUTTON, INPUT_PULLUP);
  pinMode(NEWGAMELED, OUTPUT);
   
  int i;
  for (i=0; i < BL_COUNT; i++) { 
    pinMode(button_or_led[i], INPUT);
  }
  pinMode(BL_CONTROL, OUTPUT);

  // Buzzer: Drive the + input of the buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  // Initialize game state
  awaiting_new_game = true;
  num_leds_flashed = 0;
  current_led = 0; 
}


// the code in loop function is executed repeatedly infinitely
void loop() {
  if (awaiting_new_game) {
      // turn on new game LED, wait for new game button press
      digitalWrite(NEWGAMELED, LOW);  // NOTE: output 0 turns on led
      // flash builtin led
      digitalWrite(BUILTIN_LED, LOW);  // NOTE: output 0 turns on led
      delay(500);
      digitalWrite(BUILTIN_LED, HIGH);
      if (!digitalRead(NEWGAMEBUTTON)) {
        // start game
        awaiting_new_game = false;
        digitalWrite(NEWGAMELED, HIGH);  // NOTE: output 1 turns off led
        ledcWriteNote(BUZZER_CHANNEL, NOTE_C, 1);
        start_game();
        current_led = 0;
      } else { 
        ledcWriteNote(BUZZER_CHANNEL, NOTE_A, 1);
        delay(500);
      }
  } else {
    // inside a game
    if (!digitalRead(NEWGAMEBUTTON)) {
      awaiting_new_game = true;
    } else if (!play_one_round()) {
      ledcWriteNote(BUZZER_CHANNEL, NOTE_F, 1);
      delay(500);
      awaiting_new_game = true;
    }
  }
}



//
// start_game: pick between 1 and 4 led's, remember which ones we 
// picked and flash the corresponding led's. There is one round
// of the game for each led flashed to check the user picked it
// correctly.
//
void start_game() {
  num_leds_flashed = random(BL_COUNT)+1; // FIXME: should this be increasing?

  // pick leds to flash
  ledcWriteNote(BUZZER_CHANNEL, NOTE_D, 4);
  int i, j;
  for (i=0; i < num_leds_flashed; i++) {
      j = random(BL_COUNT);
      led_mem[i] = j;
      flash_led(j);
  }
}

//
// play_one_round: Play one round of the game - read a button and
// check if it is the right button. Return true if the game should
// continue, and false if the game is over.
//
bool play_one_round() {
    int i;

    i = read_button();
    if (i >= 0)  {
        // user pressed a button 
        flash_led(i);
        if (led_mem[current_led] == i) {
          // right button was pressed, game goes on
          if (current_led + 1 == num_leds_flashed) {
            // user was successful with all leds: play happy music and
            // give user fresh problem (effectively a new game)
            start_game();
          } else {
            current_led++;      // check next led in sequence
          }
          return true;  // continue playing fresh game or next round
        } else {
          // wrong button pressed, game stops
          // play sad music
          return false;
        }
    } else {
      return true; // no button pressed, so game goes on
    } 
}



void flash_led(int i) {
  digitalWrite(BL_CONTROL, HIGH);  // set control to led mode
  pinMode(button_or_led[i], OUTPUT);
  digitalWrite(button_or_led[i], LOW);  // turn led on
  delay(500);
  digitalWrite(button_or_led[i], HIGH);  // turn led off
  delay(500);
}



//
// button_press: If any button pressed, return index of the button.
// If no button pressed, return -1. 
//
int read_button(){
  digitalWrite(BL_CONTROL, LOW);  // set control to button mode
 
  // set all GPIO for buttons to INPUT
  int i;
  for (i=0; i < BL_COUNT; i++) { 
    pinMode(button_or_led[i], INPUT);
  }

  // check which button is pressed
  for (i=0; i < BL_COUNT; i++) { 
    if (!digitalRead(button_or_led[i])) return i;
  }
  return -1;  // no button was pressed
}
