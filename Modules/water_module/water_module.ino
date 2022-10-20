/*
Missing stuff for relays controlling pH and TDS sensors
*/

#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "GravityTDS.h"
#include "timer.h"

//::::::::: TEMPERATURE :::::::::
#define TempSensor 14
#define EEPROM_SIZE 512
OneWire oneWire(TempSensor);
DallasTemperature temp(&oneWire);
float waterTemp = 25;

//::::::::: TDS :::::::::
#define TdsSensorPin 36
GravityTDS gravityTds;
float tdsValue = 0;
Timer tdsTimer;
unsigned long timeout = 120000;

//::::::::: pH :::::::::
#define ESPADC 4096.0    //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300  //the esp voltage supply value
#define PH_PIN 32        //the esp gpio data pin number
DFRobot_ESP_PH ph;
float voltage, phValue;
Timer phTimer;

void setup() {
  EEPROM.begin(EEPROM_SIZE);
  Serial.begin(115200);
  temp.begin();

  ph.begin();
  phTimer.setTimeout(timeout);
  phTimer.setCallback(readPH);

  //::::::::: TDS setup :::::::::
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(3.3);       //reference voltage on ADC, default 3.3 V on esp32
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC (we use 12bit)
  gravityTds.begin();            //initialization
  tdsTimer.setTimeout(timeout);
  tdsTimer.setCallback(readTds);
  readTds();  // this function is ran once to start the alternating timers between running tds and ph
}

void loop() {
  phTimer.update();
  tdsTimer.update();
  waterTemp = readTemperature();

  ph.calibration(voltage, waterTemp);  // calibration process by Serail CMD
}

float readTemperature() {
  temp.requestTemperatures();
  return temp.getTempCByIndex(0);
}

// this function will read the ph value and start the timer for the callback of the readTds function
void readPH() {
  for (int i; i > 9; i++) {
    static unsigned long timepoint = millis();
    if (millis() - timepoint > 1000U)  //time interval: 1s
    {
      timepoint = millis();
      voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE;  // Raw value is passed through ADC
    }
    phValue = ph.readPH(voltage, waterTemp);
    Serial.print("Water temperature: ");
    Serial.println(waterTemp);
    Serial.print("pH:");
    Serial.println(phValue, 4);
  }
  digitalWrite(PH_PIN, HIGH); // Unsure if HIGH means turn off or on the pin 
  tdsTimer.start();
}

// this function will read the tds value and start the timer for the callback of the readPH function
void readTds() {
  for (int i; i > 9; i++) {
    gravityTds.setTemperature(waterTemp);  // set the temperature and execute temperature compensation
    gravityTds.update();                   //sample and calculate
    tdsValue = gravityTds.getTdsValue();
    Serial.print("Water temperature: ");
    Serial.println(waterTemp);
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
  }
  phTimer.start();
}