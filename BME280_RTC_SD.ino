
// BME280_RTC_SD
//
// ©2021 Charles Vercauteren
// 11 februari 2021

// Tested with:
//   - Arduino Uno
//   - Adafruit Datalogging shield
//   - BME280 temperature/pressure/humidity sensor connected to I2C

#define LOG_INTERVAL   2    // Minutes, > 1 and <60

// BME280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

// SD-Card
#include <SPI.h>
#include <SD.h>
SDClass card;
SdFile root;

// RTC
#include "RTClib.h"
RTC_PCF8523 rtc;
const int chipSelect = 10;

// Diverse
bool runOnce = false;
int logInterval = LOG_INTERVAL;
DateTime now;


void setup() {
  //Diverse setup
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("BME280_RTC_SD."));
  Serial.println(F("-------------"));

  // BME280 setup.
  // -------------
  // Sensor is on I2C address 0x76
  // Sensor ID is 0x60

  if (!bme.begin(0x76)) {
    Serial.println(F("No sensor found, stopping"));
    while (1);
  }
  else {
    Serial.println(F("BME280 sensor found."));
  }
   // Default settings from datasheet.
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode. 
                  Adafruit_BME280::SAMPLING_X2,     // Temp. oversampling 
                  Adafruit_BME280::SAMPLING_X16,    // Pressure oversampling 
                  Adafruit_BME280::SAMPLING_X16,    // Humidity 
                  Adafruit_BME280::FILTER_X16,      // Filtering. 
                  Adafruit_BME280::STANDBY_MS_500); // Standby time. 

  // SD-card setup.
  // --------------
  if (!card.begin()) {
    Serial.println(F("No SD card found, stopping."));
    while(1);
  }
  Serial.println(F("SD-card found."));
  SD.begin(chipSelect);
  now = rtc.now();
  Serial.println(buildTimeString());


  // RTC setup.
  // ----------
  if (! rtc.begin()) {
    Serial.println(F("No RTC found, stopping."));
    while(1);
  }
  Serial.println(F("Real Time Clock found."));
  
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println(F("RTC is NOT initialized, let's set the time!"));
    // Set date and time to moment of compilation
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop(void) {
  File rootDir;
  File logFile;
  String fileName;
  String logString;
  static DateTime prevNow;
  sensors_event_t temp_event, pressure_event, humidity_event;

  // Code executed once
  if (!runOnce) {
    runOnce = true;

    Serial.println();
    Serial.print("Log interval: ");
    Serial.println(logInterval);

    Serial.println();
    Serial.println("Time\t\tTemp\tPress\tHumi\t\tFile / Size:");
  }

  // Time to log ?
  prevNow = now;
  now = rtc.now();
  if (now.minute()%logInterval == 0 && prevNow.minute()%logInterval != now.minute()%logInterval) {
    
    fileName = buildFileNameString();
 
    // Get&show sensor data
    logString = readSensorAndSaveToLog(fileName);
    Serial.print(logString);
    Serial.print("\t\t");
    Serial.print(fileName);
    
    // Toon bestandsgrootte
    logFile = SD.open(fileName.c_str(),FILE_READ);
    Serial.print(" / ");
    Serial.print(logFile.size());
    Serial.println();
    logFile.close();
    }

}

String buildFileNameString(){
  // Moet voldoen aan 8.3 --> werkt anders niet !!!
  // Maak string "jjjjmmdd.csv"
  String temp = "";
  
  //temp += String(now.year());
  if (now.month() < 10) { temp += "0";}
  temp += String(now.month());
  if (now.day() < 10) { temp += "0";}
  temp += String(now.day()) + ".txt";
  
  return temp;
}

String readSensorAndSaveToLog(String fileName) {
  // Returns time and sensor data as a string.
  // timeString + "\t" + tempData + "\t" + pressData + "\t" + humiData
  File logFile;
  String tempData, pressData, humiData;
  String dataString, logString;
  
  String timeString = buildTimeString();
  String dateString = buildDateString();


  // Lees sensor
  tempData = String(bme.readTemperature());
  pressData = String(bme.readPressure()/100, 0); // In hPa
  humiData = String(bme.readHumidity());
    
  // Bouw te loggen string
  dataString = timeString + "\t" + tempData + "\t" + pressData + "\t" + humiData;
  logString = dataString + "\n";

  // Schrijf naar SD-card
  logFile = SD.open(fileName.c_str(),FILE_WRITE);
  logFile.write(logString.c_str());
  logFile.close();

  return dataString;

}

String buildTimeString() {
  // Maak string huidige tijd vd vorm hh:mm:ss
  String temp = "";
  
  if (now.hour() < 10) { temp += "0";}
    temp += String(now.hour()) + ":";
    if (now.minute() < 10) { temp += "0";}
    temp += String(now.minute()) + ":";
    if (now.second() < 10) { temp += "0";}
    temp += String(now.second());

    return temp;
}

String buildDateString() {
  // Maak string van huidige datum in de vorm dd/mm/jjjj
    String temp = "";
  
  if (now.day() < 10) { temp += "0";}
    temp += String(now.day()) + "/";
    if (now.month() < 10) { temp += "0";}
    temp += String(now.month()) + "/";
    temp += String(now.year());

    return temp;
}
