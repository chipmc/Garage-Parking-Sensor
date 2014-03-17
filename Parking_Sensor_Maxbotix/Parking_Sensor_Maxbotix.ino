
/*
// ---------------------------------------------------------------------------
// This is a sensor to help us park in the garage.  I have two sensors, two adjustments and two indicator lights.
// ---------------------------------------------------------------------------


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License v3 as published by
the Free Software Foundation.

Updated to use the more powerful Maxbotix sensors - http://maxbotix.com/documents/MB1000_Datasheet.pdf
We will use them in analog mode but will pulse the sensors since there are two in the same garage
  - This will be a commanded loop - http://www.maxbotix.com/documents/LV_Chaining_Commanded_Loop_AN_Out.pdf
  - Assume we put a jumper on the board to hold BW HIGH to enable pulsed operation
  
 
Notes: 
 1) You need to reset the Adruino for adjustments to the range on the pots to take effect
 2) When starting up the board, the sensors will perform a calibration - make sure there is nothing with 14" of them
 3) The EZ0 does not compensate for temperature changes - probably not a big deal for the garage
 4) Used a Smoothing algorithm from - http://arduino.cc/en/Tutorial/Smoothing

*/


// Right and Left for the two garage bays
// Set the Timing
unsigned long pingTimer;
int pingRate = 100;  // Frequency of pings in ms 100ms is the fastest
//Define the Pins
const int ChainStart = 12;   // Triggers Right Sensor
const int LeftRange = A0; // Left Sensor Analog return - 5V opertaiotn - 9.8mV/in
const int RightRange = A1; // Right Sensor Analog return
const int RightAdjustmentPot=A2;  // First Adjustment Pot
const int LeftAdjustmentPot=A3; // Second Adjustment Pot
const int RightBlueLED=4; //Blue LED pin
const int RightGreenLED=3; //Green LED pin
const int RightRedLED=2; //Red LED pin
const int LeftBlueLED=7; //Blue LED pin
const int LeftGreenLED=6; //Green LED pin
const int LeftRedLED=5; //Red LED pin
//Define the variables for Range - Right and Left
const int MaxDistance = 320;  // Max distance in inches
const int MidDistance = 180;  // Mid-range distance in inches
const int MinDistance = 36;  // Min distance in inches
int RightInches;
int LeftInches;
int RightAdjustment = -9;  // Set manually if pots are not installed on board
int LeftAdjustment = -9;   // Set manually if pots are not installed on board
// This is the section with the smoothing variables
const int numReadings = 3;     // How Many numbers to average for smoothing
int index = 0;                  // the index of the current reading
int RightReadings[numReadings];      // the readings from the analog input
int RightTotal = 0;                  // the running total
int LeftReadings[numReadings];      // the readings from the analog input
int LeftTotal = 0;                  // the running total


void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
   // initialize all the smoothing array readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    RightReadings[thisReading] = 0;  
    LeftReadings[thisReading] = 0;
  }
  // Lots of output pins to set
  pinMode(RightRedLED,OUTPUT);
  pinMode(RightGreenLED, OUTPUT);
  pinMode(RightBlueLED, OUTPUT);
  pinMode(ChainStart,OUTPUT);
  pinMode(LeftRedLED,OUTPUT);
  pinMode(LeftGreenLED, OUTPUT);
  pinMode(LeftBlueLED, OUTPUT);
  // The first cycle is for calibration of the sensors - best to keep 14" free at this time
  digitalWrite(ChainStart,HIGH);  // Enable the calibration loop the rangefinder to ping
  delay(100);                    // Give each sensor 50ms to calibrate
  digitalWrite(ChainStart,LOW);  // Turn off the pinging
  // Read and set the adjustments for parking distances  - disabled if Pots are not installed on board
  // RightAdjustment = analogRead(RightAdjustmentPot);      // Read the Pot to see what the adjustment for parking is
  // RightAdjustment = map(RightAdjustment,0,1023,-12,12);  //  Scale the Input - adds or subtracts about a foot
  // LeftAdjustment = analogRead(LeftAdjustmentPot);      // Read the Pot to see what the adjustment for parking is
  // LeftAdjustment = map(LeftAdjustment,0,1023,-12,12);  //  Scale the Input - adds or subtracts about a foot
}

void loop()
{ 
  if (millis() >= (pingTimer + pingRate)) {
    digitalWrite(ChainStart,HIGH); // Start the chain of readings
    delay(20);   //20ms Pulse druation needed to start chaining in LV Series 
    digitalWrite(ChainStart,LOW);    // End the pulse for the Right Sensor - will go round the loop from here.
    // subtract the last reading:
    RightTotal= RightTotal - RightReadings[index];         
    LeftTotal= LeftTotal - LeftReadings[index];         
    // read from the sensor:  
    RightReadings[index] = analogRead(RightRange);      // Read the Pot to see what the adjustment for parking is
    // delay(50);
    LeftReadings[index] = analogRead(LeftRange);      // Read the Pot to see what the adjustment for parking is
    // add the reading to the total:
    RightTotal= RightTotal + RightReadings[index];       
    LeftTotal= LeftTotal + LeftReadings[index];       
    // advance to the next position in the array:  
    index = index + 1;                    
    // if we're at the end of the array...
    if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;                           
    // Take the Average and then scale (Maxbotix / Arduino Anaglog Range - 512/1024 so:
    RightInches = (RightTotal / numReadings) / 2;  //  Take the Average and then scale (Maxbotix / Arduino Anaglog Range - 512/1024
    LeftInches = (LeftTotal / numReadings) / 2;  //  Take the Average and then scale (Maxbotix / Arduino Anaglog Range - 512/1024
    pingTimer = millis();
  
    // Both sensors pinged, Print results here
    Serial.print("Raw Reading (Right / Left) ");
    Serial.print(analogRead(RightRange));
    Serial.print(" / ");
    Serial.print(analogRead(LeftRange));
    Serial.print("\t");
    Serial.print("Running Average (Right / Left) ");
    Serial.print(RightInches);
    Serial.print(" / ");
    Serial.println(LeftInches);
  }

  if (RightInches >= MaxDistance) {
    analogWrite(RightBlueLED,0);
    analogWrite(RightGreenLED,0);
    analogWrite(RightRedLED,0);
  }
  else if(RightInches < MaxDistance && RightInches > (MidDistance+RightAdjustment)) {
    analogWrite(RightBlueLED,255);
    analogWrite(RightGreenLED,0);
    analogWrite(RightRedLED,0);
  }
  else if (RightInches <= (MidDistance+RightAdjustment) && RightInches > (MinDistance+RightAdjustment)) {
    analogWrite(RightBlueLED,0);
    analogWrite(RightGreenLED,255);
    analogWrite(RightRedLED,0);
  }
  else if (RightInches <= (MinDistance+RightAdjustment)) {
    analogWrite(RightBlueLED,0);
    analogWrite(RightGreenLED,0);
    analogWrite(RightRedLED,255);
  }   
  if (LeftInches >= MaxDistance) {
    analogWrite(LeftBlueLED,0);
    analogWrite(LeftGreenLED,0);
    analogWrite(LeftRedLED,0);
  }
  else if(LeftInches < MaxDistance && LeftInches > (MidDistance+LeftAdjustment)) {
    analogWrite(LeftBlueLED,255);
    analogWrite(LeftGreenLED,0);
    analogWrite(LeftRedLED,0);
  }
  else if (LeftInches <= (MidDistance+LeftAdjustment) && LeftInches > (MinDistance+LeftAdjustment)) {
    analogWrite(LeftBlueLED,0);
    analogWrite(LeftGreenLED,255);
    analogWrite(LeftRedLED,0);
  }
  else if (LeftInches <= (MinDistance+LeftAdjustment)) {
    analogWrite(LeftBlueLED,0);
    analogWrite(LeftGreenLED,0);
    analogWrite(LeftRedLED,255);
  }   
}

