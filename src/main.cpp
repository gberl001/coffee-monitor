#include "Arduino.h"
#include "HX711.h"
#include <LiquidCrystal.h>

#define DOUT  3
#define CLK  2
#define EMPTY 69        // An empty carafe is about 69.25 ounces
#define FULL_CUP 10     // A full cup is about 10 ounces
#define SPLAT 73        // At this point you'll get splatter
#define FULL 150        // A full pot

HX711 scale(DOUT, CLK);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
double lastBrewTime = 0;

float calibration_factor = -654; //-7050 worked for my 440lb max scale setup

void initScale();

double cupsRemaining(double reading);

bool scaleIsEmpty(double reading);

void waitForNewBrew();

double getAge();

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);                           // Setup LCD

  // Get a good baseline
  lcd.print("Please Wait...");
  scale.set_scale(calibration_factor);
  delay(1000);  // Small delay for settling
  scale.tare(); // Reset the scale to 0

//  float zero_factor = scale.get_units(10); //Get a baseline reading
//  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor);

  // Setup the scale
  initScale();
}


void loop() {

  // Take a reading
  double reading = scale.get_units(10);

  // Determine the state
  if (reading <= SPLAT && !scaleIsEmpty(reading)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Empty Container");
  } else if (reading > SPLAT && !scaleIsEmpty(reading)) {
    // Calculate the cups remaining
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Age: ");
    lcd.print(getAge());
    lcd.setCursor(0, 1);
    lcd.print("Cups Left: ");
    lcd.print(cupsRemaining(reading));
  } else if (scaleIsEmpty(reading)) {
    // Assume a new batch is being brewed...
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting for");
    lcd.setCursor(0, 1);
    lcd.print("next brew");
    waitForNewBrew();
    lastBrewTime = millis();
  }



  delay(1000);
}

double getAge() {
  return (millis() - lastBrewTime)/1000/60;
}

void waitForNewBrew() {
  while (scaleIsEmpty(scale.get_units(10))) {
    delay(4000);
  }
}

bool scaleIsEmpty(double reading) {
  return reading < 10.0;
}

double cupsRemaining(double reading) {
  return (reading - SPLAT) / FULL_CUP;
}

void initScale() {
  // Ready
  lcd.clear();
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("Add container");

  // Wait for scale to have weight added to it.
  while (scale.get_units(10) < 1.0) {
    delay(4000);
  }

//  while(true) {
//    lcd.clear();
//    lcd.print(newReading = scale.get_units(10));
//
//    // Determine if it's empty or not
//    if (newReading < EMPTY * 1.15 && newReading > EMPTY * 0.85) {
//      lcd.setCursor(0, 1);
//      lcd.print("Empty!");
//    } else {
//      lcd.setCursor(0, 1);
//      lcd.print(newReading);
//    }
//    delay(1000);
//  }


}