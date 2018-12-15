/*
  Creative Commons License
  Quantizer by Pantala Labs is licensed under a
  Creative Commons Attribution 4.0 International License.
  Gibran Curtiss SalomÃ£o. MAY/2017 - CC-BY-SA
*/
#include <Wire.h>
#include <Adafruit_MCP4725.h>
Adafruit_MCP4725 dac;
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define TRIGGERPIN  2
#define QTZOPTION   3
#define CVINPIN     A6
boolean pleaseQuantizeMe;
int     lastNote = 0;
int     cvRead;
int     noteQuantized;
int     voltageQuantized;

boolean debugMe = false;

void setup() {
  pinMode(TRIGGERPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TRIGGERPIN), tickInterrupt, FALLING);
  pinMode(QTZOPTION, INPUT);
  if (debugMe) {
    Serial.begin(9600);
  }
  else {
    dac.begin(0x62);
    MIDI.begin();
  }
}

void tickInterrupt() {
  pleaseQuantizeMe = true;
}

void loop() {
  if (pleaseQuantizeMe) {
    pleaseQuantizeMe = false;
    cvRead = analogRead(CVINPIN);
    noteQuantized = map(cvRead, 0, 1023, 0, 60);
    if (digitalRead(QTZOPTION)) {
      voltageQuantized = map(noteQuantized, 0, 60, 0, 4095);
    } else {
      voltageQuantized = cvRead << 2;
    }
    MIDI.sendNoteOff(lastNote, 0, 1);
    MIDI.sendNoteOn(noteQuantized, 127, 1);
    dac.setVoltage(voltageQuantized, false);
    lastNote = noteQuantized;
  }
}

