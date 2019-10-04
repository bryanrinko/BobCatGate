#include <Multi_BitBang.h>  //https://github.com/bitbank2/Multi_BitBang
#include <Multi_OLED.h>     //https://github.com/bitbank2/Multi_OLED
#include <SoftwareSerial.h>

//Brand & version.  Up at top so easy to update
char *myBrand[] = {"BOBCATgate", "v0.09"};

//TODO: add color LEDs to signal the skaters
//https://randomnerdtutorials.com/guide-for-ws2812b-addressable-rgb-led-strip-with-arduino/

//Confgure OLED Displays (one for each gate/lane)
#define NUM_DISPLAYS 2
#define NUM_BUSES 2
// I2C bus info
uint8_t scl_list[NUM_BUSES] = {A5,A5};
uint8_t sda_list[NUM_BUSES] = {A4,A3};
int32_t speed_list[NUM_BUSES] = {400000L,400000L};
// OLED display info
uint8_t bus_list[NUM_DISPLAYS] = {0,1}; // can be multiple displays per bus
uint8_t addr_list[NUM_DISPLAYS] = {0x3c, 0x3c};
uint8_t type_list[NUM_DISPLAYS] = {OLED_128x64, OLED_128x64};
uint8_t flip_list[NUM_DISPLAYS] = {0,0};
uint8_t invert_list[NUM_DISPLAYS] = {0,0};

//Configure Bluetooth transmitter
SoftwareSerial mySerial(0, 1); // RX, TX

//Configure gate data structures
const int GATE1RELAY=7;
const int GATE2RELAY=8;

const int OPENSTATE=1;
const int debounce = 250; // debounce latency in ms
const int endDelay = 10000;

char *myLoopStates[] = {"sensorINIT", "Ready", "Starting","Started", "Ending", "Ended"};
const int SENSORINIT = 0;
const int READY = 1;
const int STARTING = 2;
const int STARTED = 3;
const int ENDING = 4;
const int ENDED = 5;

int flag = 0;
char *state[] = {"",""};

unsigned long StartTime[] = {0,0};
unsigned long EndTime[] = {0,0};
unsigned long ElapsedTime[] = {0,0};

char * header;

void setup() {
  // Start Serial
  Serial.begin(9600);

  //Initialize displays
  Multi_I2CInit(sda_list, scl_list, speed_list, NUM_BUSES);
  Multi_OLEDInit(bus_list, addr_list, type_list, flip_list, invert_list, NUM_DISPLAYS);

  header = buildHeader ();
  
  //Load the displays
  myOLED2Gate(1,"Load 1","");
  myOLED2Gate(2,"Load 2","");
  delay(2000);

  //Initialize input pins for sensor Receivers (Gate1 and Gate2)
  pinMode(GATE1RELAY, INPUT_PULLUP);
  pinMode(GATE2RELAY, INPUT_PULLUP);

  //Initialize internal state machine
  state[0] = myLoopStates[SENSORINIT];
  state[1] = myLoopStates[SENSORINIT];
}

void loop() {
   evalState(0,GATE1RELAY);
   evalState(1,GATE2RELAY);
}

void evalState(int gateIndex,int gatePin){
   int val = digitalRead(gatePin);
   
   if (state[gateIndex] == myLoopStates[SENSORINIT] && val == OPENSTATE){
      state[gateIndex] = myLoopStates[READY];
      myOLED2Gate(gateIndex+1, state[gateIndex], "");
   }else{
    if (state[gateIndex] == myLoopStates[READY] && val != OPENSTATE){
      state[gateIndex] = myLoopStates[STARTING];
      StartTime[gateIndex] = millis();
      myOLED2Gate(gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[STARTING] && val == OPENSTATE && millis()>= StartTime[gateIndex]+debounce){
      state[gateIndex] = myLoopStates[STARTED];
      myOLED2Gate(gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[STARTED] && val != OPENSTATE){
      state[gateIndex] = myLoopStates[ENDING];
      EndTime[gateIndex] = millis();
      myOLED2Gate(gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[ENDING] && val == OPENSTATE  && millis()>= EndTime[gateIndex]+debounce){
      state[gateIndex] = myLoopStates[ENDED];
      ElapsedTime[gateIndex] = EndTime[gateIndex] - StartTime[gateIndex];
      String finalTime = String(ElapsedTime[gateIndex] / 1000.0, 3);
      myOLED2Gate(gateIndex+1,finalTime.c_str(), state[gateIndex]);
    } else if(state[gateIndex] == myLoopStates[ENDED] && val == OPENSTATE && millis()>= EndTime[gateIndex]+endDelay){
      state[gateIndex] = myLoopStates[READY];
      myOLED2Gate(gateIndex+1,state[gateIndex], "");
    }
  }
}

void myOLED2Gate(int gate, char *lineText1, char *lineText2) {
  //Expect these are also delivered to bluetooth
  Serial.print(header);
  Serial.print(" : GATE ");
  Serial.print(gate);
  Serial.print(" -> ");
  Serial.print(lineText1);
  if (lineText2 != "") {
    Serial.print(" ");
    Serial.print(lineText2);    
  }
  Serial.println("");
  
  //Clear Display
  Multi_OLEDFill(gate-1, 0);

  Multi_OLEDWriteString(gate-1, 0, 0, (char *)header, FONT_SMALL, 0);
  Multi_OLEDWriteString(gate-1, 5, 2, (char *)lineText1, FONT_LARGE, 0);
  Multi_OLEDWriteString(gate-1, 5, 7, (char *)lineText2, FONT_NORMAL, 0);
}

char * buildHeader()
{
  char * buf = (char *) malloc (666);
  strcpy (buf, myBrand[DEVICE_BRAND]);
  strcat (buf, " : ");
  strcat (buf, myBrand[VERSION]);
  return buf;
}  // end of buildHeader
