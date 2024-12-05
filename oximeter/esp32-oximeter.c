//
// Copyright (C) 2021 Kiran Dsouza. All rights reserved.
// 
// Derived from code in Sparkfun library (based on Example8_SPO2)
//

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include "MAX30105.h"
#include "spo2_algorithm.h"
MAX30105 *particleSensor = NULL;     // use a pointer so we can recycle it
#define debug Serial

// variables used for Example8_SPO2
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data

#define readLED 2   // onboard LED indicates that a reading is being taken
#define BUTTON 19
#define MY_SDA 21
#define MY_SCL 22
#define MY_VCC 18  //turns display and sensor on/off

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 *display = NULL;    // use a pointer so we can recycle it


//handle button interrupt to display/sensor on/off
volatile bool isOn = true;
volatile int  lastInterruptTime;
volatile int  thisInterruptTime;
bool prevIsOn = true;

#define MIN_INTERRUPT_INTERVAL 50000

// remember last valid oxygen and pulse values for display
uint32_t oxygen = 0;
uint32_t pulse = 0;

void IRAM_ATTR myfunc(){
   Serial.println("button press!");
  thisInterruptTime = esp_timer_get_time();
  int interval;
  interval = thisInterruptTime - lastInterruptTime;
  lastInterruptTime = thisInterruptTime;
  if (interval < MIN_INTERRUPT_INTERVAL) return; // do nothing if last interrupt was recent
  isOn = !isOn;
}

void initSensorDisplay() {
  particleSensor = new MAX30105();
// Initialize sensor, based on Example8_SPO2 from Sparkfun
  if (particleSensor->begin(Wire, I2C_SPEED_FAST) == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1); // hang here on error
  }
  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  particleSensor->setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor->setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor->setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  
  // Initialize display, based on Adafruit example code
  //
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display = new  Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  if(!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // hang here on error
  }
 
  Serial.println("Initialization complete.");

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display->display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display->clearDisplay();

  // End setup with my custom splash screen
  display->setTextSize(2); // Draw 2X-scale text 
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Setup done"));
  display->println();
  display->setTextSize(1);
  display->println(F("Press finger on\nsensor and wait"));
  display->display();  // send to display
  delay(500);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Setup starting ....");

  Wire.begin(MY_SDA, MY_SCL);
  Serial.print("SDA="); Serial.println(MY_SDA);
  Serial.print("SCL="); Serial.println(MY_SCL);

  pinMode(MY_VCC, OUTPUT);
  pinMode(BUTTON, INPUT_PULLDOWN);
  attachInterrupt(BUTTON, myfunc, FALLING);
  isOn = true;
  lastInterruptTime = 0;
  digitalWrite(MY_VCC, HIGH);
  delay(500);
  initSensorDisplay();
  delay(1000);
}

// Display pulse and oxygen level on ssd1306 display
void displayPulseAndSpo2()
{
  if (!display) return;  // no display to write to
  
  if (validHeartRate) pulse = heartRate;
  if (validSPO2) oxygen = spo2;
 
  display->clearDisplay();
  // display.setTextSize(2); // Draw 2X-scale text 
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->setTextSize(1); // Draw 1X-scale text 
  display->println(F(""));
  display->setTextSize(2); // Draw 2X-scale text 
  display->print("Pulse :");
  display->println(pulse, DEC); 
  display->print("Oxygen:");
  display->println(oxygen, DEC);   
  display->display();  // send to display
}


// This code to extract heart rate and spo2 is borrowed from Sparkfun Example8_SPO2 with no changes. 
// I just added a call to display oxygen and pulse on the ssd1306 display.
//
void processSensor()
{
  if (!particleSensor) return;  // no sensor to read from
  
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor->available() == false) //do we have new data?
      particleSensor->check(); //Check the sensor for new data

    redBuffer[i] = particleSensor->getRed();
    irBuffer[i] = particleSensor->getIR();
    particleSensor->nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    if (isOn != prevIsOn) {
      break;  // button was pressed to turn off
    }
    
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor->available() == false) //do we have new data?
        particleSensor->check(); //Check the sensor for new data

      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

      redBuffer[i] = particleSensor->getRed();
      irBuffer[i] = particleSensor->getIR();
      particleSensor->nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    displayPulseAndSpo2();
  }
}


void loop() {
 if (isOn && prevIsOn) {
   // stay on
   Serial.println("Staying on!");
   processSensor();
 }
 if(isOn && !prevIsOn){
    // turn on
    Serial.println("Turning on!");
    digitalWrite(MY_VCC, HIGH);
    initSensorDisplay();
 }  
 if (!isOn &&  prevIsOn) { 
    // turn off
    Serial.println("Tuning off!");
    digitalWrite(MY_VCC, LOW);
    delete particleSensor;
    delete display;
    particleSensor = NULL;
    display = NULL;
 }
 if (!isOn and !prevIsOn) { 
    Serial.println("Staying off");
    delay(500);  
 }
 prevIsOn = isOn;
}
