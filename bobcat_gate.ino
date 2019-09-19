#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial mySerial(0, 1); // RX, TX

const int GATERELAY=7;
const int OPENSTATE=1;
const String SENSORUP = "up:";
const int debounce = 250; // debounce latency in ms
const int endDelay = 3000;

char *myLoopStates[] = {"INIT", "Ready", "Starting","Started", "Ending", "Ended"};
const int SENSORINIT = 0;
const int READY = 1;
const int STARTING = 2;
const int STARTED = 3;
const int ENDING = 4;
const int ENDED = 5;

char *myBrand[] = {"BOBgate", "v0.06"};
const int DEVICE_BRAND = 0;
const int VERSION = 1;

int flag = 0;
String state;

unsigned long StartTime = 0;
unsigned long EndTime = 0;
unsigned long ElapsedTime = 0;

void setup() {
  // Start Serial
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  
  myOLED(myBrand[DEVICE_BRAND],"Loading",myBrand[VERSION]);
  Serial.println(myBrand[DEVICE_BRAND]);
  Serial.println("Loading");
  Serial.println(myBrand[VERSION]);
  delay(2000);

  pinMode(GATERELAY, INPUT_PULLUP);
  state = myLoopStates[SENSORINIT];
}

void loop() {
  runLoop();
}

void runLoop(){
   int val = digitalRead(GATERELAY);

   if (state == myLoopStates[SENSORINIT] && val == OPENSTATE){
      state = myLoopStates[READY];
      myOLED(myBrand[DEVICE_BRAND],state,"");
      Serial.println(state);
   }else{
    if (state == myLoopStates[READY] && val != OPENSTATE){
      state = myLoopStates[STARTING];
      StartTime = millis();
      myOLED(myBrand[DEVICE_BRAND],state,"");
      Serial.println(state);
      delay(debounce);
    } else if(state == myLoopStates[STARTING] && val == OPENSTATE){
      state = myLoopStates[STARTED];
      myOLED(myBrand[DEVICE_BRAND],state,"");
      Serial.println(state);
    } else if(state == myLoopStates[STARTED] && val != OPENSTATE){
      state = myLoopStates[ENDING];
      EndTime = millis();
      myOLED(myBrand[DEVICE_BRAND],state,"");
      Serial.println(state);
      delay(debounce);
    } else if(state == myLoopStates[ENDING] && val == OPENSTATE){
      state = myLoopStates[ENDED];
      ElapsedTime = EndTime - StartTime;
      String finalTime = String(ElapsedTime / 1000.0, 3);
      myOLED(myBrand[DEVICE_BRAND],state,finalTime);
      Serial.println(finalTime);
      delay(endDelay);
    } else if(state == myLoopStates[ENDED] && val == OPENSTATE){
      state = myLoopStates[READY];
      myOLED(myBrand[DEVICE_BRAND],state,"");
      Serial.println(state);
    }
  }
}

void myOLED(String header, String line1, String line2) {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 1);
  display.println(header);

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 18);
  display.println(line1);

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(10, 40);
  display.println(line2);
  
  display.display();
}