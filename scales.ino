/*Quantizer

  Creative Commons License

  Quantizer by Pantala Labs is licensed under a
  Creative Commons Attribution 4.0 International License.
  Based on a work at https://github.com/PantalaLabs/Quantizer.

  Gibran Curtiss SalomÃ£o. MAY/2017 - CC-BY
*/

int  scaleSelect(int  input) {
  int  value = map(input, 0, 1023, 0, maxTables);
  if (value >= (maxTables)) {
    value = maxTables - 1;
  }
  return (value);
}

int  semitoneSelect(int  scaleInput, int  semitonesPerOctave) {
  int  value = map(scaleInput, 0, 1023, 0, semitonesPerOctave + 1);
  if (value >= (semitonesPerOctave)) {
    value = semitonesPerOctave;
  }
  return (value);
}

int  shiftNotes(int note, int shifts, int table[], int tableSize) {
  int noteIndex;
  Serial.println("tablesize");
  Serial.println(tableSize);
  
  for (int i = 0; i < tableSize; i++) {
    if (note == table[i]) {
      noteIndex = i + shifts;
      break;
    }
  }
  if (noteIndex > tableSize) {
    noteIndex = tableSize;
  }
  return (table[noteIndex]);
}


int  mapMaj(int  input) {
  int  value = input / 36;
  return (majTable[value]);
}

int  mapMin(int  input) {
  int  value = input / 36;
  return (minTable[value]);
}

int  mapPenta(int  input) {
  int  value = input / 42;
  return (pentaTable[value]);
}

int  mapDorian(int  input) {
  int  value = input / 36;
  return (dorianTable[value]);
}

int  mapMaj3rd(int  input) {
  int  value = input / 61;
  return (maj3rdTable[value]);
}

int  mapMin3rd(int  input) {
  int  value = input / 59;
  return (min3rdTable[value]);
}

int  mapWh(int  input) {
  int  value = input / 42;
  return (whTable[value]);
}

int  mapChromatic(int  input) {
  int  value = input / 21;
  return (chromaTable[value]);
}

int  mapMidi(int  input) {
  return (midiTable[input]);
}

int  deBounceMe(int  readCV, int  oldRead) {
  if (abs(readCV - oldRead) > debounceRange) {
    return readCV;
  }
  return oldRead;
}

