/*
   * This code is used to calibrate the pH-4502C to read 0V when measuring 0pH.
   * To calibrate shortcircut the BNC with a wire, and turn the bolt on the potentiometer closest to the BNC until the serial monitor reads 2.5V
*/
float value;
float pH;
float voltage;

void setup()
{
  Serial.begin(115200);
  pinMode(value, INPUT);

}
void loop()
{
  value = analogRead(36);
  voltage = value * (3.3 / 4096.0); // converts analog read to voltage, (expected voltage, ADC resolution)
  Serial.print(voltage);
  Serial.print("|");
  pH = (voltage * 3.3);
  Serial.println(pH);
  delay(500);
}
