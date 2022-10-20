/*
This code is used to run the climate module in the vertical farm
The pins are corrected to the relevant pins used in the farm, and should be changed if changed are made to the farm
The fan runs solely based on sensor readings instead of running in time intervals
Timers are used to provide information about runtime and off time to the database and later the interface
Check if statement logic if there are issues with the timers
*/

//::::::::::::: Include necessary libraries :::::::::::::
#include "DHT.h"
//#include <Wire.h>
#include "Adafruit_SGP30.h"
#include "timer.h"

//::::::::::::: Define the relevant pins :::::::::::::
#define DHTPIN 17
#define FAN_RELAY 26
#define LED_RELAY 27

//::::::::::::: Initialize the sensors and timers :::::::::::::
Adafruit_SGP30 sgp;
DHT dht(DHTPIN, DHT22);
Timer fanTimer;
Timer fanOffTimer;
Timer ledTimer;
Timer ledOffTimer;
Timer ledRunTimer;

//::::::::::::: Declare variables :::::::::::::
float temp = 0;
float hum = 0;
int co2 = 0;
int tvoc = 0;
int counter = 0;  // counter used for sgp30 baseline readings
bool fanStatus = false;
bool fanTimerStart = false;
bool fanOffTimerStart = false;
bool ledOn = false;
unsigned long fanRunTime = 0;  // unsigned long for time variables
unsigned long fanOffTime = 0;
unsigned long ledInterval = 0;
unsigned long ledRuntime = 0;
unsigned long ledOffTime = 0;

//Function for getting absolute humidity for correction of CO2 measure
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  // approximation formula
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));  // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                 // [mg/m^3]
  return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(115200);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  dht.begin();
  Serial.println("Humidity sensor found");
  if (!sgp.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  ledControl();

  ledTimer.setCallback(ledControl);
  ledTimer.start();
  sgp.setIAQBaseline(0x9C13, 0x9954);  // Corrected after running for 20+ hours; Values from SGP test used
}
void loop() {
  //::::::::::::: Read values from DHT and SGP30 :::::::::::::
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  sgp.setHumidity(getAbsoluteHumidity(temp, hum));  // apply humidity correction before getting measurements
  co2 = sgp.eCO2;
  tvoc = sgp.TVOC;

  //::::::::::::: print the values to serial monitor :::::::::::::
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(hum);
  Serial.print("CO2: ");
  Serial.println(co2);
  Serial.print("TVOC: ");
  Serial.println(tvoc);

  //::::::::::::: Logic to control the fan :::::::::::::
  
  // if statements are needed as the code is in a loop to avoid starting or stopping timers every loop
  if (co2 > 1500 || hum > 95) {
    digitalWrite(FAN_RELAY, LOW);
    Serial.println("Fan is running");
    fanStatus = true;
    if (fanOffTimerStart == true) {
      fanOffTimer.update();
      fanOffTime = fanOffTimer.getElapsedTime();
      fanOffTimer.stop();
      fanOffTimerStart = false;
    }
    if (fanTimerStart == false) {
      fanTimer.start();
      fanTimerStart = true;
    }
    fanTimer.update();
    fanRunTime = fanTimer.getElapsedTime();
  } else {
    digitalWrite(FAN_RELAY, HIGH);
    Serial.println("Fan is not running");
    fanStatus = false;
    if (fanOffTimerStart == false) {
      fanOffTimer.start();
      fanOffTimer.update();
      fanOffTimerStart = true;
    }
    fanOffTimer.update();
    fanOffTime = fanOffTimer.getElapsedTime();

    if (fanTimerStart == true) {
      fanTimer.update();
      fanRunTime = fanTimer.getElapsedTime();
      fanTimer.stop();
      fanTimerStart = false;
    }
  }

  //::::::::::::: Update led callback timer :::::::::::::
  ledTimer.update();

  if (ledOn == false) { // if statement to only update timer if it is running
    ledOffTimer.update();
    ledOffTime = ledOffTimer.getElapsedTime();
  }

  if (ledOn == true){
    ledRunTimer.update();
    ledRuntime = ledRunTimer.getElapsedTime();
  }

  //::::::::::::: Error control :::::::::::::
  if (!sgp.IAQmeasure()) {
    Serial.println("SGP30 reading failed");
    return;
  }

  if (!dht.readTemperature() || !dht.readHumidity()) {
    Serial.println("DHT reading failed");
  }

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x");
    Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x");
    Serial.println(TVOC_base, HEX);
  }
}

// The function alternates between setting the interval to create a 17:7 hour on/off ratio for the led
void ledControl() {
  if (ledOn == false) {
    digitalWrite(LED_RELAY, LOW);  // led is on
    ledOn = true;
    ledInterval = 61200000;  // 17 hours in milliseconds
    ledTimer.setInterval(ledInterval); // the new interval for the cycle is used

    ledOffTimer.update();
    ledOffTime = ledOffTimer.getElapsedTime();
    ledOffTimer.stop(); // Off timer is updated and saved before being turned off
    
    ledRunTimer.start(); // the run timer is started

  } else if (ledOn == true) {
    digitalWrite(LED_RELAY, HIGH);  // led is off
    ledOn = false;
    ledInterval = 25200000;  // 7 hours in milliseconds
    ledTimer.setInterval(ledInterval); // interval is used
    
    ledRunTimer.update();
    ledRuntime = ledRunTimer.getElapsedTime();
    ledRunTimer.stop(); // run timer is saved and stopped

    ledOffTimer.start(); // off timer is started
  }
}