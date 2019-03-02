#include "Arduino.h"
#include "HX711.h"
#include <LiquidCrystal.h>

#define DOUT  3
#define CLK  2
#define FULL_CUP              10          // A full cup is about 10 ounces
#define SPLATTER_POINT        73          // At this point you'll get splatter, and empty carafe is actually 69.25oz
#define FULL_CARAFE           150         // A full carafe is about 150oz
#define EMPTY_SCALE_THRESHOLD 10          // Assume the scale is empty at 10 ounces
#define MINUTES_IN_HOUR     60            // 60 Minutes in an hour
#define MILLIS_IN_MINUTE    60000         // 60000 ms in a minute

// ************ Object and variable declarations **************
HX711 scale(DOUT, CLK);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
long lastBrewTime = 0;
double latestRecordedWeight = 0.0;
float calibration_factor = -654;  // Adjust the weight calibration, tested with 250g and 500g weights

// ********************** Function declarations ****************
void initScale();
double getCupsRemaining(double reading);
bool scaleIsEmpty(double reading);
String getAgeString();
void handleEmptyScale();
void handleCarafeEmpty();
void handleCarafeNotEmpty(double reading);
double getScaleReading();


void setup() {
  // Setup LCD display
  lcd.begin(16, 2);

  // Get a good baseline and tare the scale
  lcd.print("Please Wait...");
  scale.set_scale(calibration_factor);
  delay(1000);  // Small delay for settling
  scale.tare(); // Reset the scale to 0

  // Setup the scale
  initScale();
}


void loop() {

  // Take a reading
  double reading = getScaleReading();

  // Determine the state
  if (reading <= SPLATTER_POINT && !scaleIsEmpty(reading)) {
    handleCarafeEmpty();
  } else if (reading > SPLATTER_POINT && !scaleIsEmpty(reading)) {
    handleCarafeNotEmpty(reading);
  } else if (scaleIsEmpty(reading)) {
    handleEmptyScale();
  }

  delay(1000);
}

// *******************************************************************************************************
// *******************************************************************************************************
// ************************************* State functions *************************************************
// *******************************************************************************************************
// *******************************************************************************************************

void handleCarafeEmpty() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Empty Container");
}

void handleCarafeNotEmpty(double reading) {
  // Display the age and cups remaining
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Age: ");
  lcd.print(getAgeString());
  lcd.setCursor(0, 1);
  lcd.print("Cups Left: ");
  lcd.print(getCupsRemaining(reading));
}

void handleEmptyScale() {
  // Either there is a new brew coming or someone simply lifted the carafe temporarily, record the previous weight
  double previousWeight = latestRecordedWeight;

  // While the scale is empty, display that we're waiting for more coffee
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("next brew");
  while (scaleIsEmpty(getScaleReading())) {
    delay(4000);
  }

  // Now that the scale isn't empty, determine if more coffee was added
  if (latestRecordedWeight > previousWeight + FULL_CUP) {
    // If the new wight is at least one more cup of coffee more than the old, assume a new brew
    lastBrewTime = millis();
  }
}

void initScale() {
  // Ready
  lcd.clear();
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("Add container");

  // Wait for scale to have weight added to it.
  while (getScaleReading() < 1.0) {
    delay(4000);
  }
}

// *******************************************************************************************************
// *******************************************************************************************************
// *********************************** Utility functions *************************************************
// *******************************************************************************************************
// *******************************************************************************************************

String getAgeString() {
  long minutes = (millis() - lastBrewTime) / MILLIS_IN_MINUTE;

  // Compute the hours
  String strHours = String(floor(minutes / MINUTES_IN_HOUR), 0) + "H ";

  // Compute the remaining minutes
  String strMinutes = String(minutes % MINUTES_IN_HOUR) + "M";

  return String(strHours + strMinutes);
}

bool scaleIsEmpty(double reading) {
  return reading < EMPTY_SCALE_THRESHOLD;
}

double getCupsRemaining(double reading) {
  return (reading - SPLATTER_POINT) / FULL_CUP;
}

double getScaleReading() {
  double currentReading = scale.get_units(10);

  // If the scale isn't empty, record the last weight
  if (!scaleIsEmpty(currentReading)) {
    latestRecordedWeight = scale.get_units(10);
  }

  return currentReading;
}

