/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
 outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.

 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Use this calibration_factor on the example sketch

 This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
 calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).

 Your calibration factor may be very positive or very negative. It all depends on the setup of your scale system
 and the direction the sensors deflect from zero state
 This example code uses bogde's excellent library: https://github.com/bogde/HX711
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE
 Arduino pin 2 -> HX711 CLK
 3 -> DOUT
 5V -> VCC
 GND -> GND

 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

*/
#include "Arduino.h"
#include "HX711.h"

#define DOUT  3
#define CLK  2
// which analog pin to connect
#define THERMISTOR_PIN A7
// resistance at 25 degrees C
#define THERMISTOR_NOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURE_NOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUM_SAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define B_COEFFICIENT 3950
// the value of the 'other' resistor
#define SERIES_RESISTOR 10000

void displayTemperature();

HX711 scale(DOUT, CLK);
int samples[NUM_SAMPLES];

float calibration_factor = -654; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
//  Serial.println("HX711 calibration sketch");
//  Serial.println("Remove all weight from scale");
//  Serial.println("After readings begin, place known weight on scale");
//  Serial.println("Press + or a to increase calibration factor");
//  Serial.println("Press - or z to decrease calibration factor");

  scale.set_scale(calibration_factor);
  delay(1000);  // Small delay for settling
  scale.tare(); // Reset the scale to 0

//  float zero_factor = scale.get_units(10); //Get a baseline reading
//  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor);
}

void loop() {

  Serial.print(scale.get_units(10));
  Serial.print("\t");
  displayTemperature();

  delay(1000);

  if(Serial.available()) {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a') {
      calibration_factor += 10;
      Serial.print("Calibration_factor: ");
      Serial.print(calibration_factor);
      Serial.println();
    } else if(temp == '-' || temp == 'z') {
      calibration_factor -= 10;
      Serial.print("Calibration_factor: ");
      Serial.print(calibration_factor);
      Serial.println();
    } else if (temp == 'r' || temp == 'R') {
      scale.set_scale(calibration_factor); //Adjust to this calibration factor

      Serial.print("Reading: ");
      Serial.print(scale.get_units(10), 5);
      Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
      Serial.print("; calibration_factor: ");
      Serial.print(calibration_factor);
      Serial.println();
    } else if (temp == 't' || temp == 'T') {
      scale.tare(10);
      Serial.println("Tare complete!");
    }
  }
}

void displayTemperature() {
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i=0; i< NUM_SAMPLES; i++) {
    samples[i] = analogRead(THERMISTOR_PIN);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (i=0; i< NUM_SAMPLES; i++) {
    average += samples[i];
  }
  average /= NUM_SAMPLES;

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIES_RESISTOR / average;

  double steinhart;
  steinhart = average / THERMISTOR_NOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= B_COEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

//  Serial.print("Temperature ");
  Serial.println(steinhart);
//  Serial.println(" *C");
}