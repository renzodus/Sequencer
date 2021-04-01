#include <CapacitiveSensor.h>
#include <LiquidCrystal_SR3W.h>

#include "pins.h"
#include "misc.h"

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
bool sequenceLED[16] = {0};
bool shouldLightsUpdate = true;
bool isSequenceMovingForward[4] = {1};
bool legato[4] = {0};
// Modes (0: Sequencer, 1: Keyboard, 2: Arpeggiator, 3: MIDI controller)
int mode[4] = {0, 1, 0, 0};
byte activeBank[4] = {0, 0, 0, 0};
byte activeChannel = 0;
byte prevChannel;
float cvWrite[4] = {0, 0, 0, 0};
bool gateWrite[4] = {0, 0, 0, 0};
int tempo = 120;
unsigned long preMillis = 0;
unsigned long ms = millis();

// Controls
const bool keyBoardLights[16] = {0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};
// Encoder 0: nothing, 1: tempo, 2: Mode, 3: sequence notes, 4: sequence direction, 5: scale
int encoderControl = 0;                 
bool seqGateControl = 1;                // true: set gate, false: set note
bool setSteps = 0;                      // true: set steps, false: set gate/note
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
bool shouldLCDUpdate[5] = {true}; // 0: tempo, 1: note, 2: mode, 3: option, 4: channel

// Scales
byte octave[4] = {0};
uint8_t scale[4] = {0};
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
    attachInterrupt(digitalPinToInterrupt(encoderDT), encoder, LOW);
    lcd.begin(16, 2);
    lcd.home();
    lcd.noCursor();
    lcd.print("t");
    lcd.print(tempo);
    writeToLCDCoordinate(13, 0, "Seq");
    writeToLCDCoordinate(13, 1, "ch");
    lcd.print(activeChannel);
}


void loop() {
    
    ms = millis();
    activeChannel = digitalRead(channelSelector);
    if (activeChannel != prevChannel) updateLCD(4, String(int(activeChannel)));
    

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
            case 0: // Tempo
                encoderControl = 1;
                writeToLCDCoordinate(0, 1, "Tempo   ");
                break;
                
            case 1:
                
                break;
                
            case 2: // Select Scale
                encoderControl = 5;
                writeToLCDCoordinate(0, 1, "Scl ");
                break;
                
            case 3: // Bank Switch
                if (activeBank[activeChannel] == 0) activeBank[activeChannel] = 1;
                else activeBank[activeChannel] = 0;
                writeToLCDCoordinate(0, 1, "Bank " + String(activeBank[activeChannel]));
                break;
                
            case 4: // Mode
                encoderControl = 2;
                writeToLCDCoordinate(0, 1, "Mode    ");
                break;
                
            case 5: // Play-pause
                playing[activeChannel] = !playing[activeChannel];
                break;
                
            case 6: // Stop
                for (int i = 0; i < channels; i++) {
                    playing[i] = false;
                    step[i] = 0;
                    isSequenceMovingForward[i] = true;  
                }
                // playing[activeChannel] = false;
                // step[activeChannel] = 0;
                // isSequenceMovingForward[activeChannel] = true;
                break;
                
            case 7: // Steps
                setSteps = true;
                writeToLCDCoordinate(0, 1, "Steps " + String(steps[activeChannel]));
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
                writeToLCDCoordinate(0, 1, "SeqDir" + translateSequenceDirection(sequenceDirection[activeChannel]));
                break;
            }
        }
    }


    // Play Sequence
    if ((ms - preMillis) >= (60000 / tempo) / 8) { // on every 1/32
        preMillis = ms;
        for (int i = 0; i < channels; i++) {
            if (playing[i]) {
                if (step[i]%2 == 0) { // Strong 1/32's
                    if (step[i]/2 < steps[i] && mode[i] == 0) {
                        playNote(i, sequence[activeBank[i] * 16 + step[i]], octave[i]);
                        gateWrite[i] = sequenceGate[activeBank[i] * 16 + step[i]]; 
                        sequenceLED[step[i]] = !sequenceLED[step[i]]; // Invert step's LED
                    }
                } else { // Weak 1/32's
                    if (mode[i] == 0) {
                        if (!legato[i]) gateWrite[i] = 0;
                        sequenceLED[step[i]] = !sequenceLED[step[i]]; // Revert step's LED
                    }
                }
                sequenceNextStep(i);
            }
        }
        if (mode[activeChannel] == 0) shouldLightsUpdate = true;
    }
    
    // Handle button touches
    switch (mode[activeChannel]) {
        case 0: // Sequencer Mode
            if (shouldLightsUpdate) updateLights(sequenceLED);
            if (shouldLCDUpdate[2]) updateLCD(2, "Seq");

            for (int i = 0; i < 16; i++) {
                if (seqSensorTrig[i]) {
                    if (setSteps == true) {
                        steps[activeChannel] = i + 1;
                        setSteps = false;
                        break;
                    } else {
                        if (seqGateControl) {
                            sequenceGate[activeChannel][i] = !sequenceGate[activeChannel][i];
                            sequenceLED[i] = sequenceGate[activeChannel][i];
                        } else {
                            selectedStep = i;
                        }    
                    }
                }
            }       
            break;

        case 1: // Keyboard Mode
            if (shouldLightsUpdate) updateLights(keyBoardLights);
            if (shouldLCDUpdate[2]) updateLCD(2, "Kyb");
            
            for (int i = 0; i < 16; i++) {
                // When a key is being pressed writes cv and gate
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
                        if (octave[activeChannel] > 0) octave[activeChannel]--;
                        break;
                    case 7:
                        if (octave[activeChannel] < 4) octave[activeChannel]++;
                        break;
                    }
                    break; // break the loop to generate lower key priority
                } else if (i == 15 && !seqSensorGate[i]) {
                    // If no key is pressed, set gate to low
                    gateWrite[activeChannel] = 0;
                }
            }
            break;

        case 2: // Arpeggiator
            if (shouldLightsUpdate) updateLights(keyBoardLights);
            if (shouldLCDUpdate) updateLCD(2, "Arp");
            break;
        
        default:
            break;
    }


    // Write outputs
    digitalWrite(gateOut0, gateWrite[0]);
    digitalWrite(gateOut1, gateWrite[1]);
    digitalWrite(gateOut2, gateWrite[2]);
    digitalWrite(gateOut3, gateWrite[3]);
    analogWrite(cvOut0, cvWrite[0]);
    analogWrite(cvOut1, cvWrite[1]);
    analogWrite(cvOut2, cvWrite[2]);
    analogWrite(cvOut3, cvWrite[3]);
    
    prevChannel = activeChannel;
}


void playNote (byte channel, byte note, byte oct) {
    float voltage = voltPerOctaveNotes[note] + oct;
    cvWrite[channel] = voltage;
    updateLCD(1, translateNoteName(note));
}

void writeToLCDCoordinate (int col, int row, String text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}

void updateLCD (int option, String text) {
    switch (option) {
        case 0: // Tempo
            writeToLCDCoordinate(1, 0, text);
            shouldLCDUpdate[0] = false;
            break;
        
        case 1: // Note
            writeToLCDCoordinate(8, 0, text);
            shouldLCDUpdate[1] = false;
            break;
        
        case 2: // Mode
            writeToLCDCoordinate(13, 0, text);
            shouldLCDUpdate[2] = false;
            break;
        
        case 3: // Option
            writeToLCDCoordinate(0, 1, text);
            shouldLCDUpdate[3] = false;
            break;
        
        case 4: // Channel
            writeToLCDCoordinate(13, 1, text);
            shouldLCDUpdate[4] = false;
            break;
        
        case 5: // Bank
            writeToLCDCoordinate(11, 1, text);
            shouldLCDUpdate[4] = false;

        default:
            break;
    }
}

void updateLights (bool data[16]) {
    latchedShiftOut_16bits(LED_Data, LED_Strobe, LED_Clock, data);
    shouldLightsUpdate = false;
}

void encoder () {
    static unsigned long lastInterrupt = 0;
    unsigned long interruptTime = millis();

    if (interruptTime - lastInterrupt > 5) {
        switch (encoderControl) {
            case 0:
                return;
            
            case 1: // Tempo
                tempo = setValue(tempo, 30, 999);
                updateLCD(0, String(tempo));
                break;
            
            case 2: // Mode
                mode[activeChannel] = setValue(mode[activeChannel], 0, 3);
                updateLCD(2, "Mode " + String(mode[activeChannel]));
                shouldLCDUpdate[2] = true;
                break;
            
            case 3: // Sequence note
                sequence[activeChannel][selectedStep] = setValue(sequence[activeChannel][selectedStep], 0, 60);
                updateLCD(1, translateNoteName(sequence[activeChannel][selectedStep]));
                break;
            
            case 4: // Sequence direction
                sequenceDirection[activeChannel] = setValue(sequenceDirection[activeChannel], 0, 2);
                writeToLCDCoordinate(6, 1, translateSequenceDirection(sequenceDirection[activeChannel]));
                break;
            
            case 5: // Scale
                scale[activeChannel] = setValue(scale[activeChannel], 0, 5);
                break;
            
            default:
                break;
        }
        lastInterrupt = interruptTime;  
    }
}

void sequenceNextStep (int i) {
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