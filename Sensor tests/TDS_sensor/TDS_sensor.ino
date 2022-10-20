#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>
#include "GravityTDS.h"


#define TdsSensorPin 36
#define TempSensor 14 // Insert which pin in the VF
#define EEPROM_SIZE 512 // When requesting temp it takes a bunch of samples and stores them all. To prevent using all the memory the size is limited (Max: 4096)

OneWire oneWire(TempSensor);
GravityTDS gravityTds;
DallasTemperature temp(&oneWire); 

float temperature = 25,tdsValue = 0; // both of these get overwritten on launch

void setup()
{
    EEPROM.begin(EEPROM_SIZE);
    Serial.begin(115200);
    temp.begin();
    gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(3.3);  //reference voltage on ADC, default 3.3 V on esp32
    gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC (we use 12bit)
    gravityTds.begin();  //initialization
}

void loop()
{
    temp.requestTemperatures();
    temperature = temp.getTempCByIndex(0);  //add temperature sensor and read it
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    Serial.print(tdsValue,0);
    Serial.println("ppm");
    Serial.print(temperature);
    Serial.println("°C");
    delay(1000);
}
