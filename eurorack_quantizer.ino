/*
  Creative Commons License
  Quantizer by Pantala Labs is licensed under a
  Creative Commons Attribution 4.0 International License.
  Gibran Curtiss SalomÃ£o. MAY/2017 - CC-BY-SA
*/
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
Adafruit_MCP4725 dac;
//#define DAC_RESOLUTION (12)

boolean debugMe = false;

#define TRIGGERPIN 2
#define QTZPIN 3
#define CVINPIN A6
#define ZEROEQUIVALENT 4
#define FOUROCTAVES_MIDI59_4916V 4084

volatile boolean pleaseQuantizeMe;
int lastNote = 0;

void setup()
{

  pinMode(QTZPIN, INPUT);
  pinMode(TRIGGERPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TRIGGERPIN), tickInterrupt, RISING);
  dac.begin(0x62);
  dac.setVoltage(0, false);
  if (debugMe)
    Serial.begin(9600);
  else
    MIDI.begin();

  // for (uint16_t v = 3900; v < 4095; v++)
  // {
  //   Serial.println(v);
  //   dac.setVoltage(v, false);
  //   delay(1000);
  // }
}

void tickInterrupt()
{
  pleaseQuantizeMe = true;
}

void loop()
{
  uint16_t cvRead = analogRead(CVINPIN);
  if (pleaseQuantizeMe)
  {
    pleaseQuantizeMe = false;
    uint16_t noteQuantized;
    uint16_t voltageQuantized;
    noteQuantized = constrain(map(cvRead, 0, 1023, 0, 60), 0, 59); //read 5 volts but constrain to 4.911V = 4 octaves
    if (digitalRead(QTZPIN))
      voltageQuantized = map(noteQuantized, 0, 59, ZEROEQUIVALENT, FOUROCTAVES_MIDI59_4916V);
    else
      voltageQuantized = cvRead << 2;
    MIDI.sendNoteOff(lastNote, 0, 1);
    MIDI.sendNoteOn(noteQuantized, 127, 1);
    dac.setVoltage(voltageQuantized, false);
    lastNote = noteQuantized;
  }
}
