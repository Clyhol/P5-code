#include "DHT.h"
#define DHTPIN 17 // Correct pin for the VF is 17

DHT dht(DHTPIN, DHT22); //Maybe changed to DHT11

void setup() {
  Serial.begin(115200);
  dht.begin(); //Initiering af DHT22
}

void loop() {
  Serial.print("temp:");
  float temp = dht.readTemperature();
  Serial.println(temp);
  Serial.print("humi:");
  float hum = dht.readHumidity();
  Serial.println(hum);
  delay(2000);
}
