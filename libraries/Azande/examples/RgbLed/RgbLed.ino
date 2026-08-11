/*  RGB LED
 *  This example demonstrates how to use the Azande Library together with a RGB LED.
 *  
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleRGBLED.html 
 * 
 *  
 *  The LED has one "level-pin" for each color Red(R), Green(G), Blue(B).
 *  Each pin is supplied with a PWM with the function analogWrite(...).
 *  The PWM duty 0-100% is represented by a value ranging 0-255.
 *  
 *  In this example you can set each "color-level" individually from Azande Studio.
 *  You can also select one of the predefined colors: Black, White, Red, Green, Blue and Yellow.
 *  The the predefined colors are a combination of R, G and B values.
 *  
 *  The following Feature Types are used.
 *  - Command Enum Feature: With the command "Set Color" the user can select one of the predefined colors.
 *  For each color:
 *  - Command Int32 Feature: The command "color" sets the "color-level".
 *  - Event Int32 Feature: The event "color" shows current "color-level". 
 *    By setting range (0-255) the Feature Panel will be a Gauge in Azande Studio.
 *  - Command Enum Feature: The command "Adjust xxx" is a set of adjustments user can apply on the "color-level".
 *  
 *  The circuit: 
 *  LED     Arduino Board
 *  Pin     Pin             Type          Comment
 *  R       Digital Pin 9   Output PWM    Use pin that support analogWrite. On board pins with '~' symbol.
 *  G       Digital Pin 10  Output PWM    Use pin that support analogWrite. On board pins with '~' symbol.
 *  B       Digital Pin 11  Output PWM    Use pin that support analogWrite. On board pins with '~' symbol.
 *  -       GND             Supply
 *  
 *  Install Azande Studio: https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */

#include <Azande.h>

// Azande Feature Macros for Color Red
define_int_event(     eventRed,   "Red",                          10, , 0, 255, );
define_int_command(   cmdRed,     "Red",    SetRed,               10, , 0, 255);
define_enum_command(cmdAdjustRed, "Adjust Red", AdjustRed,        10, define_enum_item(0, "+1")\
                                                                      define_enum_item(1, "-1")\
                                                                      define_enum_item(2, "+10")\
                                                                      define_enum_item(3, "-10")\
                                                                      define_enum_item(4, "Off")\
                                                                      define_enum_item(5, "Max"))

// Azande Feature Macros for Color Red
define_int_event(     eventGreen, "Green",                        11, , 0, 255, );
define_int_command(   cmdGreen,   "Green",  SetGreen,             11, , 0, 255);
define_enum_command(cmdAdjustGreen, "Adjust Green", AdjustGreen,  11, define_enum_item(0, "+1")\
                                                                      define_enum_item(1, "-1")\
                                                                      define_enum_item(2, "+10")\
                                                                      define_enum_item(3, "-10")\
                                                                      define_enum_item(4, "Off")\
                                                                      define_enum_item(5, "Max"))

// Azande Feature Macros for Color Red
define_int_event(     eventBlue,  "Blue",                         12, , 0, 255, );
define_int_command(   cmdBlue,    "Blue",   SetBlue,              12, , 0, 255);
define_enum_command(cmdAdjustBlue, "Adjust Blue", AdjustBlue,     12, define_enum_item(0, "+1")\
                                                                      define_enum_item(1, "-1")\
                                                                      define_enum_item(2, "+10")\
                                                                      define_enum_item(3, "-10")\
                                                                      define_enum_item(4, "Off")\
                                                                      define_enum_item(5, "Max"))
                                                            
// Azande Feature Macro for predefined colors
define_enum_command(cmdColor, "Set Color", SetColor,              18, define_enum_item(0, "Black")\
                                                                      define_enum_item(1, "White")\
                                                                      define_enum_item(2, "Red")\
                                                                      define_enum_item(3, "Green")\
                                                                      define_enum_item(4, "Blue")\
                                                                      define_enum_item(5, "Yellow"))
                                                            
// Variables
Azande azande;    // The Azande object variable. 'Serial' is default used 'Stream'.

// LED Pin Configuration
int redPin = 9;   // Red LED,   connected to digital pin 9
int grnPin = 10;  // Green LED, connected to digital pin 10
int bluPin = 11;  // Blue LED,  connected to digital pin 11

// Predefined Colors
//                  R     G     B
int black[3]    = { 0,    0,    0   };
int white[3]    = { 100,  100,  100 };
int red[3]      = { 100,  0,    0   };
int green[3]    = { 0,    100,  0   };
int blue[3]     = { 0,    0,    100 };
int yellow[3]   = { 40,   95,   0   };
int dimWhite[3] = { 30,   30,   30  };

// Set initial color
int redVal;
int grnVal;
int bluVal;

// Set up the LED outputs
void setup()
{
  // Setup variables
  redVal = black[0];
  grnVal = black[1]; 
  bluVal = black[2];

  // sets the pins to LED as output
  pinMode(redPin, OUTPUT);   
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 

  Serial.begin(19200);
  
  azande.begin();
  // Add features to be visible on Azande Studio.
  azande.add(eventRed);
  azande.add(cmdRed);
  azande.add(eventGreen);
  azande.add(cmdGreen);
  azande.add(eventBlue);
  azande.add(cmdBlue);
  azande.add(cmdColor);
  azande.add(cmdAdjustRed);
  azande.add(cmdAdjustGreen);
  azande.add(cmdAdjustBlue);
}

void loop()
{
  // Write current values to LED pins
  analogWrite(redPin, redVal);   
  analogWrite(grnPin, grnVal);      
  analogWrite(bluPin, bluVal); 

  delay(50);
  azande.readStream(); // Let Azande handle the new data.
}

// This function is called when receiving the command "Red" (cmdRed), from Azande Studio
void SetRed(long value)
{
  // make sure within range 0-255
  if (value < 0)
  {
    redVal = 0;
  }
  else if (value > 255)
  {
    redVal = 255;
  }
  else
  {
    redVal = value;
  }
  azande.send(eventRed, redVal);
}

// This function is called when receiving the command "Green" (cmdGreen), from Azande Studio
void SetGreen(long value)
{
  // make sure within range 0-255
  if (value < 0)
  {
    grnVal = 0;
  }
  else if (value > 255)
  {
    grnVal = 255;
  }
  else
  {
    grnVal = value;
  }
  azande.send(eventGreen, grnVal);
}

// This function is called when receiving the command "Blue" (cmdBlue), from Azande Studio
void SetBlue(long value)
{
  // make sure within range 0-255
  if (value < 0)
  {
    bluVal = 0;
  }
  else if (value > 255)
  {
    bluVal = 255;
  }
  else
  {
    bluVal = value;
  }
  azande.send(eventBlue, bluVal);
}

// This function is called when receiving the command "Set Color" (cmdColor), from Azande Studio 
void SetColor(long enumValue)
{
  switch (enumValue)
  {
    case 0: // Black
      SetRed(black[0]);
      SetGreen(black[1]);
      SetBlue(black[2]);
      break;

    case 1: // White
      SetRed(white[0]);
      SetGreen(white[1]);
      SetBlue(white[2]);
      break;

    case 2: // Red
      SetRed(red[0]);
      SetGreen(red[1]);
      SetBlue(red[2]);
      break;

    case 3: // Green
      SetRed(green[0]);
      SetGreen(green[1]);
      SetBlue(green[2]);
      break;

    case 4: // Blue
      SetRed(blue[0]);
      SetGreen(blue[1]);
      SetBlue(blue[2]);
      break;

    case 5: // Yellow
      SetRed(yellow[0]);
      SetGreen(yellow[1]);
      SetBlue(yellow[2]);
      break;

    default:
      break;
  }
}

// This function is called when receiving the command "Adjust Red" (cmdAdjustRed), from Azande Studio
void AdjustRed(long enumValue)
{
  switch (enumValue)
  {
    case 0: // +1
      SetRed(redVal + 1);
      break;

    case 1: // -1
      SetRed(redVal - 1);
      break;

    case 2: // +10
      SetRed(redVal + 10);
      break;

    case 3: // -10
      SetRed(redVal - 10);
      break;

    case 4: // Off
      SetRed(0);
      break;


    case 5: // Max
      SetRed(255);
      break;

    default:
      break;
  }
}

// This function is called when receiving the command "Adjust Green" (cmdAdjustGreen), from Azande Studio
void AdjustGreen(long enumValue)
{
  switch (enumValue)
  {
    case 0: // +1
      SetGreen(grnVal + 1);
      break;

    case 1: // -1
      SetGreen(grnVal - 1);
      break;

    case 2: // +10
      SetGreen(grnVal + 10);
      break;

    case 3: // -10
      SetGreen(grnVal - 10);
      break;

    case 4: // Off
      SetGreen(0);
      break;


    case 5: // Max
      SetGreen(255);
      break;

    default:
      break;
  }
}

// This function is called when receiving the command "Adjust Blue" (cmdAdjustBlue), from Azande Studio
void AdjustBlue(long enumValue)
{
  switch (enumValue)
  {
    case 0: // +1
      SetBlue(bluVal + 1);
      break;

    case 1: // -1
      SetBlue(bluVal - 1);
      break;

    case 2: // +10
      SetBlue(bluVal + 10);
      break;

    case 3: // -10
      SetBlue(bluVal - 10);
      break;

    case 4: // Off
      SetBlue(0);
      break;


    case 5: // Max
      SetBlue(255);
      break;

    default:
      break;
  }
}


