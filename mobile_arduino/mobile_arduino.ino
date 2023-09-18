#include <SoftwareSerial.h>

SoftwareSerial SUART(4, 5);  // RX, TX
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {A0, A1, A2, A3};
byte pin_column[COLUMN_NUM] = {A4, A5, A6, A7};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27, 16, 2);

char currentStage = 'W'; // Start with Welcome stage

Servo servo1;
Servo servo2;
#define SERVO1_PIN 28  // choose appropriate pins
#define SERVO2_PIN 29   // choose appropriate pins

// Define the sensor pins
#define sensor1Pin 6
#define sensor2Pin 7
#define sensor3Pin 8
#define sensor4Pin 9
#define sensor5Pin 10
#define ir6Pin 50
#define ir7Pin 51
#define ir8Pin 34
#define irBasePin 53

const int pumpPins[] = {38, 35, 39, 36, 37}; // Replace with your actual pump pin numbers

#define relay 11
#define relay2 12
#define relaydc1 13
#define relaydc2 52
 
int count = 0;
char command;

// ultra 1
#define ultra1Trig 40
#define ultra1Echo 41
//ultra 2
#define ultra2Trig 42
#define ultra2Echo 43
// ultra 3
#define ultra3Trig 44
#define ultra3Echo 45
// ultra 4
#define ultra4Trig 46
#define ultra4Echo 47
//ultra 5
#define ultra5Trig 48
#define ultra5Echo 49

#define NUM_SENSORS 5

int trigPins[NUM_SENSORS] = {40, 42, 44, 46, 48};  // Replace with your trig pins
int echoPins[NUM_SENSORS] = {41, 43, 45, 47, 49};  // Replace with your echo pins

int durations[NUM_SENSORS]; // Add this line to declare durations array
int distances[NUM_SENSORS]; // Add this line to declare distances array
bool executeForLoop = true;

// Define the switch pins
#define startSwitchPin 26
#define endSwitchPin 24

// Define the motor control pins
const int dirPin = 3;
const int stepPin = 4;
const int enPin = 5;

// Define the delay (in microseconds) between each step to control the motor speed
const int stepDelay = 1000; // Adjust the delay to set the motor speed

// Define the states for the state machine
enum State {
  MOVING_FORWARD,
  WAITING_AT_CHART_DETECTION,
  WAITING_AT_END_SWITCH,
  MOVING_IN_REVERSE,
  MOVE,
  PUMP1,
  PUMP2,
  PUMP3,
  PUMP4,
  PUMP5,
  STOP_AT_IR6,
  STOP_AT_IR7,
  STOP_AT_IR8,
  Servo2,
  INNER_DONE  
};
enum perfume {
  start,
  idle,
Acqua ,
Prada ,
Miss ,
Vctoria,
Oud ,
Sakura 
};

perfume per = start;
// Initialize the state machine to the IDLE state
State currentState = MOVE;

// Store the time when the chart was detected
unsigned long chartDetectedTime = 0;

// Define the delay (in milliseconds) after chart detection
const unsigned long chartDetectionDelay = 5000; // 5 seconds

// Chart detection flag and time variables
bool chartDetected = false;

unsigned long chartDetectionStartTime = 0;

// Sensor states array
bool sensorStates[8] = {false, false, false, false, false, false, false, false};
bool pumpStates[5]={false, false, false, false, false};

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  SUART.begin(9600);  // start software serial
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  pinMode(sensor3Pin, INPUT_PULLUP);
  pinMode(sensor4Pin, INPUT_PULLUP);
  pinMode(sensor5Pin, INPUT_PULLUP);
  pinMode(ir6Pin, INPUT_PULLUP);
  pinMode(ir7Pin, INPUT_PULLUP);
  pinMode(ir8Pin, INPUT_PULLUP);
  pinMode(irBasePin, INPUT_PULLUP);

  pinMode(startSwitchPin, INPUT_PULLUP);
  pinMode(endSwitchPin, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
    digitalWrite(relay, HIGH);  // Assuming LOW keeps the relay2 OFF initially.
  lcd.init();
  lcd.backlight();
  lcd.print("Perfumix Machine");
  delay(2000);  // Wait for 2 seconds
  lcd.clear();
  lcd.print("Men 1, Women 2,");
  lcd.setCursor(0, 1);
  lcd.print("Unisex 3");

  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay2, HIGH);  // Assuming LOW keeps the relay2 OFF initially.
  pinMode(relaydc1, OUTPUT);
  digitalWrite(relaydc1, HIGH);  // Assuming LOW keeps the relay OFF initially.
  pinMode(relaydc2, OUTPUT);
  digitalWrite(relaydc2, HIGH);  // Assuming LOW keeps the relay OFF initially.
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo1.write(180);
  servo2.write(180);
  for (int i = 0; i < 5; i++) {
      pinMode(pumpPins[i], OUTPUT);
      digitalWrite(pumpPins[i], HIGH); // Turn off all pumps initially
    }
    for(int i=0; i<NUM_SENSORS; i++) {
        pinMode(trigPins[i], OUTPUT);
        pinMode(echoPins[i], INPUT);
    }
}

void loop() { 
    char key = keypad.getKey();

    if (executeForLoop) {

for (int i = 0; i < NUM_SENSORS; i++) {
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);

    durations[i] = pulseIn(echoPins[i], HIGH);
    distances[i] = durations[i] * 0.0343 / 2;

 

    if (distances[i] == 5 || distances[i] == 6) {  // Corrected the logical error
        Serial.print("SENSOR:"); 
        Serial.print(i);
        Serial.print(",DISTANCE:");
        Serial.println(distances[i]);

        // Send the data to ESP over Serial2
        Serial2.print("SENSOR:"); 
        Serial2.print(i);
        Serial2.print(",DISTANCE:");
        Serial2.println(distances[i]);
        Serial2.println(""); 
        delay(100);  
    }
}

    }
        executeForLoop = false;
  if (key) {
    if (currentStage == 'W') {
      if (key == '1' || key == '2' || key == '3') {
        lcd.clear();
        switch (key) {
          case '1':
            lcd.print("Men: Acqua 1,");
            lcd.setCursor(0, 1);
            lcd.print("Prada 2");
            currentStage = '1';
            break;
          case '2':
            lcd.print("Women: Dior 1,");
            lcd.setCursor(0, 1);
            lcd.print("Vctoria  2");
            currentStage = '2';
            break;
          case '3':
            lcd.print("Unisex: Oud 1,");
            lcd.setCursor(0, 1);
            lcd.print("Sakura 2");
            currentStage = '3';
            break;
        }
      }
    } else if (currentStage == '1' && (key == '1' || key == '2')) {
      // Handle the case when the user has selected Men's perfume.
      lcd.clear();
      if (key == '1') {
        lcd.print("You chose Acqua");
        per = Acqua;
      } else {
        lcd.print("You chose Prada");
        per = Prada;
      }
      delay(2000);
      resetStage();
    } else if (currentStage == '2' && (key == '1' || key == '2')) {
      // Handle the case when the user has selected Women's perfume.
      lcd.clear();
      if (key == '1') {
        lcd.print("You chose Dior");
        per = Miss;
      } else {
        lcd.print("You chose Vctoria");
        per = Vctoria;
      }
      delay(2000);
      resetStage();
    } else if (currentStage == '3' && (key == '1' || key == '2')) {
      // Handle the case when the user has selected Unisex perfume.
      lcd.clear();
      if (key == '5') {
        lcd.print("You chose Oud");
        per = Oud;
      } else {
        lcd.print("You chose Sakura");
        per = Sakura ;
      }
      delay(2000);
      resetStage();
    }
  }

  switch(per){
  case start:
   if (digitalRead(startSwitchPin)==LOW && !digitalRead(irBasePin)==HIGH) {
      Serial.println("enter a perfume");
      delay(5000);
         per = idle;
        
      }
      else{
        Serial.println("Waiting");
      }
    break;
 
    case idle :
    if (Serial2.available()>0) {
      command = Serial2.read();
      
     if (command == 'M') {
      Serial.println("Mixing Miss Dior!");
      per = Miss; 
     }
     if (command == 'A') {
      Serial.println("Acqua_Di_Gio");
      per =Acqua;
     }
      else if(command == 'P'){
      Serial.println("Prada_Black");
      per = Prada;
    }
    else if(command == 'V'){
     Serial.println("Vctoria's_Secret");
     per = Vctoria;
    }
    else if(command == 'O'){
     Serial.println("Oud_Robert_Piguet");
     per = Oud;
    }
    else if(command == 'S'){
     Serial.println("Sakura_Christian_Dior");
     per = Sakura;
    }
    }
break;
    case Miss:
    switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 2:
                detected = !digitalRead(sensor3Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 3");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;

  case Acqua :
    switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 0:
                detected = !digitalRead(sensor1Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 1");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;

  case Prada :
     switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 1:
                detected = !digitalRead(sensor2Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 2");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;

 case Vctoria:
    switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 3:
                detected = !digitalRead(sensor4Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 4");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;
   case Oud:
     switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 0:
                detected = !digitalRead(sensor1Pin);
                if (detected) {   
                  count = 1;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 1");
                }
                break;
              case 2:
                detected = !digitalRead(sensor3Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 3");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;
  
  case Sakura:
    switch (currentState) {
    case MOVE:
    
     for (int i = 0; i < 5; i++) {
      pumpStates[i] = false;
    }

    digitalWrite(dirPin, LOW); // Set the motor direction forward
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    Serial.println("idle");

    // Check for chart detection
    if (!chartDetected) {
        for (int i = 0; i < 8; i++) {
            // Check the corresponding sensor for detection
            if (!sensorStates[i]) {
                bool detected = false;
            switch (i) {
              case 1:
                detected = !digitalRead(sensor2Pin);
                if (detected) {   
                  count = 1;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 2");
                }
                break;
              case 3:
                detected = !digitalRead(sensor4Pin);
                if (detected) {   
                  count = 2;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 4");
                }
                break;
              case 4:
                detected = !digitalRead(sensor5Pin);
                if (detected) {
                  count = 5;
                  chartDetected = true;
                  pumpStates[i] = true;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 5");
                }
                break;
              case 5:
                detected = !digitalRead(ir6Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 6");
                  currentState = STOP_AT_IR6;
                }
                break;
              case 6:
                detected = !digitalRead(ir7Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 7");
                  currentState = STOP_AT_IR7;
                }
                break;
              case 7:
                detected = !digitalRead(ir8Pin);
                if (detected) {
                  chartDetected = false;
                  chartDetectionStartTime = millis();
                  sensorStates[i] = true;
                  Serial.println("read sensor 8");
                  currentState = STOP_AT_IR8;
                }
                break;
            }
            
          }
        }
      }

      // Check if chart detection has occurred
      if (chartDetected) {
        currentState = MOVING_FORWARD;
        break;
      }
      if (digitalRead(endSwitchPin) == LOW) {
        currentState = WAITING_AT_END_SWITCH;
        break;
      }
      break;
    
          case MOVING_FORWARD:
      // The chart is detected, stop the motor
      Serial.println("stop sensor detect");
      digitalWrite(stepPin, LOW); // Stop the motor
      chartDetected = false; // Reset the chartDetected flag for the next chart detection
      currentState = WAITING_AT_CHART_DETECTION; // Move to the state waiting for the specified delay
      chartDetectedTime = millis(); // Record the time when the chart is detected
      break;

  case WAITING_AT_CHART_DETECTION:
    for (int i = 0; i < count; i++) {
        digitalWrite(relay, LOW); // turn the relay ON
        delay(5000);

      if (pumpStates[0]){
        Serial.println("PUMP1 ON");
        digitalWrite(pumpPins[0], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[0], HIGH); // Turn off PUMP1
        Serial.println("PUMP1 OFF");
      }      
      else if (pumpStates[1]){
        Serial.println("PUMP2 ON");
        digitalWrite(pumpPins[1], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[1], HIGH); // Turn off PUMP1
        Serial.println("PUMP2 OFF");
     }      
     else if (pumpStates[2]){
        Serial.println("PUMP3 ON");
        digitalWrite(pumpPins[2], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[2], HIGH); // Turn off PUMP1
        Serial.println("PUMP3 OFF");
     }      
      else if (pumpStates[3]){
        Serial.println("PUMP4 ON");
        digitalWrite(pumpPins[3], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[3], HIGH); // Turn off PUMP1
        Serial.println("PUMP4 OFF");
      }      
      else if (pumpStates[4]){
        Serial.println("PUMP5 ON");
        digitalWrite(pumpPins[4], LOW); // Turn PUMP1 ON
        delay(1000); // Run the pump for 1 second
        digitalWrite(pumpPins[4], HIGH); // Turn off PUMP1
        Serial.println("PUMP5 OFF");
      }           
        digitalWrite(relay, HIGH); // turn the relay OFF
        delay(5000);
    }

    // Check for the time delay before moving to the next state
    if (millis() - chartDetectedTime >= chartDetectionDelay) {
        currentState = MOVE;
    }
    break;

      case STOP_AT_IR6:
      delay(2000);
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relay2, LOW);  // Turn on relay2
      delay(3000);
      digitalWrite(relaydc1, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc on");
      delay(5000); // Wait for 5 seconds
      digitalWrite(relaydc1, HIGH); // Turn off the DC motor
      delay(3000);
      digitalWrite(relay2, HIGH);  // Turn off relay2
      delay(6000);
      currentState = MOVE; // Transition back to the IDLE state
      break;

case STOP_AT_IR7:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      servo2.write(90);  // move the servo1 to 90 degrees
      delay(1000);  // delay for 1 second
      servo2.write(180);  // move the servo1 back to 0 degrees
      Serial.println("servo 1 run");
      delay(1000); // Give a delay between the two servo operations to make them distinctly noticeable
      servo1.write(90);  // move the servo2 to 90 degrees
      delay(1000);  // delay for 1 second
      servo1.write(180);  // move the servo2 back to 0 degrees
      Serial.println("servo 2 run");

      currentState = MOVE;
      break;  

      case STOP_AT_IR8:
      digitalWrite(stepPin, LOW); // Stop the stepper motor
      digitalWrite(relaydc2, LOW); // Assuming HIGH turns the DC motor ON. Adjust if it's the opposite for your setup.
      Serial.println("dc2 on");
      delay(15000); // Wait for 5 seconds
      digitalWrite(relaydc2, HIGH); // Turn off the DC motor
      currentState = MOVE; // Transition back to the IDLE state
      break;

      case WAITING_AT_END_SWITCH:
      if (digitalRead(endSwitchPin) == LOW) {
        delay(3000);
        currentState = MOVING_IN_REVERSE;
      }
      break;

    case MOVING_IN_REVERSE:
      Serial.println("REVESE");
      digitalWrite(dirPin, HIGH); // Set the motor direction reverse
      while (digitalRead(startSwitchPin) == HIGH) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      // The start switch is pressed again, stop the motor
      for (int i = 0; i < 8; i++) {
        sensorStates[i] = false; // Reset the sensor states for the next movement
      }
      currentState = INNER_DONE;
      break;

    case INNER_DONE:
            executeForLoop = true;
    per = start;  // Transition to the start state of the outer state machine.
    currentState = MOVE;  // Reset the inner state machine to MOVE.
    break;
  
  }
  break;

  }
  
}
  void resetStage() {
  currentStage = 'W';
  lcd.clear();
  lcd.print("Men 1, Women 2,");
  lcd.setCursor(0, 1);
  lcd.print("Unisex 3");
}



