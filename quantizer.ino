/*Quantizer

  Creative Commons License

  Quantizer by Pantala Labs is licensed under a 
  Creative Commons Attribution 4.0 International License.
  Based on a work at https://github.com/PantalaLabs/Quantizer.

  Gibran Curtiss Salomão. MAY/2017 - ________
*/


#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#include <Wire.h>
#include <Adafruit_MCP4725.h>
Adafruit_MCP4725 dac;
#define DAC_RESOLUTION  (8)
#include <TimerOne.h>

#include "tables.h"

#define CV_IN1              A2 //input channel 1 
#define SCALE_SELECTOR      A3 //scaleSelect
#define MAXTABLES           8
#define DEBOUNCERANGE       19

#define clockInPin          3

boolean debugMode = false;

int   quantizedCV1    = -1;
int   debouncedCV1    = -1;
int   lastNotePlayed  = -1;
int   scaleRead       = 0;

unsigned long lastTick;
volatile boolean pleaseReadCv = false;


int deBounceMe(int readCV, int oldRead) {
  if (abs(readCV - oldRead) > DEBOUNCERANGE) {
    return readCV;
  }
  return oldRead;
}

void setup() {
  pinMode(clockInPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(clockInPin), tickInterrupt, RISING);
  dac.begin(0x62);
  if (debugMode) {
    Serial.begin(9600);
  }
  else {
    MIDI.begin();
  }
}

void tickInterrupt() {
  pleaseReadCv = true;
}

void loop() {
  if (pleaseReadCv) {
    pleaseReadCv = false;
    lastTick = millis();
    lastNotePlayed = quantizedCV1;
    scaleRead     = scaleSelect(analogRead(SCALE_SELECTOR));
    //    debouncedCV1  = deBounceMe(analogRead(CV_IN1), debouncedCV1);
    debouncedCV1  = analogRead(CV_IN1);
    switch (scaleRead) {
      case  0:
        quantizedCV1 = mapMaj(debouncedCV1);
        break;
      case  1:
        quantizedCV1 = mapMin(debouncedCV1);
        break;
      case  2:
        quantizedCV1 = mapPenta(debouncedCV1);
        break;
      case  3:
        quantizedCV1 = mapDorian(debouncedCV1);
        break;
      case  4:
        quantizedCV1 = mapMaj3rd(debouncedCV1);
        break;
      case  5:
        quantizedCV1 = mapMin3rd(debouncedCV1);
        break;
      case  6:
        quantizedCV1 = mapWh(debouncedCV1);
        break;
      case  7:
        quantizedCV1 = mapChromatic(debouncedCV1);
        break;
    }
    int voltageEquiv = map(mapMidi(quantizedCV1), 0, 61, 0, 4095);
    dac.setVoltage(voltageEquiv, false);
    if(lastNotePlayed>-1){
      MIDI.sendNoteOff(lastNotePlayed, 0, 1);     // Stop last note
    }
    MIDI.sendNoteOn(quantizedCV1, 127, 1);
    if (debugMode) {
      Serial.print(quantizedCV1);
      Serial.println(".");
    }
  }
}

void readClockSourceButton() {
//  if (digitalRead(clockInButtonPin) == HIGH) {
//    if (internalClock) {
//      //becomes EXTERNAL clock
//      internalClock = false;
//      oldTickInterval = tickInterval; //save the interval timer to restore if come back to internal clock
//      Timer1.stop();
//      Timer1.detachInterrupt();
//      attachInterrupt(digitalPinToInterrupt(clockInPin), tickInterrupt, RISING);
//      tickLast = millis();
//    } else {
//      //becomes INTERNAL clock
//      internalClock = true;
//      detachInterrupt(digitalPinToInterrupt(clockInPin));
//      tickInterval = oldTickInterval;
//      Timer1.initialize(tickInterval * 1000);
//      Timer1.attachInterrupt(tickInterrupt);
//      Timer1.restart();
//    }
//    Serial.println(internalClock);
//    markThisEvent();
//    interfaceDebounce = interfaceSuperDebounce;
//  }
}

