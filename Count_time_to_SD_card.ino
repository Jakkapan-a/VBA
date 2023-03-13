#include <Arduino.h>
#include <TM1637Display.h>
#include <BUTTON.h>
#include <PINOUT.h>
#include <SD.h>
// #include <DS1302.h>

// SD Card
#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK 13

// TM1637
#define CLK 4
#define DIO 5
TM1637Display display(CLK, DIO);

// Button
#define BUTTON_ESC 21
#define BUTTON_UP 20
#define BUTTON_DOWN 17
#define BUTTON_ENTER 16

BUTTON button_esc(BUTTON_ESC);
BUTTON button_up(BUTTON_UP);
BUTTON button_down(BUTTON_DOWN);
BUTTON button_enter(BUTTON_ENTER);

// Clock and Timer SD1302
#define DATA_PIN 2
#define CLK_PIN 3
#define RST_PIN 4

String header = "DateTime, TimeStart, TimeEnd, Time Total, Count Start, Count End, Count Total";
String dataString = "";

File dataFile;

void saveFile(String);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card not found");
    while (1);
  }
  Serial.println("SD card found");
  
  File dataFile = SD.open("test.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("test");
    dataFile.close();
  }
  else {
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}


