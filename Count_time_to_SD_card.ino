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
#include <SPI.h>
#include <SD.h>
#include <DS1302.h>

const uint8_t SEG_P01[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
  SEG_B | SEG_C,                                  // 1
};

const uint8_t SEG_P02[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,          // 2
};

const uint8_t SEG_WAIT[] = {
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
};

const uint8_t SEG_[] = {
  0b00000000,  // -
  0b00000000,  // -
  0b00000000,  // -
  SEG_G,       // -
};

// SD Card
#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK 13
File dataFile;

// TM1637 Display
#define CLK 5
#define DIO 6
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
#define SENSOR_START 7  //
#define SENSOR_STOP 8   //

void isSensorStartPress();
void isSensorStartRelease();
BUTTON sensor_start(SENSOR_START, isSensorStartPress, isSensorStartRelease);

void isSensorStopPress();
void isSensorStopRelease();
BUTTON sensor_stop(SENSOR_STOP, isSensorStopPress, isSensorStopRelease);
// LED
#define LED_STATUS 9  // A6

PINOUT led_status(LED_STATUS, true);

// Ds1302
#define DATA_PIN 4
#define CLK_PIN 3
#define RST_PIN 2
DS1302 rtc(RST_PIN, DATA_PIN, CLK_PIN);
Time time;
// Variable
String _header[4]{ "DateTime", "TimeStart", "TimeEnd", "Time Total(Sec)" };
String _dataString[4] = { "55", "55", "55", " 55" };
String fileName = "";
int count_start = 0;
int current_mode[3] = { 0, 0, 0 };
uint8_t date = 0;
uint8_t month = 0;
uint16_t year = 0;
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;

unsigned long period = 500;
unsigned long last_time_ms = 0;

bool isStarted = false;
int count_started = 0;
Time last_time;

void saveFile();
void mainMenu(void);
void subMenu(void);
int getMonth(String);
void setBrightnessDisplay(void);
String getArrayToString(String[], int size);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Display
  display.setBrightness(5);
  display.setSegments(SEG_);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card not found");
    while (1) {
      display.setBrightness(5, true);
      display.setSegments(SEG_WAIT);
      delay(500);
      display.setBrightness(0, false);
      display.setSegments(SEG_WAIT);
      delay(500);
    }
  }


  rtc.halt(false);          // Enable RTC clock
  rtc.writeProtect(false);  // Enable write to RTC
  // rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  // rtc.setDate(11,3, 2023);   // Set the date to August 6th, 2010

  // time = rtc.getTime();
  // date = time.date;
  // month = time.mon;
  // year = time.year;
  // String filename = "D" + String(time.year) + String(time.mon) + String(time.date) + ".csv";
  // // If file exists, append to it otherwise create a new file add header
  // //  dataFile = SD.open(filename, FILE_WRITE);
  // // if (!SD.exists(filename)) {
  // //   // Serial.println("DATA.txt exists.");
  // //   String _strs = getArrayToString(_header, 4);
  // //   dataFile = SD.open(filename, FILE_WRITE);
  // //   if (dataFile) {
  // //     dataFile.println(_strs);
  // //     dataFile.close();


  // //     Serial.println("Done.");
  // //   } else {
  // //     Serial.println("File not found 1");
  // //   }
  // // }
  // // String _strs = getArrayToString(_dataString, 4);
  // // dataFile = SD.open(filename, FILE_WRITE);
  // // if (dataFile) {
  // //   dataFile.println(_strs);
  // //   dataFile.close();
  // //   Serial.println("Done 2.");
  // // } else {
  // //   Serial.println("File not found 2");
  // // }



  time = rtc.getTime();
  // last_time = time;
  led_status.off();
  Serial.println(getArrayToString(_header, 4));
  Serial.println("Setup complete");
  display.showNumberDec(0, false, 4, 0);
}


void loop() {
  if (led_status.isOn()) {
    time = rtc.getTime();
    if (time.sec != last_time.sec) {
      last_time = time;
      count_start++;
      display.showNumberDec(count_start, false, 4, 0);
    }
  }

  button_esc.update();
  button_up.update();
  button_down.update();
  button_enter.update();
  sensor_start.update();
  sensor_stop.update();
  mainMenu();
  if (count_started < 100) {
    count_started++;
  } else if (!isStarted) {
    isStarted = true;
  }
}

void mainMenu() {
  // Display
  switch (current_mode[0]) {
    case 0:
      // Normal mode
      // display.showNumberDec(0, false, 4, 0);
      break;
    case 1:
      // F001 Mode Setting Date Time
      // SubMenu
      switch (current_mode[1]) {
        case 0:
          // 0 DATE
          if (current_mode[2] == 0) {
            display.showNumberHexEx(0xF000);

          } else if (current_mode[2] == 1) {
            // Display Date
            time = rtc.getTime();
            date = time.date;
            month = time.mon;
            year = time.year;
            Serial.print("Date : ");
            Serial.println(date);
            Serial.print("Month : ");
            Serial.println(month);
            Serial.print("Year : ");
            Serial.println(year);
            current_mode[2] += 1;
            Serial.print("CUrr : ");
            Serial.println(current_mode[2]);
            display.clear();
          } else if (current_mode[2] == 2) {
            // Display Date
            setBrightnessDisplay();
            display.showNumberDec(date, false, 4, 0);

          } else if (current_mode[2] == 3) {
            // Display Month
            setBrightnessDisplay();
            display.showNumberDec(month, false, 4, 0);
          } else if (current_mode[2] == 4) {
            // Display Year
            setBrightnessDisplay();
            display.showNumberDec(year, false, 4, 0);
          } else {
            rtc.setDate(date, month, year);
            current_mode[2] = 0;
            Serial.println("Set Date Complete");
          }
          break;
        case 1:
          // 1 Time
          if (current_mode[2] == 0) {
            display.showNumberHexEx(0xF001);
          } else if (current_mode[2] == 1) {
            // Get time
            time = rtc.getTime();
            hour = time.hour;
            minute = time.min;
            second = time.sec;
            Serial.print("Hour : ");
            Serial.println(hour);
            Serial.print("Minute : ");
            Serial.println(minute);
            Serial.print("Second : ");
            Serial.println(second);
            current_mode[2] += 1;
            display.clear();
          } else if (current_mode[2] == 2) {
            // Display Hour
            setBrightnessDisplay();
            display.showNumberDec(hour, false, 4, 0);
          } else if (current_mode[2] == 3) {
            // Display Minute
            setBrightnessDisplay();
            display.showNumberDec(minute, false, 4, 0);
          } else if (current_mode[2] == 4) {
            // Display Second
            setBrightnessDisplay();
            display.showNumberDec(second, false, 4, 0);
          } else {
            rtc.setTime(hour, minute, second);
            current_mode[2] = 0;
            Serial.println("Set Time Complete");
          }
          break;
        case 2:
          // 2 Name
          if (current_mode[2] == 0) {
            display.showNumberHexEx(0xF002);
          }
          break;
        default:
          current_mode[1] = 0;
          break;
      }
      break;
    default:
      current_mode[0] = 0;
      break;
  }
}
void isButtonPressESC() {
  Serial.println("Button ESC pressed!");
  if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 0) {
    current_mode[0] = 0;
    current_mode[1] = 0;
    current_mode[2] = 0;
  display.showNumberDec(0, false, 4, 0);

  } else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] != 0) {
    current_mode[2] = 0;
  } else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] != 0) {
    current_mode[2] = 0;
  } else if (current_mode[0] == 1 && current_mode[1] == 2 && current_mode[2] != 0) {
    current_mode[2] = 0;
  } else {
    current_mode[0] = 0;
    current_mode[1] = 0;
    current_mode[2] = 0;
  display.showNumberDec(0, false, 4, 0);

  }



  display.setBrightness(5);
}

void isButtonPressDOWN() {
  Serial.println("Button DOWN pressed!");
  if (current_mode[0] == 1 && current_mode[1] != -1 && current_mode[2] == 0) {
    current_mode[1] -= 1;
  }
  // Date
  else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 2) {
    date -= 1;
    if (date < 1 || date > 31) {
      date = 31;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 3) {
    month -= 1;
    if (month < 1 || month > 12) {
      month = 12;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 4) {
    year -= 1;
    if (year < 2000) {
      year = 2100;
    }
  }
  // Time
  else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 2) {
    hour -= 1;
    if (hour < 0 || hour > 23) {
      hour = 23;
    }
    Serial.println(hour);
  } else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 3) {
    minute -= 1;
    if (minute < 0 || minute > 59) {
      minute = 59;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 4) {
    second -= 1;
    if (second < 0 || second > 59) {
      second = 59;
    }
  }
}

void isButtonPressUP() {
  Serial.println("Button UP pressed!");
  if (current_mode[0] == 1 && current_mode[1] != -1 && current_mode[2] == 0) {
    current_mode[1] += 1;
  }
  // Date
  else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 2) {
    date += 1;
    if (date > 31 || date < 1) {
      date = 1;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 3) {
    month += 1;
    if (month > 12 || month < 1) {
      month = 1;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 0 && current_mode[2] == 4) {
    year += 1;
  }
  // Time
  else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 2) {
    hour += 1;
    if (hour > 23) {
      hour = 0;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 3) {
    minute += 1;
    if (minute > 59 || minute < 0) {
      minute = 0;
    }
  } else if (current_mode[0] == 1 && current_mode[1] == 1 && current_mode[2] == 4) {
    second += 1;
    if (second > 59 || second < 0) {
      second = 0;
    }
  }
}

void isButtonPressENTER() {
  Serial.println("Button ENTER pressed!");
  if (current_mode[0] == 0) {
    current_mode[0] = 1;
    current_mode[1] = 0;
    current_mode[2] = 0;
  } else {
    if (current_mode[1] >= 0 && current_mode[0] == 1) {
      current_mode[2] += 1;
    } else {
      current_mode[1] = 0;
    }
  }
}

void isSensorStartPress() {
  Serial.println("Sensor Start pressed!");
}

void isSensorStartRelease() {
  Serial.println("Sensor Start released!");
  if (led_status.isOn() || !isStarted) {
    return;
  } else if (current_mode[0] != 0 || current_mode[1] != 0 || current_mode[2] != 0) {
    return;
  }

  led_status.on();
  Serial.print("Status LED: ");
  Serial.println(led_status.isOn());
  count_start = -1;
  _dataString[1] = rtc.getTimeStr();
}

void isSensorStopPress() {
  Serial.println("Sensor Stop pressed!");
  //  led_status.off();
  if (!led_status.isOn() || !isStarted) {
    return;
  } else if (current_mode[0] != 0 || current_mode[1] != 0 || current_mode[2] != 0) {
    return;
  }
  led_status.off();
  Serial.print("Status LED: ");
  Serial.println(led_status.isOn());

  // Serial.println(rtc.getDateStr());
  String dt = rtc.getDateStr();
  //
  _dataString[0] = dt + " " + rtc.getTimeStr();
  _dataString[2] = rtc.getTimeStr();
  _dataString[3] = String(count_start);
  saveFile();
}

void isSensorStopRelease() {
  Serial.println("Sensor Stop released!");
}

bool isBlink = false;
void setBrightnessDisplay() {
  if (millis() - last_time_ms >= period) {
    isBlink = !isBlink;
    if (isBlink) {
      display.setBrightness(0, true);
    } else {
      display.setBrightness(5, true);
    }
    last_time_ms = millis();
  } else if (millis() < 1000) {
    last_time_ms = millis();
  }
}
String getArrayToString(String arr[], int size) {
  String str = "";
  for (int i = 0; i < size; i++) {
    str += arr[i];
    if (i != (size - 1)) {
      str += ",";
    }
  }
  return str;
}
void saveFile() {
}