/*
  Name : Count time to SD card
  Author : Jakkapan Attala
  Date : 14/03/2023
  Version : 2.0.0
*/
#include <Arduino.h>
#include <TM1637Display.h>
#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <SPI.h>
#include <SD.h>
#include <DS1302.h>

const uint8_t SEG_WAIT[] = {
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
};
const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const uint8_t SEG_ERROR[] = {
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,  // E
  SEG_E | SEG_G,                          // r
  SEG_E | SEG_G,                          // r
  SEG_C | SEG_D | SEG_E | SEG_G,          // o
};
const uint8_t SEG_[] = {
  0b00000000,  // -
  0b00000000,  // -
  0b00000000,  // -
  SEG_G,       // -
};

#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK 13

// Ds1302
#define DATA_PIN 4
#define CLK_PIN 3
#define RST_PIN 2
DS1302 rtc(RST_PIN, DATA_PIN, CLK_PIN);

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
TcBUTTON button_esc(BUTTON_ESC, isButtonPressESC, NULL);

void isButtonPressDOWN();
void isButtonReleaseDOWN();
TcBUTTON button_down(BUTTON_DOWN, isButtonPressDOWN, NULL);

void isButtonPressUP();
void isButtonReleaseUP();
TcBUTTON button_up(BUTTON_UP, isButtonPressUP, NULL);

void isButtonPressENTER();
void isButtonReleaseENTER();
TcBUTTON button_enter(BUTTON_ENTER, isButtonPressENTER, NULL);
// Sensor
#define SENSOR_START 7  //
#define SENSOR_STOP 8   //

void isSensorStartPress();
void isSensorStartRelease();
TcBUTTON sensor_start(SENSOR_START, isSensorStartPress, isSensorStartRelease);

void isSensorStopPress();
void isSensorStopRelease();
TcBUTTON sensor_stop(SENSOR_STOP, isSensorStopPress, isSensorStopRelease);
// LED
#define LED_STATUS 9  // 9
TcPINOUT led_status(LED_STATUS, true);

// Variable
String _header[4]{ "Date", "TimeStart", "TimeEnd", "Time Total(Sec)" };
String _dataString[4] = { "-", "-", "-", " -" };

int count_start = 0;
int current_mode[3] = { 0, 0, 0 };
uint8_t date = 0;
uint8_t month = 0;
uint16_t year = 0;
uint8_t hou = 0;
uint8_t minute = 0;
uint8_t second = 0;

int period = 500;
unsigned long last_time_ms = 0;
unsigned long last_time_cs = 0;

bool isStarted = false;
bool isReady = false;
int count_Ready = 0;
int count_started = 0;
Time last_time;
String filename = "Data.csv";
Time time;
void saveFile(void);
void mainMenu(void);
void subMenu(void);
int getMonth(String);
void mainTimer(void);
void setBrightnessDisplay(void);
String getArrayToString(String[], int);
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
  time = rtc.getTime();
  filename = "D" + String(time.mon) + String(time.date) + "S.csv";

  led_status.off();
  Serial.println(getArrayToString(_header, 4));
  Serial.println("Setup complete");
  display.showNumberDec(0, false, 4, 0);
}
bool isSave = false;
bool isRecord = false;
void loop() {
  if (isRecord) {
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
  mainTimer();
  if (count_started < 100) {
    count_started++;
  } else if (!isStarted) {
    isStarted = true;
  }

  if (isSave) {
    Serial.println(rtc.getDateStr());
    _dataString[0] = rtc.getDateStr();
    _dataString[2] = rtc.getTimeStr();
    _dataString[3] = String(count_start);

    for (int j = 0; j < 4; j++) {
      Serial.print("Data : ");
      Serial.println(_dataString[j]);
    }
    Serial.println("Filename : " + filename);
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) { 
      // Check Header 
      if (dataFile.size() == 0) {
        for(int k = 0; k < 4; k++){
          dataFile.print(_header[k]);
          dataFile.print(",");
        }
        dataFile.println("");
      }
      // dataFile.println(sa);
      for (int j = 0; j < 4; j++) {
        Serial.print("Data : ");
        dataFile.print(_dataString[j]);
        dataFile.print(",");
        Serial.println(_dataString[j]);
      }
      dataFile.println("");
      dataFile.close();
      Serial.println("Done!.");
      for (int i = 0; i < 5; i++) {
        display.setBrightness(5, true);
        display.setSegments(SEG_DONE);
        delay(50);
        display.setBrightness(0, false);
        display.setSegments(SEG_DONE);
        delay(50);
      }
      display.setBrightness(5, true);
      display.setSegments(SEG_DONE);
    } else {
      Serial.println("File not found!");
      for (int i = 0; i < 10; i++) {
        display.setBrightness(5, true);
        display.setSegments(SEG_ERROR);
        delay(50);
        display.setBrightness(0, false);
        display.setSegments(SEG_ERROR);
        delay(50);
        led_status.on();
      }
      led_status.off();
      display.setBrightness(5, true);
      display.setSegments(SEG_ERROR);
    }
    count_Ready = 0;
    isReady = true;
    isSave = false;
  }
}
void mainTimer() {
  if (millis() - last_time_cs >= 1000) {
    if (isReady) {
      count_Ready++;
      if (count_Ready >= 10) {
        isReady = false;
        count_Ready = 0;
        display.showNumberDec(0, false, 4, 0);
      }
      last_time_cs = millis();
    } else if (millis() < 1000) {
      last_time_cs = millis();
    }
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
            current_mode[2] += 1;
            // Serial.print("CUrr : ");
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
            hou = time.hour;
            minute = time.min;
            second = time.sec;
            // Serial.print("h : ");
            // Serial.println(hou);
            // Serial.print("Minute : ");
            // Serial.println(minute);
            // Serial.print("Second : ");
            // Serial.println(second);
            current_mode[2] += 1;
            display.clear();
          } else

            if (current_mode[2] == 2) {
            // Display Hour
            setBrightnessDisplay();
            display.showNumberDec(hou, false, 4, 0);
          } else if (current_mode[2] == 3) {
            // Display Minute
            setBrightnessDisplay();
            display.showNumberDec(minute, false, 4, 0);
          } else if (current_mode[2] == 4) {
            // Display Second
            setBrightnessDisplay();
            display.showNumberDec(second, false, 4, 0);
          } else {
            rtc.setTime(hou, minute, second);
            current_mode[2] = 0;
            Serial.println("Set Time Complete");
          }

          break;
        case 2:
          // 2 Name
          current_mode[1] = 0;
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
    hou -= 1;
    if (hou < 0 || hou > 23) {
      hou = 23;
    }
    Serial.println(hou);
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
    hou += 1;
    if (hou > 23) {
      hou = 0;
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
  if (isRecord) {
    return;
  }
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
  if (isRecord || !isStarted) {
    return;
  } else if (current_mode[0] != 0 || current_mode[1] != 0 || current_mode[2] != 0) {
    return;
  }

  isRecord = true;
  // led_status.on();
  Serial.print("Status LED: ");
  Serial.println(led_status.isOn());
  count_start = -1;
  _dataString[1] = rtc.getTimeStr();
  // _data_02 = rtc.getTimeStr();
  // Serial.println("Data 02 :" + _data_02);
  isReady = false;
}

void isSensorStopPress() {
  Serial.println("Sensor Stop pressed!");
  //  led_status.off();
  if (!isRecord || !isStarted) {
    return;
  } else if (current_mode[0] != 0 || current_mode[1] != 0 || current_mode[2] != 0) {
    return;
  }
  // led_status.off();
  // Serial.print("Status LED: ");
  // Serial.println(led_status.isOn());

  isSave = true;
  isRecord = false;

  // saveFile();
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
