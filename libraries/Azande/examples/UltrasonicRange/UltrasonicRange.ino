/*  Ultrasonic Range
 *  This example demonstrates how to use the Azande Library together with a Ultrasonic Range Detector.
 *  
 *  Example Specification:
 *  https://zeijlonsystems.se/products/azande/specification/ArduinoExampleUltrasonicRange.html
 *  
 *  The following Feature Types are used.
 *  - Command Bool Feature: With the cmdSetUnit the user can selct unit cm/mm to use.
 *  - Event Int32 Feature: The events eventDistanceCm and eventDistanceMm present the measured distance.
 *  - Command Int32 Feature: With the commands cmdSetAlarmCm and cmdSetAlarmMm the user can set limit for alarm.
 *  - Event Bool Feature: The event eventAlarm is sent every time a new measurment is done. True when distance <= limit.
 *  
 *  The circuit: 
 *  Ultrasonic Range Detector     Arduino Board
 *  Pin                           Pin       Type
 *  Vcc                           5V        Supply
 *  Trig                          Pin 4     Output
 *  Echo                          Pin 2     Input (if a 3.3V Arduino, use voltage divider)
 *  Gnd                           GND       Supply
 *  
 *  
 *  Install Azande Studio: https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */
 
#include <Azande.h>

// Azande Common Feature Macros
define_bool_command(cmdSetUnit, "Set Unit", SetUnit, 0, "cm", "mm");
define_bool_event(eventAlarm, "Too-Close Alarm", 10, "Activated", "Not Activated"); 

// Azande Feature Macros When unit is cm
define_int_event(eventDistanceCm, "Distance", 6,"cm",,,);
define_int_command(cmdSetAlarmCm, "Set Too-Close Alarm", SetAlarmCm, 10, "cm", 0, 150);

// Azande Feature Macros When unit is mm
define_int_event(eventDistanceMm, "Distance", 6,"mm",,,);
define_int_command(cmdSetAlarmMm, "Set Too-Close Alarm", SetAlarmMm, 10, "mm", 0, 1500);

// Configure connection to Ultrasonic Range Detector
int inputPin=2;   // ECHO pin 
int outputPin=4;  // TRIG pin 

// Variables
Azande azande(Serial);    // The Azande object variable. Using 'Serial' as 'Stream'
int alarmMillimeterLimit; // Alarm limit when distance is less or equal to this value the alarm is activated.
bool isUnitCm;            // When true the unit is "cm", when false "mm".

void setup() 
{ 
  // Setup Variables.
  alarmMillimeterLimit = 0;
  isUnitCm = true;
  
  // Setup pins to Ultrasonic Range Detector.
  pinMode(inputPin, INPUT); 
  pinMode(outputPin, OUTPUT); 

  Serial.begin(19200);

  azande.begin();
  // Add features to be visible on Azande Studio.
  azande.add(cmdSetUnit);
  azande.add(eventDistanceCm);
  azande.add(cmdSetAlarmCm);
  azande.add(eventAlarm);
}
 
void loop() 
{ 
  int millimeters;
  millimeters = GetMillimeterDistance(); // Execute one measurement

  if (isUnitCm)
  {
    azande.send(eventDistanceCm, (millimeters / 10));
  }
  else
  {
    azande.send(eventDistanceMm, millimeters);
  }
  azande.send(eventAlarm, (millimeters <= alarmMillimeterLimit));
  
  delay(100); 
  azande.readStream(); // Let Azande handle the new data.
} 

// Execute one measurement
int GetMillimeterDistance(void)
{
  int distance;
  
  digitalWrite(outputPin, HIGH); //Trigger ultrasonic detection 
  delayMicroseconds(10); 
  digitalWrite(outputPin, LOW); 
  distance = pulseIn(inputPin, HIGH); //Read ultrasonic reflection
  distance *= 10; // cm -> mm
  distance = distance/58; //Calculate distance 
  
  return distance;
}

// This function is called when receiving the command "Set Too-Close Alarm"(cmdSetAlarmCm), from Azande Studio.
void SetAlarmCm(long value)
{
  alarmMillimeterLimit = value * 10;
}

// This function is called when receiving the command "Set Too-Close Alarm"(cmdSetAlarmMm), from Azande Studio.
void SetAlarmMm(long value)
{
  alarmMillimeterLimit = value;
}

// This function is called when receiving the command "Set Unit"(cmdSetUnit), from Azande Studio.
void SetUnit(bool value)
{
  isUnitCm = value;

  // Change collection of available features and notify Azande Studio
  azande.clear(); // Clear collection of features.
  azande.add(cmdSetUnit);
  azande.add(eventAlarm);

  if (isUnitCm)
  {
    azande.add(eventDistanceCm);
    azande.add(cmdSetAlarmCm);
  }
  else
  {
    azande.add(eventDistanceMm);
    azande.add(cmdSetAlarmMm);
  }
  azande.notify(); // Notify Azande Studio that the feature collection has changed.
}

