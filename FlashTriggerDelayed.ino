//Include Libraries
#include <Keyboard.h>   // Keyboard Emulation
#include <Encoder.h>    // Encoder controls
#include <Wire.h>   // I2C Communication
#include <SerLCD.h> //  I2C LCD Screen control. 

// Encoder Defintions
const int pinEncoderA = 2;
const int pinEncoderB = 3;
Encoder delayEncoder(pinEncoderA, pinEncoderB);

// Trigger rleated pins
const int pinStart = A3;    // start Button input
const int pinExternalTrigger = 5;   // trigger signal when used as slave
const int pinModeSelect = 6;    // Select master-slave mode
const int pinEnterKey = 7;  // rotary switch, keyboard "return"
const int pinF5Key = 8;     // rotary switch, keyboard "F5"
const int pinNoKey = 9;     // rotary switch, no keyboard emul
const int pinUndefinedKey = 10; // rotary switch, not defined
const int pinFlash = 11;    // Signal sent to trigger flash

// Loop timing control
unsigned long currentMillis = 0;
unsigned long lastMillis = 0;
unsigned long updatePeriod = 200;

// Delay control
int nominalDelay = 0;
int actualDelay = 0;
const int delayCalCoef = 1;
const int delayCalConst = 0;
unsigned long delayStartMicros = 0;
unsigned long delayCountMicros = 0;

// Define screen
SerLED lcd;

// Program Setup
void setup() {
    // Begin keyboard emulation
    keyboard.begin();
    // Set start button as an interrupt
    pinMode(pinStart, INPUT_PULLUP);
    attachInterrupt(ditigalPinToInterrupt(pinStart),TriggeredMaster, FALLING);
    // set external trigger as interrupt
    pinMode(pinExternalTrigger, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinExternalTrigger),TriggeredSlave, CHANGE);
    // Set up LCD screen
    Wire.begin();
    lcd.begin(Wire);
    lcd.setBacklight(255,255,255);
    lcd.setContrast(0);
    lcd.clear();

    //Pause for effect
    delay(2000);

    // Start delay display on screen
    lcd.setCursor(1,0);
    lcd.print("TRIGGER DELAY (uS)") ;
    lcd.setCursor(7,1);
}

// Main loop. Only handles setting of the delay
void loop() {
    currentMillis = millis()
    if {(currentMillis-lastMillis) >= updatePeriod) {
        // update delay in 10 micro increments
        nominalDelay += (delayEncoder.read()*10);
        actualDelay = delayCalCoef*nominalDelay+delayCalConst;
        delayEncoder.write(0);   // reset buffer to zero
        lcd.print(nominalDelay);
    }
}

// TODO: Figure out delay here. 
// Interrupt service routine for button trigger
void TriggeredMaster() {
    // Switch in master mode (HIGH), execute. Do not execute in slave mode
    if (digitalRead(pinSlaveMaster) == HIGH) {
        // Enter key selected on rotary switch
        if(digitalRead(pinEnterKey) == HIGH) {
            //keyboard write
            keyboard.press(KEY_RETURN);
            // Cycle line to flash high low. 
            digitalWrite(pinFlash) = HIGH;
            delay(100);
            digitalWrite(pinFlash) = LOW;
            keyboard.releaseAll();
            delay(1000);
        }
        if(digitalRead(pinF5Key) == HIGH) {
            //keyboard write
            keyboard.press(KEY_F5);
            // Cycle line to flash high low. 
            digitalWrite(pinFlash) = HIGH;
            delay(100);
            digitalWrite(pinFlash) = LOW;
            keyboard.releaseAll();
            delay(1000);
        }
        if(digitalRead(pinNoKey) == HIGH) {
            // No keyboard press
            // Cycle line to flash high low. 
            digitalWrite(pinFlash) = HIGH;
            delay(100);
            digitalWrite(pinFlash) = LOW;
            delay(1000);
        }
    }
}

// Interrupt service routine for slaved trigger
void TriggeredSlave() {
    // Signal from slave trigger will always trigger flash, regardless of switch
    // No keyboard press
    // Cycle line to flash high low. 
    delayStartMicros = micros();
    while ((delayCountMicros - delayStartMicros) < triggerDelay) {
        delayCountMicros = micros();
    }
    digitalWrite(pinFlash) = HIGH;
    delay(100);
    digitalWrite(pinFlash) = LOW;
    delay(1000);
}