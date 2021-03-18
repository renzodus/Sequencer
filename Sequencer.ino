#include <CapacitiveSensor.h>
#include <LiquidCrystal_SR3W.h>
#include <Type4067Mux.h>

#include "pins.h"

// Sequencer
int channels = 4;
bool playing[4] = {false};
byte steps[4] = {16, 16, 16, 16};
byte step[4] = {0, 0, 0, 0};
// Secuencia, 0-15 banco 0 , 16-31 banco 1
byte sequence[4][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 12, 12, 12, 12, 12, 12, 0, 0, 0, 0, 12, 12, 12, 12, 24, 24, 24, 24, 36, 36, 36, 36},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
bool sequenceGate[4][32] = {{0}, {0}, {0}, {0}};
byte sequenceDirection[4] = {0};        // 0: Forward, 1: Backwards, 2: Back & Forth
bool isSequenceMovingForward[4] = {1};
bool legato[4] = {0};
// Modes (0: Sequencer, 1: Keyboard, 2: Arpeggiator, 3: MIDI controller)
byte mode[4] = {0, 1, 0, 0};
byte activeBank[4] = {0, 0, 0, 0};
byte activeChannel = 0;
byte cvWrite[4] = {0, 0, 0, 0};
bool gateWrite[4] = {0, 0, 0, 0};
int tempo = 120;
unsigned long preMillis = 0;
unsigned long ms = millis();

// Controls
// Encoder 0: nothing, 1: tempo, 2: Mode, 3: sequence notes, 4: sequence direction
int encoderControl = 0;
bool seqGateControl = 1;                // true: set gate, false: set note
int selectedStep = 0;                   // Selected step to set
int seqSensorRead[16] = {0};
bool seqSensorGate[16] = {0};
bool seqSensorGatePrev[16] = {0};
bool seqSensorTrig[16] = {0};
int seqTriggerFactor = 700;
int optionSensorRead[10] = {0};
bool optionSensorGate[10] = {0};
bool optionSensorGatePrev[10] = {0};
bool optionSensorTrig[10] = {0};
int optionTriggerFactor = 700;
CapacitiveSensor capSensor[26] = {
    CapacitiveSensor(capSensorSend, seqSensor00In),
    CapacitiveSensor(capSensorSend, seqSensor01In),
    CapacitiveSensor(capSensorSend, seqSensor02In),
    CapacitiveSensor(capSensorSend, seqSensor03In),
    CapacitiveSensor(capSensorSend, seqSensor04In),
    CapacitiveSensor(capSensorSend, seqSensor05In),
    CapacitiveSensor(capSensorSend, seqSensor06In),
    CapacitiveSensor(capSensorSend, seqSensor07In),
    CapacitiveSensor(capSensorSend, seqSensor08In),
    CapacitiveSensor(capSensorSend, seqSensor09In),
    CapacitiveSensor(capSensorSend, seqSensor10In),
    CapacitiveSensor(capSensorSend, seqSensor11In),
    CapacitiveSensor(capSensorSend, seqSensor12In),
    CapacitiveSensor(capSensorSend, seqSensor13In),
    CapacitiveSensor(capSensorSend, seqSensor14In),
    CapacitiveSensor(capSensorSend, seqSensor15In),
    CapacitiveSensor(capSensorSend, optionSensor0In),
    CapacitiveSensor(capSensorSend, optionSensor1In),
    CapacitiveSensor(capSensorSend, optionSensor2In),
    CapacitiveSensor(capSensorSend, optionSensor3In),
    CapacitiveSensor(capSensorSend, optionSensor4In),
    CapacitiveSensor(capSensorSend, optionSensor5In),
    CapacitiveSensor(capSensorSend, optionSensor6In),
    CapacitiveSensor(capSensorSend, optionSensor7In),
    CapacitiveSensor(capSensorSend, optionSensor8In),
    CapacitiveSensor(capSensorSend, optionSensor9In)
};

//LCD
LiquidCrystal_SR3W lcd(LCD_Data, LCD_Clock, LCD_Strobe);

// LEDs
Type4067Mux ledMux(LED_COM, OUTPUT, DIGITAL, LED_S0, LED_S1, LED_S2, LED_S3);

// Scales
byte octave[4] = {0};
byte chromatic[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
byte major[7] = { 0, 2, 4, 5, 7, 9, 11 };
byte minor[7] = { 0, 2, 3, 5, 7, 8, 10 };
byte doric[7] = { 0, 2, 3, 5, 7, 9, 10 };

// 1voct
float voltPerOctaveNotes[61] = {
    0, 1/12, 1/12*2, 1/12*3, 1/12*4, 1/12*5, 1/12*6, 1/12*7, 1/12*8, 1/12*9, 1/12*10, 1/12*11, 
    1, 1 + 1/12, 1 + 1/12*2, 1 + 1/12*3, 1 + 1/12*4, 1 + 1/12*5, 1 + 1/12*6, 1 + 1/12*7, 1 + 1/12*8, 1 + 1/12*9, 1 + 1/12*10, 1 + 1/12*11, 
    2, 2 + 1/12, 2 + 1/12*2, 2 + 1/12*3, 2 + 1/12*4, 2 + 1/12*5, 2 + 1/12*6, 2 + 1/12*7, 2 + 1/12*8, 2 + 1/12*9, 2 + 1/12*10, 2 + 1/12*11, 
    3, 3 + 1/12, 3 + 1/12*2, 3 + 1/12*3, 3 + 1/12*4, 3 + 1/12*5, 3 + 1/12*6, 3 + 1/12*7, 3 + 1/12*8, 3 + 1/12*9, 3 + 1/12*10, 3 + 1/12*11, 
    4, 4 + 1/12, 4 + 1/12*2, 4 + 1/12*3, 4 + 1/12*4, 4 + 1/12*5, 4 + 1/12*6, 4 + 1/12*7, 4 + 1/12*8, 4 + 1/12*9, 4 + 1/12*10, 4 + 1/12*11, 
    5
};


void setup() {
    lcd.begin(16, 2);
    lcd.home();
    lcd.noCursor();
    lcd.print("t");
    lcd.print(tempo);
    lcd.setCursor(13, 0);
    lcd.print("Seq");
    lcd.setCursor(13, 1);
    lcd.print("ch");
    lcd.print(activeChannel);
}


void loop() {
    
    ms = millis();
    

    // Read button touches
    for (int i = 0; i < 16; i++) {
        // Secuence
        seqSensorGatePrev[i] = seqSensorGate[i];
        seqSensorRead[i] = capSensor[i].capacitiveSensor(30);
        seqSensorGate[i] = seqSensorRead[i] >= seqTriggerFactor;
        seqSensorTrig[i] = seqSensorGate[i] == 1 && seqSensorGatePrev[i] == seqSensorGate[i];
        if (i < 10) {
            // Options
            optionSensorGatePrev[i] = optionSensorGate[i];
            optionSensorRead[i] = capSensor[i].capacitiveSensor(30);
            optionSensorGate[i] = optionSensorRead[i] >= optionTriggerFactor;
            optionSensorTrig[i] = optionSensorGate[i] == 1 && optionSensorGatePrev[i] == optionSensorGate[i];    
        }
    }

    // Handle options/menu touches
    for (int i = 0; i < 10; i++) {
        if (optionSensorTrig[i]) {
            switch (i) {  
            case 0:
                // Tempo
                encoderControl = 1;
                writeToLCDCoordinate(0, 1, "Tempo   ");
                break;
                
            case 1:
                
                break;
                
            case 2:
                
                break;
                
            case 3:
                
                break;
                
            case 4: // Mode
                encoderControl = 2;
                writeToLCDCoordinate(0, 1, "Mode    ");
                break;
                
            case 5: // Play-pause
                playing[activeChannel] = !playing[activeChannel];
                break;
                
            case 6: // Stop
                playing[activeChannel] = false;
                step[activeChannel] = 0;
                break;
                
            case 7:
                
                break;
                
            case 8: // Sequence gate/note
                seqGateControl = !seqGateControl;
                if (seqGateControl) {
                    encoderControl = 0;
                    writeToLCDCoordinate(0, 1, "SeqGate ");
                } else {
                    encoderControl = 3;
                    writeToLCDCoordinate(0, 1, "SeqNote ");
                }
                break;
                
            case 9: // Sequence direction
                encoderControl = 4;
                writeToLCDCoordinate(0, 1, "SeqDir  ");
                break;
            }
        }
    }


    // Encoder
    switch (encoderControl) {
        case 0:
            break;
        
        case 1:
            // sumar o restar al tempo
            break;
        
        default:
            break;
    }


    // Play Sequence
    if ((ms - preMillis) >= (60000 / tempo) / 8) { // on every 1/32
        preMillis = ms;
        for (int i = 0; i < channels; i++) {
            if (playing[i]) {
                if (step[i]%2 == 0) { // Strong 1/32's
                    if (step[i]/2 < steps[i] && mode[i] == 0) {
                        // Index based on active bank
                        int bankIndex;
                        if (activeBank[i] == 0) bankIndex = 0;
                        else bankIndex = 16;

                        // cvWrite[i] = sequence[bankIndex + step[i]];
                        playNote(i, sequence[bankIndex + step[i]], octave[i]);
                        gateWrite[i] = sequenceGate[bankIndex + step[i]];
                        ledMux.write(!sequenceGate[bankIndex + step[i]], step[i]); // Step indicator LED
                    }
                } else { // Weak 1/32's
                    if (mode[i] == 0 && legato[i]) gateWrite[i] = 0;
                }

                // Next step
                switch (sequenceDirection[i]) {
                    case 0: // Forward
                        isSequenceMovingForward[i] = 1;
                        if (step[i] == (steps[i] * 2 - 1)) step[i] = 0;
                        else step[i]++;
                    break;
                    
                    case 1: // Backwards
                        isSequenceMovingForward[i] = 0;
                        if (step[i] == 0) step[i] = steps[i] * 2 - 1;
                        else step[i]--;
                    break;

                    case 2: // Back & Forth
                        if (isSequenceMovingForward[i]) {
                            if (step[i] == (steps[i] * 2 - 1)) {
                                isSequenceMovingForward[i] = 0;
                                step[i]--;
                            } else {
                                step[i]++;
                            }  
                        } else {
                            if (step[i] == 0) {
                                isSequenceMovingForward[i] = 1;
                                step[i]++;  
                            } 
                            else {
                                step[i]--;
                            }    
                        }
                    break;
                }
                
            }
        }
    }
    

    // Sequencer Mode
    if (mode[activeChannel] == 0) {
        // Handle button touches
        for (int i = 0; i < 16; i++) {
            if (seqSensorTrig[i]) {
                if (seqGateControl) sequenceGate[activeChannel][i] = !sequenceGate[activeChannel][i];
                else selectedStep = i;
            }
        }
    }

    // Keyboard Mode
    if (mode[activeChannel] == 1) {
        for (int i = 0; i < 16; i++) {
            // When a key is being pressed writes cv and gate and breaks loop,
            // generating lower key priority
            if (seqSensorGate[i]) {
                switch (i) {
                case 1:
                case 2:
                case 4 ... 6:
                case 8 ... 15:
                    gateWrite[activeChannel] = 1;
                    playNote(activeChannel, keyToNote(i), octave[activeChannel]);
                    break;
                case 0:
                    if (octave[activeChannel] > 0) {
                        octave[activeChannel]--;
                    }
                    break;
                case 7:
                    if (octave[activeChannel] < 4) {
                        octave[activeChannel]++;
                    }
                    break;
                }
                break;
            } else if (i == 15 && !seqSensorGate[i]) {
                // If no key is pressed, set gate to low
                gateWrite[activeChannel] = 0;
            }
        }
        
    }

    // LEDs & LCD
    switch (mode[activeChannel]) {
        case 0: // Sequencer
            for (int i = 0; i < 16; i++) {
                int bankIndex;
                if (activeBank[i] == 0) bankIndex = 0;
                else bankIndex = 16;
                for (int j = bankIndex; j < bankIndex + 16; j++) ledMux.write(sequenceGate[j], i);
            }
            writeToLCDCoordinate(13, 0, "Seq");
            break;

        case 1: // Keyboard
            lightKeyboard();
            writeToLCDCoordinate(13, 0, "Kyb");
            break;

        case 2:
            lightKeyboard();
            writeToLCDCoordinate(13, 0, "Arp");
            break;
    }

    writeToLCDCoordinate(0, 0, String(tempo));
    if (encoderControl != 3) writeToLCDCoordinate(8, 0, translateNoteName(int(cvWrite[activeChannel] * 12)));
    writeToLCDCoordinate(11, 1, String(int(activeBank)));
    writeToLCDCoordinate(15, 1, String(int(activeChannel)));


    // Write outputs
    digitalWrite(gateOut0, gateWrite[0]);
    digitalWrite(gateOut1, gateWrite[1]);
    digitalWrite(gateOut2, gateWrite[2]);
    digitalWrite(gateOut3, gateWrite[3]);
    analogWrite(cvOut0, cvWrite[0]);
    analogWrite(cvOut1, cvWrite[1]);
    analogWrite(cvOut2, cvWrite[2]);
    analogWrite(cvOut3, cvWrite[3]);
    
}

void playNote (byte channel, byte note, byte oct) {
    float voltage = voltPerOctaveNotes[note] + oct;
    cvWrite[channel] = map(voltage, 0, 5, 0, 255);
}

void lightKeyboard () {
    for (int i = 0; i < 16; i++) {
        switch (i) {
        case 1:
        case 2:
        case 4 ... 6:
        case 8 ... 15:
            ledMux.write(1, i);
            break;
        }
    }
}

void writeToLCDCoordinate (int col, int row, String text) {
    lcd.setCursor(col, row);
    lcd.print(text);
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