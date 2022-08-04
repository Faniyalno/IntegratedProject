#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHT11_PIN 8 // connecting to digital pin D8
DHT DHT(DHT11_PIN,DHT11);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  DHT.begin();
}

void loop() {
  // declare var sensor
  float tempC =DHT.readTemperature();
  float hum = DHT.readHumidity();
  int LDR = analogRead(0); // analog pin A0
  int rainsens = analogRead(1); // analog pin A1
  String data = String(LDR)+"#"+String(rainsens)+"#"+String(tempC)+"#"+String(hum);
  //print & send data to nodeMcu
  Serial.println(data);
  delay(60000);
}
