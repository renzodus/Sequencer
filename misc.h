void latchedShiftOut_16bits (int data, int strobe, int clock, bool seq[16]) {
    digitalWrite(data, 0);
    for (int i = 0; i < 16; i++) {
        digitalWrite(data, seq[i]);
        digitalWrite(clock, 1);
        digitalWrite(clock, 0);
    }
    digitalWrite(strobe, 1);
    digitalWrite(strobe, 0);
    digitalWrite(data, 0);
}

String translateNoteName (int note) {
    int oct = note/12;
    int rest;
    String name;
    if (note == 0) rest = 0;
    else rest = note%12;
    
    switch (rest) {
        case 0: name = "C"; break; 
        case 1: name = "C#"; break;
        case 2: name = "D"; break;
        case 3: name = "D#"; break;
        case 4: name = "E"; break;
        case 5: name = "F"; break;
        case 6: name = "F#"; break;
        case 7: name = "G"; break;
        case 8: name = "G#"; break;
        case 9: name = "A"; break;
        case 10: name = "A#"; break;
        case 11: name = "B"; break;
    }

    return name + String(oct);
}

String translateSequenceDirection (int direction) {
    switch (direction){
        case 0: return "->";
        case 1: return "<-";
        case 2: return "<>";
    }
}

int keyToNote(int key) {
    switch (key) {
        case 1: return 1;
        case 2: return 3;
        case 4: return 6;
        case 5: return 8;
        case 6: return 10;
        case 8: return 0;
        case 9: return 2;
        case 10: return 4;
        case 11: return 5;
        case 12: return 7;
        case 13: return 9;
        case 14: return 11;
        case 15: return 12;
    }
}

int setValue (int value, int minimum, int maximum) {
    if (digitalRead(encoderCLK) == 1) value++;
    else value--;
    return min(maximum, max(minimum, value));
}