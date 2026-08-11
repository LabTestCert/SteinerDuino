/*  Calculate
 *   This is a simple example demonstrating how to use the Azande library.
 *   
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleCalculate.html
 *   
 *   Features:
 *   3 feature are implemented:
 *   - "Calculate Command": A Command Enum Feature that consists of 5 different enums.
 *    When user click a button in Azande Studio, corresponding enum-value is sent to the target.
 *          - Button "set 0": (enum-value=0) Sets VALUE to 0
 *          - Button "add 1": (enum-value=1) Add 1 to VALUE.
 *          - Button "sub 1": (enum-value=2) Subtract 1 from VALUE.
 *          - Button "mul 2": (enum-value=3) Multiply VALUE with 2.
 *          - Button "div 2": (enum-value=4) Divide VALUE with 2.
 *  - "Set Value Command": An Command Int32 Feature sent to Azande Studio. The event holds the new value for VALUE.
 *  - "Value Event": An Event Int32 Feature holds the VALUE. The Event is sent to Azande Studio every time VALUE is changed. 
 *  
 *  The circuit: 
 *  No external hardware needed.
 *  
 *  Install Azande Studio: https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */
 
#include <Azande.h>

// Azande Feature Macros
define_enum_command(cmdCalc, "Calculate Command", Calc, ,   define_enum_item(0, "set 0")\
                                                            define_enum_item(1, "add 1")\
                                                            define_enum_item(2, "sub 1")\
                                                            define_enum_item(3, "mul 2")\
                                                            define_enum_item(4, "div 2"))
define_int_command(cmdSetValue, "Set Value Command", SetValue, , , , );
define_int_event(eventValue, "Value Event", , , , , );

// Variables
long value;               // The VALUE that is used for all calculations.
Azande azande(Serial);    // The Azande object variable. Using 'Serial' as 'Stream'
bool isStartMsgPending;   // Used when making sure to send, initial messages, only once after reset.

void setup() 
{
  // Setup Variables.
  value = 0;
  isStartMsgPending = true;
  
  Serial.begin(19200);

  azande.begin();  
  // Add features to be visible on Azande Studio.
  azande.add(cmdCalc);
  azande.add(cmdSetValue);
  azande.add(eventValue);
}

void loop() 
{
  // Send initial messages. Execute only once after reset.
  if(isStartMsgPending)
  {
    isStartMsgPending = false;
    azande.send(eventValue, value); // Send current VALUE
  }

  delay(10);
  azande.readStream(); // Let Azande handle the new data.
}

// This function is called when receiving the command "Set Value Command"(cmdSetValue), from Azande Studio.
void SetValue(long newValue)
{
  value = newValue; // Set new VALUE
  azande.send(eventValue, value); // Send current VALUE
}

// This function is called when receiving the command "Calculate Command"(cmdCalc), from Azande Studio.
void Calc(long enumValue)
{
  switch(enumValue)
  {
    case 0: // "Counter=0"
      value = 0;
    break;

    case 1: // "Counter++"
      value++;
    break;

    case 2: // "Counter--"
      value--;
    break;

    case 3: // "mul 2"
      value *= 2;
    break;

    case 4: // "div 2"
      value /= 2;
    break;

    default:
    break;
  }

  azande.send(eventValue, value); // Send current VALUE
}


