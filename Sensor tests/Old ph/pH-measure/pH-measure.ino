float calibration = 18.78; //change this value to calibrate. This will be found in the linear regression.
const int analogInPin = 36;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10], temp;
void setup() {
  Serial.begin(115200);
}
void loop() {
  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(analogInPin);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];
  float pHVol = (float)avgValue * 3.3 / 4092 / 6; // changed to 3.3v and 4092 ADC resolution
  float phValue = -4.48 * pHVol + calibration + 0.3; // these values will be found through linear regression of the calibration
  Serial.print("sensor = ");
  Serial.println(phValue);
  delay(500);
}
