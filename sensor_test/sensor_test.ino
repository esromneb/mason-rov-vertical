// yellow 3.3
// green  gnd
// blue orange stripe   scl  A5
// purp green stripe.   sda. A4

#define BUTTON_RED    9
#define BUTTON_GREEN  8

#include <Wire.h>
#include "MS5837.h"

MS5837 sensor;

void setup() {

  Serial.begin(115200);

  Serial.println("Starting");

  Wire.begin();

  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  while (!sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }

  // .init sets the sensor model for us but we can override it if required.
  // Uncomment the next line to force the sensor model to the MS5837_30BA.
  //sensor.setModel(MS5837::MS5837_30BA);

  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
}

#define DEPTH_OFFSET 0.11

void loop() {
  // Update pressure and temperature readings
  sensor.read();

  Serial.print("Pressure: ");
  Serial.print(sensor.pressure());
  Serial.println(" mbar");

  Serial.print("Temperature: ");
  float tempc = sensor.temperature();
  float tempf =  (tempc * 9.0 / 5.0) + 32.0;
  Serial.print(tempc);
  Serial.print(" deg C,      ");
  Serial.print(tempf);
  Serial.println(" deg F");

  Serial.print("Depth: ");
  float depthm = sensor.depth() + DEPTH_OFFSET;
  float depthf = depthm * 3.28084;
  Serial.print(depthm);
  Serial.print(" m,       ");
  Serial.print(depthf);
  Serial.println(" ft");


  Serial.print("Altitude: ");
  Serial.print(sensor.altitude());
  Serial.println(" m above mean sea level");

  bool red_press = !digitalRead(BUTTON_RED);
  bool green_press = !digitalRead(BUTTON_GREEN);
  
  if(red_press) {
    Serial.println("Red Press");
  }
  if(green_press) {
    Serial.println("Green Press");
  }

  delay(1000);
}
