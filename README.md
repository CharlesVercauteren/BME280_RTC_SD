# BME280_RTC_SD

Temperature/pressure/humidity measuring with BME280.<br>
Logging with Adafruit datalogger (Real time clock + SD-card).<br>
Tested on Arduino Uno.<br>
<br>
SD-card must be formatted in FAT16. The SD-card can be removed to read the log files, but don't forget to reset the Arduino after the SD-card is placed back in the datalogger.<br>
De log-files are named MMDD.txt. Initial logging interval is 15 minutes. This can be changed in the sketch, zie #define LOG_INTERVAL.<br>
