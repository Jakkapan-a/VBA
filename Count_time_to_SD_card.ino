/*
  Name : Count time to SD card
  Author : Jakkapan Attala
  Date : 13/03/2023
  Version : 1.0.0 
*/
#include <Arduino.h>
#include <TM1637Display.h>
#include <BUTTON.h>
#include <PINOUT.h>
#include <SD.h>
#include <DS1302.h>

const uint8_t SEG_P01[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G, // P
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
  SEG_B | SEG_C, // 1
};

const uint8_t SEG_P02[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G, // P
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G, // 2
};



// SD Card
#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK 13
File dataFile;

// TM1637 Display
#define CLK 4
#define DIO 5
TM1637Display display(CLK, DIO);

// Button
#define BUTTON_ESC 14
#define BUTTON_DOWN 15
#define BUTTON_UP 16
#define BUTTON_ENTER 17

void isButtonPressESC();
void isButtonReleaseESC();
BUTTON button_esc(BUTTON_ESC, isButtonPressESC, NULL);


void isButtonPressDOWN();
void isButtonReleaseDOWN();
BUTTON button_down(BUTTON_DOWN, isButtonPressDOWN, NULL);

void isButtonPressUP();
void isButtonReleaseUP();
BUTTON button_up(BUTTON_UP, isButtonPressUP, NULL);

void isButtonPressENTER();
void isButtonReleaseENTER();
BUTTON button_enter(BUTTON_ENTER, isButtonPressENTER, NULL);
// Sensor
#define SENSOR_START 7
#define SENSOR_STOP 8

void isSensorStartPress();
void isSensorStartRelease();
BUTTON sensor_start(SENSOR_START, isSensorStartPress, NULL);

void isSensorStopPress();
void isSensorStopRelease();
BUTTON sensor_stop(SENSOR_STOP, isSensorStopPress, NULL);
// LED
// #define LED_START 14
// #define LED_STOP 15

// PINOUT led_start(LED_START);
// PINOUT led_stop(LED_STOP);

// Ds1302
#define DATA_PIN 2
#define CLK_PIN 3
#define RST_PIN 4
DS1302 rtc(DATA_PIN, CLK_PIN, RST_PIN);
Time time;
// Variable
String header = "DateTime, TimeStart, TimeEnd, Time Total, Count Start, Count End, Count Total";
String dataString = "";
String fileName = "";
int count_start = 0;
int current_mode[3] = { 0, 0 , 0};

void saveFile(String);
void mainMenu(void);
void subMenu(void);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 
 /* if (!SD.begin(SD_CS)) {
    Serial.println("SD card not found");
    while (1)
      ;
  }
  Serial.println("SD card found");

  File dataFile = SD.open("test.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("File not found");
    while (1)
      ;
  }
  dataFile.println(header);
  dataFile.close();

  rtc.halt(false);          // Enable RTC clock
  rtc.writeProtect(false);  // Enable write to RTC
*/
  // Display
  display.setBrightness(5);
  display.showNumberDec(0, false, 4, 0);

  time = rtc.getTime();
  // fileName = time.day +".txt";

  Serial.println("Setup complete");
}

void loop() {
  button_esc.update();
  button_up.update();
  button_down.update();
  button_enter.update();
  sensor_start.update();
  sensor_stop.update();

  mainMenu();
}

void mainMenu() {
  // Display
  switch (current_mode[0]) {
    case 0:
      // Normal mode
      display.showNumberDec(0, false, 4, 0);
      break;
    case 1:
    // F001 Mode Setting Date Time
    // SubMenu
      switch (current_mode[1]) {
        case 0:
          // Display show F001
          display.showNumberHexEx(0xF001);
          break;
        case 1:
          // P001 Setting Date
          display.showNumberHexEx(0xF002);
          break;
        case 2:
          // P002 Setting Time
          byte segments[] = {SEG_A, SEG_D, SEG_D, SEG_D};
           display.setSegments(SEG_P01);
          break;
        default:
          current_mode[1] = 0;
          break;
      }
      break;

    case 2:   // F002 Mode Setting File Name
    break;
    default:
      current_mode[0] = 0;
      break;
  }
}
void isButtonPressESC() {
  Serial.println("Button ESC pressed!");
  current_mode[0] = 0;
}

void isButtonPressDOWN() {
  Serial.println("Button DOWN pressed!");
  current_mode[1] -= 1;
}

void isButtonPressUP() {
  Serial.println("Button UP pressed!");
  current_mode[1] += 1;
}

void isButtonPressENTER() {
  Serial.println("Button ENTER pressed!");
  current_mode[0] = 1;
}

void isSensorStartPress() {
  Serial.println("Sensor Start pressed!");
}

void isSensorStartRelease() {
  Serial.println("Sensor Start released!");
}

void isSensorStopPress() {
  Serial.println("Sensor Stop pressed!");
}

void isSensorStopRelease() {
  Serial.println("Sensor Stop released!");
}