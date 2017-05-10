/*Quantizer

  Creative Commons License

  Quantizer by Pantala Labs is licensed under a
  Creative Commons Attribution 4.0 International License.
  Based on a work at https://github.com/PantalaLabs/Quantizer.

  Gibran Curtiss SalomÃ£o. MAY/2017 - CC-BY
*/
#include <Wire.h>
#include <Adafruit_MCP4725.h>
Adafruit_MCP4725 dac;
#define DAC_RESOLUTION  (8)
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

//digital pins
#define clockInPin          3
#define ledPin              8

//analogic pins
#define cvIn1Pin            A0
#define cvIn2Pin            A1
#define semitoneShiftPin    A5
#define octaveShiftPin      A6
#define scaleSelectorPin    A7

#define maxTables           8
#define debounceRange       20

int octaveSize          = 0;
int lastNotePlayed      = 0;
int tempNote;

unsigned long lastTrigger;

volatile boolean pleaseReadCv = true;
boolean         triggerOn = false;

boolean debugMe = false;

#include "tables.h"

void setup() {
  attachInterrupt(digitalPinToInterrupt(clockInPin), tickInterrupt, RISING);
  dac.begin(0x62);
  pinMode(ledPin, OUTPUT);
  pinMode(clockInPin, INPUT);
  if (debugMe) {
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
  int quantizedNoteCV1  = 0;
//  int cv2Offset         = 0;
  int debouncedReadCV1  = 0;
  int debouncedReadCV2  = 0;
  int scaleRead         = 0;
  int octaveOffset      = 0;
  int semitoneOffset    = 0;
  int notesToShift      = 0;

  if (triggerOn && (millis() > (lastTrigger + 20))) {
    triggerOn = false;
    digitalWrite(ledPin, LOW);
  }

  if (pleaseReadCv) {
    lastTrigger     = millis();
    pleaseReadCv    = false;
    triggerOn       = true;
    lastNotePlayed  = quantizedNoteCV1;
    debouncedReadCV1    = softDebounce(analogRead(cvIn1Pin), debouncedReadCV1); 
    debouncedReadCV2    = softDebounce(analogRead(cvIn2Pin), debouncedReadCV2); 
    scaleRead           = scaleSelect(softDebounce(analogRead(scaleSelectorPin), scaleRead));
    octaveOffset        = octaveSelect(softDebounce(analogRead(octaveShiftPin), octaveOffset));
    switch (scaleRead) {
      case  0:
        octaveSize = 12;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote  = mapChromatic(debouncedReadCV1+debouncedReadCV2);
        //cv2Offset = noteSelect(mapChromatic(debouncedReadCV2), chromaTable, sizeof(chromaTable));
        //notesToShift = cv2Offset + (octaveOffset * octaveSize) + semitoneOffset;
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, chromaTable, sizeof(chromaTable));
        break;
      case  1:
        octaveSize = 7;
        semitoneOffset = semitoneSelect(softDebounce(analogRead(semitoneShiftPin), semitoneOffset), octaveSize);
        tempNote = mapMaj(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, majTable, sizeof(majTable));
        break;
      case  2:
        octaveSize = 7;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapMin(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, minTable, sizeof(minTable));
        break;
      case  3:
        octaveSize = 6;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapPenta(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, pentaTable, sizeof(pentaTable));
        break;
      case  4:
        octaveSize = 7;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapDorian(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, dorianTable, sizeof(dorianTable));
        break;
      case  5:
        octaveSize = 4;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapMaj3rd(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, maj3rdTable, sizeof(maj3rdTable));
        break;
      case  6:
        octaveSize = 4;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapMin3rd(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, min3rdTable, sizeof(min3rdTable));
        break;
      case  7:
        octaveSize = 4;
        semitoneOffset = semitoneSelect(analogRead(semitoneShiftPin), octaveSize);
        tempNote = mapWh(debouncedReadCV1+debouncedReadCV2);
        notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift , whTable, sizeof(whTable));
        break;
    }

    // CVOUT
    //    quantizedNoteCV1 = (quantizedNoteCV1 > 60) ? 60 : quantizedNoteCV1;
    if (quantizedNoteCV1 > 60) {
      quantizedNoteCV1 = 60;
    }
    if (quantizedNoteCV1 < 0) {
      quantizedNoteCV1 = 0;
    }
    int  voltageEquivalent = map(mapMidi(quantizedNoteCV1), 0, 60, 2, 4095);
    dac.setVoltage(voltageEquivalent, false);


    MIDI.sendNoteOff(lastNotePlayed, 0, 1);
    MIDI.sendNoteOn(quantizedNoteCV1, 127, 1);

    if (debugMe) {
      Serial.print("Scale:");
      Serial.print(scaleRead);
      Serial.print(" Note:");
      Serial.print(tempNote);
      Serial.print(" Oct:");
      Serial.print(octaveOffset);
      Serial.print(" Semi:");
      Serial.print(semitoneOffset);
      Serial.print(" Shift:");
      Serial.print(notesToShift);
      String strg = String(quantizedNoteCV1);
      Serial.print(" Quant:");
      Serial.print(strg);
      Serial.print(" DAC:");
      Serial.print(voltageEquivalent);
      Serial.println(".");
    }
    digitalWrite(ledPin, HIGH);
  }
}




//function copied from another project. NOT WORKING
//void readClockSourceButton() {
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
//}


