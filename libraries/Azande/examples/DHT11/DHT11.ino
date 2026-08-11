/*  DHT11
 *  This is a simple example demonstrating how to use the Azande library together with a DHT11 sensor.
 *  The DHT11 measures Relative Humidity and Temperature.
 *  
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleDHT11.html
 *  
 *  Dependencies
 *  This example requires that you have the SimpleDHT library installed.
 *  Download SimpleDHT Library from https://github.com/winlinvip/SimpleDHT
 *  
 *  Features
 *  The following Feature Types are used:
 *  Event Int32 Feature: The events "Humidity" and "Temperature" sends the measurements to Azande Studio.
 *  Event Text Feature: The event "Sensor State" is sent, to Azande Studio, after every measurement.
 *  
 *  The circuit: 
 *  DHT11   Arduino Board
 *  Pin     Pin             Type
 *  S       Digital Pin 2   Input
 *  Vcc     5v or 3V        Supply
 *  Gnd     GND             Supply
 *  
 *  Install Azande Studio: https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */

#include <SimpleDHT.h>
#include <Azande.h>

// Azande Feature Macros
define_text_event(eventSensorState, "Sensor State", , 64);
define_int_event(eventHumidity, "Humidity", 8, "%", , , );
define_int_event(eventTemperature, "Temperature", 10, "C", , , );

// Variables and Objects
Azande azande;                  // The Azande object. Default using 'Serial' as 'Stream'
SimpleDHT11 dht11;              // The DHT11 object
unsigned char loopCounter;      // Counts how many loops executed since last read
unsigned char readErrorCounter; // Counts how many time read have failed

// DHT11 Connection Configuration
int pinDHT11 = 2;

void setup() 
{
  // Setup variables
  loopCounter = 0;
  readErrorCounter = 0;
  
  Serial.begin(19200);

  azande.begin();
  // Add features to be visible on Azande Studio.
  azande.add(eventSensorState);
  azande.add(eventHumidity);
  azande.add(eventTemperature);
}

void loop() 
{
  byte temperature = 0;
  byte humidity = 0;

  loopCounter++;
  if ((loopCounter > 12) || (readErrorCounter > 0)) // Execute every 12 x 100ms, or if Read Error do a retry.
  {
    loopCounter = 0;
    
    if (dht11.read(pinDHT11, &temperature, &humidity, NULL) != SimpleDHTErrSuccess) 
    {
      // Failed to read from sensor.
      readErrorCounter++;

      if (readErrorCounter > 5) // Failed to read 5 times.
      {
        readErrorCounter = 0;
        azande.send(eventSensorState, "Read Error");
      }
    }
    else // Read Success
    {
      readErrorCounter = 0;
      azande.send(eventSensorState, "OK");
      azande.send(eventHumidity, humidity);
      azande.send(eventTemperature, temperature);
    }
  }
  
  delay(100);
  azande.readStream(); // Let Azande handle the new data.
}

