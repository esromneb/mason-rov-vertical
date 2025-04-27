#include <Wire.h>
#include "MS5837.h"

// on 2nd tethere
// yellow 3.3
// green  gnd
// blue orange st                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ripe  / both brown /  scl  A5
// purp green stripe  / both blue / sda. A4

// #define TRIGGER_PIN 2    // Trigger pin (D2) on Arduino Nano
// #define ECHO_PIN 3       // Echo pin (D3) on Arduino Nano

#define MOTOR_IN1 5
#define MOTOR_IN2 6
#define MOTOR_ENABLE 8

#define BUTTON_RED    9
#define BUTTON_GREEN  11


// Create states
// each one needs it's own number
#define START 0
#define SURFACE_MEASUREMENT 1
#define DIVE_MIDDLE 2  // going to the middle
#define AT_MIDDLE 3  // got to the middle
#define DIVE_BOTTOM 4
#define AT_BOTTOM 5
#define RETURN_SURFACE 6
#define MANUAL_CONTROL 7



// Variables for distance measurement
long duration;
float distance;

// Variable needed for the state machine
int state = START;

MS5837 sensor;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tunables

// Change speeds here
#define DIVE_SPEED 140
#define REV_BURST 160
#define PARK_SPEED 0
#define UP_SPEED 255

// Middle target
// 137 is 4.5 ft
// 137 plus or minus 15 gives us a good window
#define MIDDLE_LOW (1.22-0.2)
#define MIDDLE_HIGH (1.52+0.2)


// Bottom target
// sensor doesn't read below 19 usually
#define BOTTOM_LOW 19
#define BOTTOM_HIGH 35


// IN METERS NOT FEET
// set to 0, observe the depth in meters, then set
// When this is set to 0, if depth reads negative in air, set this to the positive number it reads
#define DEPTH_OFFSET 0.05

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("Hello");


  // Set the trigger pin as OUTPUT and echo pin as INPUT
  // pinMode(TRIGGER_PIN, OUTPUT);
  // pinMode(ECHO_PIN, INPUT);

  // Setup motor
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);

  // setup i2c for pressure sensor
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

// returns the distance in CM
// if the return is -1 then ignore the reading
float getDistance() {
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

  return depthm;
}

// descend with the motor at speed
void motorDown(int speed) {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENABLE, speed);
}

void motorUp(int speed) {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  analogWrite(MOTOR_ENABLE, speed);
}

void motorOff() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENABLE, 0);
}

// tested the motor speeds with this
void loop2() {
  Serial.println("Dive Speed");
  motorDown(DIVE_SPEED);
  delay(5000);

  Serial.println("Park Speed");
  motorDown(PARK_SPEED);
  delay(5000);

  Serial.println("up Speed");
  motorUp(UP_SPEED);
  delay(5000);
}

void check_buttons() {
  bool red_press = !digitalRead(BUTTON_RED);
  bool green_press = !digitalRead(BUTTON_GREEN);
  if(red_press || green_press) {
    state = MANUAL_CONTROL;
  }
}

void loop() {
  // Clear the previous reading
  // lcd.clear();
 
  delay(30);
  distance = getDistance();
  Serial.println(distance);

  bool red_press = !digitalRead(BUTTON_RED);
  bool green_press = !digitalRead(BUTTON_GREEN);

  check_buttons();


  // every case must have a break
  switch(state) {
    default:
    case START:
      Serial.println("start");
      motorOff();
      state = SURFACE_MEASUREMENT;
      break;
    case SURFACE_MEASUREMENT:
      Serial.println("surface measurement");
      check_buttons();
      delay(5000);
      check_buttons();
      state = DIVE_MIDDLE;
      break;
    case DIVE_MIDDLE:
      motorDown(DIVE_SPEED);
      Serial.println("dive middle");

      // 1.37 is 4.5 ft
      // 1.37 plus or minus 15 gives us a good window
      if(distance >= MIDDLE_LOW && distance < MIDDLE_HIGH) {
        // we're at the middle
        state = AT_MIDDLE;
      }

     
      break;
    case AT_MIDDLE:
      Serial.println("at middle");
      // motorDown(PARK_SPEED);
      motorUp(REV_BURST);
      delay(900);
      motorOff();
      delay(5000);
      check_buttons();
      state = DIVE_BOTTOM;
      break;
    case DIVE_BOTTOM:
      Serial.println("Dive to bottom");
      motorDown(DIVE_SPEED);

      delay(15000);
      check_buttons();
      state = AT_BOTTOM;
      // if(distance >= BOTTOM_LOW && distance < BOTTOM_HIGH) {
      //   // we're at the middle
      //   state = AT_BOTTOM      
      // }

      break;
    case AT_BOTTOM:
      Serial.println("at bottom");
      motorDown(PARK_SPEED);
      delay(5000);
      check_buttons();
      state = RETURN_SURFACE;
      break;
    case RETURN_SURFACE:
      Serial.println("return to surface");
      motorUp(UP_SPEED);
      break;
    case MANUAL_CONTROL:
     
      if(green_press) {

        motorDown(DIVE_SPEED);
        Serial.println("Manual DOWN");
        delay(50);
      } else if(red_press) {
        motorUp(UP_SPEED);
        Serial.println("Manual UP");
        delay(50);
      } else {
        motorOff();
        Serial.println("In Manual");
      }
      break;
  }


}
