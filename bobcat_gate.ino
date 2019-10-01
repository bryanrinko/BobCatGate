#include <Multi_BitBang.h>  //https://github.com/bitbank2/Multi_BitBang
#include <Multi_OLED.h>     //https://github.com/bitbank2/Multi_OLED
#include <SoftwareSerial.h>

#define NUM_DISPLAYS 2
#define NUM_BUSES 1
// I2C bus info
uint8_t scl_list[NUM_BUSES] = {A5};
uint8_t sda_list[NUM_BUSES] = {A4};
int32_t speed_list[NUM_BUSES] = {1000000L};
// OLED display info
uint8_t bus_list[NUM_DISPLAYS] = {0,0}; // can be multiple displays per bus
uint8_t addr_list[NUM_DISPLAYS] = {0x3c, 0x3d};
uint8_t type_list[NUM_DISPLAYS] = {OLED_128x64, OLED_128x64};
uint8_t flip_list[NUM_DISPLAYS] = {0,0};
uint8_t invert_list[NUM_DISPLAYS] = {0,0};

SoftwareSerial mySerial(0, 1); // RX, TX

const int GATE1RELAY=7;
const int GATE2RELAY=8;

const int OPENSTATE=1;
const int debounce = 250; // debounce latency in ms
const int endDelay = 3000;

char *myLoopStates[] = {"sensorINIT", "Ready", "Starting","Started", "Ending", "Ended"};
const int SENSORINIT = 0;
const int READY = 1;
const int STARTING = 2;
const int STARTED = 3;
const int ENDING = 4;
const int ENDED = 5;

char *myBrand[] = {"BOBCATgates", "v0.08"};
const int DEVICE_BRAND = 0;
const int VERSION = 1;

int flag = 0;
char *state[] = {"",""};

unsigned long StartTime[] = {0,0};
unsigned long EndTime[] = {0,0};
unsigned long ElapsedTime[] = {0,0};

void setup() {
  // Start Serial
  Serial.begin(9600);

  Multi_I2CInit(sda_list, scl_list, speed_list, NUM_BUSES);
  Multi_OLEDInit(bus_list, addr_list, type_list, flip_list, invert_list, NUM_DISPLAYS);
  
  myOLED2Gate(myBrand[DEVICE_BRAND],-1, "Loading",myBrand[VERSION]);
  Serial.println(myBrand[DEVICE_BRAND]);
  Serial.println("Loading");
  Serial.println(myBrand[VERSION]);
  delay(2000);

  pinMode(GATE1RELAY, INPUT_PULLUP);
  pinMode(GATE2RELAY, INPUT_PULLUP);

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
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1, state[gateIndex], "");
   }else{
    if (state[gateIndex] == myLoopStates[READY] && val != OPENSTATE){
      state[gateIndex] = myLoopStates[STARTING];
      StartTime[gateIndex] = millis();
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[STARTING] && val == OPENSTATE && millis()>= StartTime[gateIndex]+debounce){
      state[gateIndex] = myLoopStates[STARTED];
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[STARTED] && val != OPENSTATE){
      state[gateIndex] = myLoopStates[ENDING];
      EndTime[gateIndex] = millis();
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1, state[gateIndex], "");
    } else if(state[gateIndex] == myLoopStates[ENDING] && val == OPENSTATE  && millis()>= EndTime[gateIndex]+debounce){
      state[gateIndex] = myLoopStates[ENDED];
      ElapsedTime[gateIndex] = EndTime[gateIndex] - StartTime[gateIndex];
      String finalTime = String(ElapsedTime[gateIndex] / 1000.0, 3);
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1, state[gateIndex], finalTime.c_str());
    } else if(state[gateIndex] == myLoopStates[ENDED] && val == OPENSTATE && millis()>= EndTime[gateIndex]+endDelay){
      state[gateIndex] = myLoopStates[READY];
      myOLED2Gate(myBrand[DEVICE_BRAND],gateIndex+1,state[gateIndex], "");
    }
  }
}

void myOLED2Gate(char *header, int gate, char *lineText1, char *lineText2) {
  Serial.print("GATE:");
  Serial.print(gate);
  Serial.print(" -> ");
  if (gate == 1){
     Serial.println(lineText1);
  } else if (gate == 2){
     Serial.println(lineText2);
  }
  
  //Clear Display
  Multi_OLEDFill(gate-1, 0);
  Multi_OLEDWriteString(gate-1, 0, 1, (char *)header, FONT_SMALL, 0);
  Multi_OLEDWriteString(gate-1, 10, 18, (char *)lineText1, FONT_NORMAL, 0);
  Multi_OLEDWriteString(gate-1, 10, 40, (char *)lineText2, FONT_LARGE, 0);
}
