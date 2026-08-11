/*  Software Reset
 *  This example demonstrates how to use the Azande library.
 *  
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleSoftwareReset.html
 *  
 *  3 feature are implemented.
 *  - "Software Reset": A Void-Command that starts a 3s delayed software reset. 
 *    The command is executed when user click button in Azande Studio.
 *  - "Uptime": An Int32-Event sent to Azande Studio every second. Holds system Uptime.
 *  - "Board State": A Text-Event sent to notify user about the current state of the board.
 *  
 *  The circuit: No external hardware needed.
 *  
 *  Install Azande Studio: https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */
 
#include <Azande.h>

// Azande Feature Macros
define_void_command(cmdReset,"Software Reset", myReset,);
define_int_event(eventUptime, "Uptime", , "s", , , );
define_text_event(eventBoardState, "Board State", , 64);

// Variables
long uptime;              // This variable holds how long time(10ms ticks) the system have bin running since reset.
int secondsToReset;       // Keeps track of how many seconds remain until a sw reset is performed. Inactive when 0.
bool isBoardStatePending; // Used when making sure to send, board initial state, only once after reset.
Azande azande(Serial);    // The Azande object variable. Using 'Serial' as 'Stream'

void(* executeReset) (void) = 0; // Setup a function pointer to address 0. A call will genrate software reset

void setup() 
{
  // Setup Variables.
  uptime = 0;
  secondsToReset = 0;
  isBoardStatePending = true;
  
  Serial.begin(19200);

  azande.begin();
  // Add features to be visible on Azande Studio.
  azande.add(cmdReset);
  azande.add(eventUptime);
  azande.add(eventBoardState);
}


void loop() 
{
  // Send board initial state. Execute only once after reset.
  if (isBoardStatePending) 
  {
    isBoardStatePending = false;
    azande.send(eventUptime, uptime/100);
    azande.send(eventBoardState, "Board Restarted");
  }
  
  delay(10); // 10ms delay make the loop() to execute ~100 times every second.
  
  uptime++;
  if ((uptime % 100) == 0) // every 1s
  {
    azande.send(eventUptime, (uptime/100)); // Send Uptime value every second
    
    if (secondsToReset > 0)
    {
      secondsToReset--;
      if (secondsToReset == 0)
      {
        executeReset(); // This call will generate a software reset 
      }
    }
    else if ((uptime % 1000) == 0) // every 10s
    {
      azande.send(eventBoardState, "Still Running"); // Send 10s after reset and then every 10s.
    }
  }

  azande.readStream(); // Let Azande handle the new data.
}

// This function is called when receiving the command "Software Reset"(cmdReset), from Azande Studio.
void myReset(void)
{
  azande.send(eventBoardState, "Reset Pending");
  secondsToReset = 4; // By setting this variable the "Delayed Reset" is activated. System will reseted winthin 3-4s.
}


