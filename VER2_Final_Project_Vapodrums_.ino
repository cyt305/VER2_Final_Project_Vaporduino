#include "BetterButton.h"
#include "Stepper.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>


#include "AudioSampleKickk.h"
#include "AudioSampleKick2.h"
#include "AudioSampleSnare.h"
#include "AudioSampleSnare2.h"
#include "AudioSampleOpenhihat.h"
#include "AudioSampleClosedhihatt.h"
#include "AudioSampleCrash.h"
#include "AudioSampleTomhi.h"
#include "AudioSampleTomlow.h"
#include "AudioSampleClaps.h"
#include "AudioSampleLowcowbell.h"
#include "AudioSampleTambourine.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlayMemory          playMem3;       //xy=182,358
AudioPlayMemory          playMem2;       //xy=184,323
AudioPlayMemory          playMem1;       //xy=201,286
AudioMixer4              mixer1;         //xy=364,313
AudioOutputI2S           i2s1;           //xy=536,245
AudioConnection          patchCord1(playMem3, 0, mixer1, 2);
AudioConnection          patchCord2(playMem2, 0, mixer1, 1);
AudioConnection          patchCord3(playMem1, 0, mixer1, 0);
AudioConnection          patchCord4(mixer1, 0, i2s1, 0);
AudioConnection          patchCord5(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=829,318
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000
};


int selectButtonPin = 33;
int selectPot = A13;
int channelPot = A14;
int tempoPot = A12;
int ledPins[4] = {9, 10, 11, 12};
int currStep;

int channelSelect;
int stepSelect;

int channelDisplayed;



boolean stepOns[12][16] = {
  { HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, HIGH, HIGH, LOW }, // KICK1
  { LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW }, // KICK2
  { LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, HIGH }, //SNARE1
  { LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH }, //SNARE2
  { LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW }, //OPEN HIHAT
  { HIGH, HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW }, //CLOSED HIHAT
  { LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW }, //CRASH
  { LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW }, //TOM
  { LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW }, //TOM2
  { LOW, HIGH, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW }, //CLAP
  { LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW }, //COWBELL
  { LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW }, //TAMBOU
};

int midiNotes[12] = { 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46 };

bool circlesFilled[16] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

BetterButton selectButton(selectButtonPin, 1);
Stepper stepper1(16, 1000);


void setup() {

  Serial.begin(9600);

  pinMode(selectButtonPin, INPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  selectButton.pressHandler(onPress);
  selectButton.releaseHandler(onRelease);

  stepper1.stepHandler(onStep);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);


  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  delay(1000);
  display.invertDisplay(false);
  delay(1000);

  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);
}

void loop() {
  currStep = stepper1.currentStep;
  selectButton.process();

  stepper1.process();
  stepper1.setTimePerStep(map(analogRead(tempoPot), 0, 1023, 20, 250));

  checkChannelPot();
  stepSelectPot();

  Serial.println(channelSelect);

  screenDisplay();


}


void onPress(int val) {
  //stepSelectButton
  if (val == 1) {
    if (stepOns[channelSelect][stepSelect] == HIGH) {
      stepOns[channelSelect][stepSelect] = LOW;
    } else if (stepOns[channelSelect][stepSelect] == LOW) {
      stepOns[channelSelect][stepSelect] = HIGH;
    }
    delay(5);
  }
}

void onStep(int num) {
  checkLeds();

  if (stepOns[0][num] == HIGH) {
    playMem1.play(AudioSampleKickk);
  }
  if (stepOns[1][num] == HIGH) {
    playMem1.play(AudioSampleKick2);
  }
  if (stepOns[2][num] == HIGH) {
    playMem1.play(AudioSampleSnare);
  }
  if (stepOns[3][num] == HIGH) {
    playMem1.play(AudioSampleSnare2);
  }
  if (stepOns[4][num] == HIGH) {
    playMem1.play(AudioSampleOpenhihat);
  }
  if (stepOns[5][num] == HIGH) {
    playMem2.play(AudioSampleClosedhihatt);
  }
}


void onRelease(int val) {
  Serial.print(val);
  Serial.println(" off");
}

void checkLeds() {

  //blinks every 4 step

  int stepCounter = currStep / 4;
  if (stepCounter == 0) {
    analogWrite(ledPins[0], 225);
    analogWrite(ledPins[1], 10);
    analogWrite(ledPins[2], 10);
    analogWrite(ledPins[3], 10);
  }
  else if (stepCounter == 1) {
    analogWrite(ledPins[0], 10);
    analogWrite(ledPins[1], 225);
    analogWrite(ledPins[2], 10);
    analogWrite(ledPins[3], 10);
  }
  else if (currStep / 4 == 2) {
    analogWrite(ledPins[0], 10);
    analogWrite(ledPins[1], 10);
    analogWrite(ledPins[2], 225);
    analogWrite(ledPins[3], 10);
  }
  else if (currStep / 4 == 3) {
    analogWrite(ledPins[0], 10);
    analogWrite(ledPins[1], 10);
    analogWrite(ledPins[2], 10);
    analogWrite(ledPins[3], 225);
  }
}

void checkChannelPot() {
  channelSelect = map(analogRead(channelPot), 0, 1023, 0, 11);
}

void stepSelectPot() {
  stepSelect = map(analogRead(selectPot), 0, 1023, 0, 16);
}

void screenDisplay(void) {
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setCursor(0, 35);
  display.setTextColor(BLACK, WHITE);
  display.print(F("instr "));
  display.setTextColor(WHITE);
  display.print(F("..*// "));

  //display channel when selected//
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.setTextColor(WHITE);
  if (channelSelect == 0) {
    display.println("Kick 1");
  }
  else if (channelSelect == 1) {
    display.println("Kick 2");
  }
  else if (channelSelect == 2) {
    display.println("Snare 1");
  }
  else if (channelSelect == 3) {
    display.println("Snare 2");
  }
  else if (channelSelect == 4) {
    display.println("OPEN hh");
  }
  else if (channelSelect == 5) {
    display.println("CLOSED*hh");
  }
  else if (channelSelect == 6) {
    display.println("Crash!");
  }
  else if (channelSelect == 7) {
    display.println("Tom 1");
  }
  else if (channelSelect == 8) {
    display.println("Tom 2");
  }
  else if (channelSelect == 9) {
    display.println("Claps");
  }
  else if (channelSelect == 10) {
    display.println("Cowbell");
  }
  else if (channelSelect == 11) {
    display.println("Tambourine");
  }

  for (int i = 0; i < 16; i++) {
    if (stepOns[channelSelect][i] == true) {
      circlesFilled[i] = true;
    }
    else {
      circlesFilled[i] = false;
    }
  }
  
 //draw circles//
  for (int i = 0; i < 16; i++) {
    if (circlesFilled[i] == true) {
      display.fillCircle((i + 1) * 7, 20, 2, WHITE);
    } else if (circlesFilled[i] == false) {
      display.drawCircle((i + 1) * 7, 20, 2, WHITE);
    }
  }

  display.display();
  delay(100 );
}
