uint16_t scaleSelect(uint16_t input) {
  uint8_t value = map(input, 0, 1023, 0, 8);
  if (value >= (MAXTABLES)) {
    value = MAXTABLES - 1;
  }
  return (value);
}

uint16_t mapMaj(uint16_t input) {
  uint8_t value = input / 36;
  return (majTable[value]);
}

uint16_t mapMin(uint16_t input) {
  uint8_t value = input / 36;
  return (minTable[value]);
}

uint16_t mapPenta(uint16_t input) {
  uint8_t value = input / 42;
  return (pentaTable[value]);
}

uint16_t mapDorian(uint16_t input) {
  uint8_t value = input / 36;
  return (dorianTable[value]);
}

uint16_t mapMaj3rd(uint16_t input) {
  uint8_t value = input / 61;
  return (maj3rdTable[value]);
}

uint16_t mapMin3rd(uint16_t input) {
  uint8_t value = input / 59;
  return (min3rdTable[value]);
}

uint16_t mapWh(uint16_t input) {
  uint8_t value = input / 42;
  return (whTable[value]);
}

uint16_t mapChromatic(uint16_t input) {
  uint8_t value = input / 21;
  return (chromaTable[value]);
}

uint16_t mapMidi(uint16_t input) {
  return (midiTable[input]);
}

