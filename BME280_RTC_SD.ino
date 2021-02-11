
// BME280_RTC_SD
//
// ©2021 Charles Vercauteren
// 11 februari 2021

#define LOG_INTERVAL   15    // Minuten, moet <60

// BME280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

// SD-Card
#include <SPI.h>
#include <SD.h>
Sd2Card card;
SdVolume volume;
SdFile root;
//char file[64] = "dummy";


// RTC
#include "RTClib.h"
RTC_PCF8523 rtc;

const int chipSelect = 10;

// Allerlei
bool runOnce = false;

//File logFile;
int logInterval = LOG_INTERVAL;
DateTime now;


void setup() {
  //Allerlei setup
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("BME280_RTC_SD."));
  Serial.println(F("-------------"));

  // BME280 setup.
  // -------------
  // Sensor zit op I2C adres 0x76
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
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
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
    // Set datum en tijd op compilatiemoment.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
}

void loop(void) {
  File rootDir;
  File logFile;
  String fileName;
  String logString;
  static DateTime prevNow;
  //String timeString, dateString, logString;
  sensors_event_t temp_event, pressure_event, humidity_event;

  // Toon inhoud SD-Card
  if (!runOnce) {
    runOnce = true;

    Serial.print("Log interval: ");
    Serial.println(logInterval);
  
  }

  // Tijd om te loggen ?
  prevNow = now;
  now = rtc.now();
  if (now.minute()%logInterval == 0 && prevNow.minute()%logInterval != now.minute()%logInterval) {
    Serial.println("\nLogging.");
    
    fileName = buildFileNameString();
    Serial.print("Bestandsnaam: "); Serial.println(fileName);

    // Meet en log sensorgegevens
    logString = readSensorAndLog(fileName);
    Serial.println("Tijd, temperatuur, druk en vochtigheid: ");
    Serial.println(logString);
    
    // Toon bestandsgrootte
    Serial.print("Bestandsgrootte: ");
    logFile = SD.open(fileName.c_str(),FILE_READ);
    Serial.println(logFile.size());
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
  /*
  // Debug
  if (now.day() < 10) { temp += "0";}
  temp += String(now.day());
  if (now.hour() < 10) { temp += "0";}
  temp += String(now.hour()) + ".txt";
  */
  
  return temp;
}

String readSensorAndLog(String fileName) {
  File logFile;
  String tempData, pressData, humiData;
  String logString;
  
  String timeString = buildTimeString();
  String dateString = buildDateString();

  //strcpy(file, fileName.c_str());

  // Lees sensor
  tempData = String(bme.readTemperature());
  pressData = String(bme.readPressure()/100); // In hPa
  humiData = String(bme.readHumidity());
    
  // Bouw te loggen string
  logString = timeString + "\t" + tempData + "\t" + pressData + "\t" + humiData + "\n";

  
  // Schrijf naar SD-card
  logFile = SD.open(fileName.c_str(),FILE_WRITE);
  logFile.write(logString.c_str());
  logFile.close();

  return logString;

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
/*
void printDirectory(File dir, int numTabs) {

  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }

    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }

    Serial.print(entry.name());

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } 
    else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();

    }
}*/
