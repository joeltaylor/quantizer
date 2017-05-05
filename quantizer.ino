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
int  clockInPin         = 3;
int  ledPin             = 8;

//analogic pins
int  cvIn1Pin           = 2;
int  semitoneShiftPin   = 5;
int  octaveShiftPin     = 6;
int  scaleSelectorPin   = 7;

int octaveSize          = 0;
int maxTables           = 8;
int debounceRange       = 19;
int lastNotePlayed      = 0;
int tempNote;
unsigned long lastTrigger;

volatile boolean pleaseReadCv = false;
boolean triggerOn = false;

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
  int debouncedReadCV1  = 0;
  int scaleRead         = 0;
  int semitoneRead      = 0;
  int octaveOffset      = 0;
  int notesToShift      = 0;

  if (triggerOn && (millis() > (lastTrigger + 10))) {
    triggerOn = false;
    digitalWrite(ledPin, LOW);
  }

  if (pleaseReadCv) {
    lastTrigger     = millis();
    pleaseReadCv    = false;
    triggerOn       = true;
    lastNotePlayed  = quantizedNoteCV1;
    debouncedReadCV1    = analogRead(cvIn1Pin);
    scaleRead           = scaleSelect(analogRead(scaleSelectorPin));
    octaveOffset        = map(analogRead(octaveShiftPin), 0, 1023, 0, 4);
    semitoneRead        = analogRead(semitoneShiftPin);

    switch (scaleRead) {
      case  0:
        octaveSize = 7;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapMaj(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, majTable, sizeof(majTable));
        break;
      case  1:
        octaveSize = 7;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapMin(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, minTable, sizeof(minTable));
        break;
      case  2:
        octaveSize = 6;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapPenta(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, pentaTable, sizeof(pentaTable));
        break;
      case  3:
        octaveSize = 7;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapDorian(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, dorianTable, sizeof(dorianTable));
        break;
      case  4:
        octaveSize = 4;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapMaj3rd(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, maj3rdTable, sizeof(maj3rdTable));
        break;
      case  5:
        octaveSize = 4;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapMin3rd(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, min3rdTable, sizeof(min3rdTable));
        break;
      case  6:
        octaveSize = 4;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapWh(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, whTable, sizeof(whTable));
        break;
      case  7:
        octaveSize = 12;
        semitoneRead = map(semitoneRead, 0, 1023, 0, octaveSize);
        tempNote = mapChromatic(debouncedReadCV1);
        notesToShift = (octaveOffset * octaveSize) + semitoneRead;
        quantizedNoteCV1 = shiftNotes(tempNote, notesToShift, chromaTable, sizeof(chromaTable));
        break;
    }
    // CVOUT
    //    int  voltageEquivalent = map(mapMidi(quantizedNoteCV1), 0, 60, 0, 4095);
    //    dac.setVoltage(voltageEquivalent, false);

    int myNote = quantizedNoteCV1;
    MIDI.sendNoteOff(lastNotePlayed, 0, 1);
    MIDI.sendNoteOn(myNote, 127, 1);


    if (debugMe) {
      
      Serial.print(" scaleRead: ");
      Serial.print(scaleRead);
      Serial.print(" tempNote: ");
      Serial.print(tempNote);
      Serial.print(" shift: ");
      Serial.print(notesToShift);
      String strg;
      strg = quantizedNoteCV1;
      Serial.print("quantized: ");
      Serial.print(strg);
      Serial.println(" .");
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


