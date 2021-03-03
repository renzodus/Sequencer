#include <CapacitiveSensor.h>
#include <LiquidCrystal_SR3W.h>
#include "Type4067Mux.h"

#define capSensorSend 2
#define seqSensor00In 22
#define seqSensor01In 23
#define seqSensor02In 24
#define seqSensor03In 25
#define seqSensor04In 26
#define seqSensor05In 27
#define seqSensor06In 28
#define seqSensor07In 29
#define seqSensor08In 30
#define seqSensor09In 31
#define seqSensor10In 32
#define seqSensor11In 33
#define seqSensor12In 34
#define seqSensor13In 35
#define seqSensor14In 36
#define seqSensor15In 37
#define optionSensor0In 0
#define optionSensor1In 0
#define optionSensor2In 0
#define optionSensor3In 0
#define optionSensor4In 0
#define optionSensor5In 0
#define optionSensor6In 0
#define optionSensor7In 0
#define optionSensor8In 0
#define optionSensor9In 0

#define LCD_Strobe 0
#define LCD_Data 0
#define LCD_Clock 0
#define LCD_BackLightPin 0

#define LED_COM 0
#define LED_S0 0
#define LED_S1 0
#define LED_S2 0
#define LED_S3 0

#define gateOut0 0
#define gateOut1 0
#define gateOut2 0
#define gateOut3 0
#define cvOut0 0
#define cvOut1 0
#define cvOut2 0
#define cvOut3 0

// Sequencer
byte channels = 4;
bool playing[4] = {false};
byte steps[4] = {16, 16, 16, 16};
byte step[4] = {0, 0, 0, 0};
int globalCount = 0;
// Secuencia, 0-15 banco 0 , 16-31 banco 1
byte sequence[4][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 51, 51, 51, 51, 51, 51, 51, 51, 0, 0, 0, 0, 51, 51, 51, 51, 102, 102, 102, 102, 153, 153, 153, 153},
    {255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0},
    {255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0},
    {255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0},
};
bool sequenceGate[4][32] = {{0}, {0}, {0}, {0}};
// Modos (0: Sequencer, 1: Keyboard, 2: Arpeggiator, 3: MIDI controller)
byte mode[4] = {0, 1, 0, 0};
byte activeBank[4] = {0, 0, 0, 0};
byte activeChannel = 0;
byte cvWrite[4] = {0, 0, 0, 0};
bool gateWrite[4] = {0, 0, 0, 0};
int tempo = 120;
unsigned long preMillis = 0;
unsigned long ms = millis();

// Controles
// 0: nada, 1: tempo, 2: sequence notes
byte encoderControl = 0;
int debounce = 200;
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
byte chromatic[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
byte major[7] = { 0, 2, 4, 5, 7, 9, 11 };
byte minor[7] = { 0, 2, 3, 5, 7, 8, 10 };
byte doric[7] = { 0, 2, 3, 5, 7, 9, 10 };

// 1voct
float voltPerOctaveNotes[12] = { 1/12, 1/12*2, 1/12*3, 1/12*4, 1/12*5, 1/12*6, 1/12*7, 1/12*8, 1/12*9, 1/12*10, 1/12*11, 1/12*12};


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
    

    // Leer entradas de secuencia
    for (int i = 0; i < 16; i++) {
        seqSensorGatePrev[i] = seqSensorGate[i];
        seqSensorRead[i] = capSensor[i].capacitiveSensor(30);
        if (seqSensorRead[i] >= seqTriggerFactor) seqSensorGate[i] = 1;
        if (seqSensorGate[i] == 1 && seqSensorGatePrev[i] == seqSensorGate[i]) seqSensorTrig[i] = 1;
        else seqSensorTrig[i] = 0;
    }

    // Leer entradas de opciones
    for (int i = 0; i < 10; i++) {
        optionSensorGatePrev[i] = optionSensorGate[i];
        optionSensorRead[i] = capSensor[i].capacitiveSensor(30);
        if (optionSensorRead[i] >= optionTriggerFactor) optionSensorGate[i] = 1;
        if (optionSensorGate[i] == 1 && optionSensorGatePrev[i] == optionSensorGate[i]) optionSensorTrig[i] = 1;
        else optionSensorTrig[i] = 0;
    }

    // Manejar toques de botones de opciones
    for (int i = 0; i < 10; i++) {
        if (optionSensorTrig[i]) {
            switch (i) {  
            case 0:
                // Tempo
                encoderControl = 1;
                lcd.setCursor(0, 1);
                lcd.print("Tempo");
                break;
                
            case 1:
                
                break;
                
            case 2:
                
                break;
                
            case 3:
                
                break;
                
            case 4:
                
                break;
                
            case 5:
                
                break;
                
            case 6:
                playing[activeChannel] = !playing[activeChannel];
                break;
                
            case 7:
                
                break;
                
            case 8:
                
                break;
                
            case 9:
                
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


    // Tocar Secuencia
    if ((ms - preMillis) >= (60000 / tempo) / 8) { // Recorrer secuencia en fusas
        preMillis = ms;
        for (int i = 0; i < channels; i++) {
            if (playing[i]) {
                if (step[i]%2 == 0) { // Fusas fuertes
                    if (step[i]/2 < steps[i] && mode[i] == 0) {
                        // Seleccionar secuencia según banco activo
                        int bankIndex;
                        if (activeBank[i] == 0) bankIndex = 0;
                        else bankIndex = 16;

                        cvWrite[i] = sequence[bankIndex + step[i]];
                        gateWrite[i] = sequenceGate[bankIndex + step[i]];
                        ledMux.write(!sequenceGate[bankIndex + step[i]], step[i]); // LED indicador de step
                    }
                } else { // Fusas débiles, baja el gate
                    if (mode[i] == 0) gateWrite[i] = 0;
                }

                // Resetear contador en último step
                if (step[i] == (steps[i] * 2 - 1)) step[i] = 0;
                else step[i]++;
            }
        }
    }
    

    // Modo Sequencer
    if (mode[activeChannel] == 0) {
        // Manejar toques de botones de sequencer
        for (int i = 0; i < 16; i++) if (seqSensorTrig[i]) sequenceGate[activeChannel][i] = !sequenceGate[activeChannel][i];
    }

    // Modo Keyboard
    if (mode[activeChannel] == 1) {
        for (int i = 0; i < 16; i++) {
            // Si se está tocando una tecla, escribe el gate y cv correspondiente y rompe el loop
            // Esto genera prioridad en la tecla más grave
            if (seqSensorGate[i]) {
                switch (i) {
                case 1:
                case 2:
                case 4 ... 6:
                case 8 ... 15:
                    gateWrite[activeChannel] = 1;
                    playNote(activeChannel, i, octave[activeChannel]);
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
                // Si no se está tocando ninguna tecla, baja el gate
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
    writeToLCDCoordinate(8, 0, translateNoteName(int(cvWrite[activeChannel] * 12)));
    writeToLCDCoordinate(11, 1, String(int(activeBank)));
    writeToLCDCoordinate(15, 1, String(int(activeChannel)));


    //Escribir salidas
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
    switch (note) {
        case 0: return "C";
        case 1: return "C#";
        case 2: return "D";
        case 3: return "D#";
        case 4: return "E";
        case 5: return "F";
        case 6: return "F#";
        case 7: return "G";
        case 8: return "G#";
        case 9: return "A";
        case 10: return "A#";
        case 11: return "B";
    }
}