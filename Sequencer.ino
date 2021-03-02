#include <CapacitiveSensor.h>
#include <LiquidCrystal_SR3W.h>

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
bool playing = false;
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
byte mode[4] = {0, 0, 0, 0};
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
int seqTriggerFactor = 700;
int optionTriggerFactor = 700;
int seqSensorValues[16] = {0};
int seqSensorTime[16] = {0};
int optionSensorValues[10] = {0};
int optionSensorTime[10] = {0};
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

// Scales
byte octave[4] = {0};
byte chromatic[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
byte major[7] = { 0, 2, 4, 5, 7, 9, 11 };
byte minor[7] = { 0, 2, 3, 5, 7, 8, 10 };
byte doric[7] = { 0, 2, 3, 5, 7, 9, 10 };

// 1voct
float voltPerOctaveNotes[12] = { 1/12, 1/12*2, 1/12*3, 1/12*4, 1/12*5, 1/12*6, 1/12*7, 1/12*8, 1/12*9, 1/12*10, 1/12*11, 1/12*12};


void setup() {
    lcd.begin();
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

    // Leer entradas de secuencia y opciones respectivamente
    for (int i = 0; i < 16; i++) seqSensorValues[i] = capSensor[i].capacitiveSensor(30);
    for (int i = 0; i < 10; i++) optionSensorValues[i] = capSensor[i + 16].capacitiveSensor(30);

    // Manejar toques de botones de opciones
    for (int i = 0; i < 10; i++) {
        if (optionSensorValues[i] >= optionTriggerFactor && millis() - optionSensorTime[i] > debounce) {
            switch (i) {  
            case 0:
                // Tempo
                encoderControl = 1;
                lcd.setCursor(0, 1);
                lcd.print("Tempo");
                break;
                
            case 1:
                playing = !playing;
                break;
                
            case 2:
                playing = !playing;
                break;
                
            case 3:
                playing = !playing;
                break;
                
            case 4:
                playing = !playing;
                break;
                
            case 5:
                playing = !playing;
                break;
                
            case 6:
                playing = !playing;
                break;
                
            case 7:
                playing = !playing;
                break;
                
            case 8:
                playing = !playing;
                break;
                
            case 9:
                playing = !playing;
                break;
            
            default:
                break;
            }
            optionSensorTime[i] = millis();
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
	if (playing) {
        // Recorrer secuencia en fusas
		if ((ms - preMillis) >= (60000 / tempo) / 8) { 
            preMillis = ms;

            // Fusas fuertes
            if (globalCount%2 == 0) {
                if (globalCount/2 < steps) {
                    for (int i = 0; i < channels; i++) {
                        if (mode[i] == 0) {
                            // Seleccionar secuencia según banco activo
                            int bank = activeBank[i];
                            int bankIndex;
                            if (bank == 0) bankIndex = 0;
                            else bankIndex = 16;

                            // Setear valores a escribir
                            for (int j = bankIndex; j < bankIndex + 16; j++){
                                cvWrite[i] = sequence[j];
                                gateWrite[i] = sequenceGate[j];
                            }     
                        }
                    }
                }
            } else {
                // Fusas débiles, baja el gate
                for (int i = 0; i < channels; i++) if (mode[i] == 0) gateWrite[i] = 0;
            }

            // Resetear contador en último step
            for (int i = 0; i < channels; i++) {
                if (step[i] == (steps[i] * 2 - 1)) step[i] = 0;
                else step[i]++;
            }
            
        }
    }


    // Modo Sequencer
    if (mode[activeChannel] == 0) {
        // Manejar toques de botones de sequencer
        for (int i = 0; i < 16; i++) {
            if (seqSensorValues[i] >= seqTriggerFactor && millis() - seqSensorTime[i] > debounce) {
                sequenceGate[activeChannel][i] = !sequenceGate[activeChannel][i];
                seqSensorTime[i] = millis();
            }
        }
    }

    // Modo Keyboard
    if (mode[activeChannel] == 1) {
        for (int i = 0; i < 16; i++) {
            // Si se está tocando una tecla, escribe el gate y cv correspondiente y rompe el loop
            // Esto genera prioridad en la tecla más grave
            if (seqSensorValues[i] >= seqTriggerFactor) {
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
                default:
                    break;
                }
                
                break;
            } else if (i == 16 && seqSensorValues[i] < triggerFactor) {
                // Si no se está tocando ninguna tecla, baja el gate
                gateWrite[activeChannel] = 0;
            }
        }
        
    }


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