//Include Libraries
#include <Keyboard.h>   // Keyboard Emulation
#include <Encoder.h>    // Encoder controls
#include <Wire.h>   // I2C Communication
#include <SerLCD.h> // I2C LCD Screen control. 

// Encoder Defintions
const int pinEncoderA = 5;
const int pinEncoderB = 6;
Encoder delayEncoder(pinEncoderA, pinEncoderB);

// Trigger rleated pins
const int pinStart = A3;    // start Button input
const int pinExternalTrigger = A4;   // trigger signal when used as slave
const int pinSlaveArm = A5;
const int pinModeSelect = A2;    // Select master-slave mode
const int pinEnterKey = A1;  // rotary switch, Keyboard "return"
const int pinF5Key = 9;     // rotary switch, Keyboard "F5"
const int pinNoKey = 10;     // rotary switch, no Keyboard emul
const int pinUndefinedKey = 11; // rotary switch, not defined
const int pinFlash = 12;    // Signal sent to trigger flash


// Loop timing control
unsigned long currentMillis = 0;
unsigned long lastMillis = 0;
unsigned long updatePeriod = 200;

// Delay control
int nominalDelay = 0;
int actualDelay = 0;
const int delayCalCoef = 1;
const int delayCalConst = 0;
unsigned long delayStartTimeUnits = 0;
unsigned long delayCountTimeUnits = 0;

//Trigger Flags
bool flagMasterTrigger = false;
bool flagSlaveTrigger = false;
unsigned long lastTrigger = 0;

// Define screen
SerLCD lcd;

// Program Setup
void setup() {
  Serial.begin(9600);
    // Begin Keyboard emulation
    Keyboard.begin();
    // Set start button as an interrupt
    pinMode(pinStart, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinStart),TriggeredMaster, CHANGE);
    // set external trigger as interrupt
    pinMode(pinExternalTrigger, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinExternalTrigger),TriggeredSlave, CHANGE);
    // 
    pinMode(pinSlaveArm, INPUT_PULLUP);
    pinMode(pinEnterKey, INPUT_PULLUP);
    pinMode(pinF5Key, INPUT_PULLUP);
    pinMode(pinNoKey, INPUT_PULLUP);
    pinMode(pinUndefinedKey, INPUT_PULLUP);

    pinMode(pinFlash,OUTPUT);
    
    // Set up LCD screen
    Wire.begin();
    lcd.begin(Wire);
    lcd.setBacklight(255,255,255);
    lcd.setContrast(0);
    lcd.clear();

    //Pause for effect
    delay(2000);

    // Switch to interrupt
    pinMode(pinModeSelect, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinModeSelect),ChangeMode, CHANGE);      
    // Start delay display on screen
    if(digitalRead(pinModeSelect) == HIGH) {    //Master
        lcd.setCursor(0,0);
        lcd.print("MASTER DELAY ms");
    }
    else {  //Slave
        lcd.setCursor(0,0);
        lcd.print("SLAVE DELAY us");
    }
    lcd.setCursor(6,1);
}

// Main loop. Only handles setting of the delay
void loop() {
    if (flagMasterTrigger == true) {
        flagMasterTrigger = false;
        // Enter key selected on rotary switch
        if(digitalRead(pinEnterKey) == LOW) {
            Serial.println("Enter?");
            //Keyboard write
            Keyboard.press(KEY_RETURN);
            // Delay millis
            delayCountTimeUnits = millis();
            delayStartTimeUnits = millis();
            while ((delayCountTimeUnits - delayStartTimeUnits) <= actualDelay) {
                delayCountTimeUnits = millis();
            }
            digitalWrite(pinFlash, HIGH);
            delay(200);
            digitalWrite(pinFlash, LOW);
            Keyboard.releaseAll();
            delay(1000);
        }
        else if(digitalRead(pinF5Key) == LOW) {
          Serial.println("F5?");
            //Keyboard write
            Keyboard.press(KEY_F5);
            // Delay millis
            delayStartTimeUnits = millis();
            while ((delayCountTimeUnits - delayStartTimeUnits) < actualDelay) {
                delayCountTimeUnits = millis();
            }
            // Cycle line to flash high low. 
            digitalWrite(pinFlash, HIGH);
            delay(200);
            digitalWrite(pinFlash, LOW);
            Keyboard.releaseAll();
            delay(1000);
        }
        else if(digitalRead(pinNoKey) == LOW) {
            Serial.println("NOKEY?");
            // No Keyboard press
            // Cycle line to flash high low. 
            digitalWrite(pinFlash, HIGH);
            delay(200);
            digitalWrite(pinFlash, LOW);
            delay(1000);
        }
//        if(digitalRead(pinUndefinedKey) == LOW) {
//            Serial.println("UNDEF?");
//            // No Keyboard press
//            // Cycle line to flash high low. 
//            digitalWrite(pinFlash, HIGH);
//            delay(20);
//            digitalWrite(pinFlash, LOW);
//            delay(1000);
//        }
    }
    if (flagSlaveTrigger == true){
        flagSlaveTrigger = false;
        // Cycle line to flash high low. 
        delayStartTimeUnits = micros();
        while ((delayCountTimeUnits - delayStartTimeUnits) < actualDelay) {
            delayCountTimeUnits = micros();
        }
        digitalWrite(pinFlash, HIGH);
        delay(100);
        digitalWrite(pinFlash, LOW);
        delay(1000);
    }
    char lcdBuff[16];
    currentMillis = millis();
    if ((currentMillis-lastMillis) >= updatePeriod) {
        // update delay in 10 micro increments
        nominalDelay = nominalDelay + (delayEncoder.read()/4*10);
        if(nominalDelay <0) {
          nominalDelay = 0;
        }
        actualDelay = delayCalCoef*nominalDelay+delayCalConst;
        delayEncoder.write(0);   // reset buffer to zero
        lcd.setCursor(6,1);
        sprintf(lcdBuff,"%04d",nominalDelay);
        lcd.print(lcdBuff);
    }
}

// Interrupt service routine for button trigger
void TriggeredMaster() {
    // Switch in master mode (HIGH), execute. Do not execute in slave mode
    if ((millis() - lastTrigger) >= 4000) { //Kludge to prevent double fire
        if (digitalRead(pinModeSelect) == HIGH) {
            lastTrigger = millis();
            flagMasterTrigger=true;
        }
    }
}

// Interrupt service routine for slaved trigger
void TriggeredSlave() {
    if (digitalRead(pinSlaveArm) == HIGH){
        flagSlaveTrigger=true;
    }
}

void ChangeMode() {
    nominalDelay = 0;
    lcd.clear();
    if(digitalRead(pinModeSelect) == HIGH) {    //Master
        lcd.setCursor(0,0);
        lcd.print("MASTER DELAY ms");
    }
    else {  //Slave
        lcd.setCursor(0,0);
        lcd.print("SLAVE DELAY us");
    }
    lcd.setCursor(6,1);
}
