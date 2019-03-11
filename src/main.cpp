#include "Arduino.h"
#include "HX711.h"
#include <LiquidCrystal_I2C.h>

#define DOUT  3
#define CLK  2
#define FULL_CUP              10          // A full cup is about 10 ounces
#define SPLATTER_POINT        73          // At this point you'll get splatter, and empty carafe is actually 69.25oz
#define EMPTY_CARAFE          69          // An empty carafe is about 69.25oz
#define FULL_CARAFE           150         // A full carafe is about 150oz
#define EMPTY_SCALE_THRESHOLD 10          // Assume the scale is empty at 10 ounces
#define MINUTES_IN_HOUR     60            // 60 Minutes in an hour
#define MILLIS_IN_MINUTE    60000         // 60000 ms in a minute
#define EVENT_PIN           4             // The pin that will send out an event notification
#define SERIAL_DEBUG        0             // Set to 1 to have statements printed to the monitor

// ************ Object and variable declarations **************
HX711 scale(DOUT, CLK);
LiquidCrystal_I2C lcd(0x27,20,4);
long lastBrewTime = 0;
double latestRecordedWeight = 0.0;
float calibration_factor = -580;  // Adjust the weight calibration, tested with 250g and 500g weights

// ********************** Function declarations ****************
void initScale();
double getCupsRemaining(double reading);
bool scaleIsEmpty(double reading);
String getAgeString();
void handleEmptyScale();
void handleCarafeEmpty();
void handleCarafeNotEmpty(double reading);
double getScaleReading();
bool carafeIsEmpty(double reading);
void handleFreshBrew();

void setup() {
#if SERIAL_DEBUG > 0
  Serial.begin(9600);
#endif
  // Setup LCD display
  lcd.init();
  lcd.backlight();

  // Enable the event pin as output
  pinMode(EVENT_PIN, OUTPUT);
  digitalWrite(EVENT_PIN, LOW);

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
  if (scaleIsEmpty(reading)) {
    handleEmptyScale();
  } else if (carafeIsEmpty(reading)) {
    handleCarafeEmpty();
  } else if (!carafeIsEmpty(reading)) {
    handleCarafeNotEmpty(reading);
  }

  delay(4000);
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
#if SERIAL_DEBUG > 0
    Serial.println("Tare");
#endif
    // TODO: This may be causing problems more than it is solving them
    scale.tare();
    delay(2000);
  }

  // Now that the scale isn't empty, determine if more coffee was added
  if (latestRecordedWeight > previousWeight + FULL_CUP) {
    // If the new wight is at least one more cup of coffee more than the old, assume a new brew
    handleFreshBrew();
  }
}

void handleFreshBrew() {
  // Update the last brew time
  lastBrewTime = millis();

  // Notify the WiFi module driving the event pin HIGH for half a second
  digitalWrite(EVENT_PIN, HIGH);
  delay(500);
  digitalWrite(EVENT_PIN, LOW);
}

void initScale() {
  // Ready
  lcd.clear();
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("Add container");

  // Wait for scale to have weight added to it.
  while (getScaleReading() < 1.0) {
    delay(2000);
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

bool carafeIsEmpty(double reading) {
  // If the reading is between empty and where the coffee splatters...
  return reading >= EMPTY_CARAFE && reading <= SPLATTER_POINT;
}

double getCupsRemaining(double reading) {
  return (reading - SPLATTER_POINT) / FULL_CUP;
}

double getScaleReading() {
  // Keep taking readings one second apart until they are within 1 ounce of each other (indicating stability)
  // This should ensure a reading isn't taken while weight is added or removed from the scale
  double firstReading = 0;
  double secondReading = 10;
  while (abs(firstReading - secondReading) >= 1) {
    firstReading = scale.get_units(10);
    delay(1000);
    secondReading = scale.get_units(10);
  }

#if SERIAL_DEBUG > 0
  Serial.print("Reading is ");
  Serial.println(secondReading);
#endif

  // If the scale isn't empty, record the last weight
  if (!scaleIsEmpty(secondReading)) {
    latestRecordedWeight = secondReading;
  }

  return secondReading;
}

