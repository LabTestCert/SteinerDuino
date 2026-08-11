/*  Calculate WiFi
 *   This is a simple example demonstrating how to use 
 *   the Azande Library together with a WiFi Shield or a MKR1000.
 *   This example is designed for Arduino systems using 
 *   the WINC1500 Wi-Fi Module.
 *   
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleCalculateWiFi.html  
 *   
 *   This example use the WiFi101 Library
 *   More details at: https://www.arduino.cc/en/Reference/WiFi101
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
#include <WiFi101.h>
#include "arduino_secrets.h" // This file must be updated with your personal credentials.

#define USE_SERIAL_DEBUG // Comment this row if you don't want the debug texts on 'Serial'

// Config: Wi-Fi Connection ======================================
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

// Config: Azande Studio Socket Server ===========================
// See Window/Settings/Socket Server in Azande Studio. ===========
char myPcNetworkName[] = "Zeijlon-Systems";
unsigned int azandeStudioServerPortNr = 41414;  // Port number

// Variables: Azande Studio Socket Server. =======================
WiFiClient azandeSocketClient;
int azandeServerHeartBeatCounter = 0;

// Azande ========================================================
// Azande Feature Macros
define_target_info(targetInfo, "Calculate WiFi", "1.0", "2018-02-24", "Testing");
define_enum_command(cmdCalc, "Calculate Command", Calc, ,   define_enum_item(0, "set 0")\
                                                            define_enum_item(1, "add 1")\
                                                            define_enum_item(2, "sub 1")\
                                                            define_enum_item(3, "mul 2")\
                                                            define_enum_item(4, "div 2"))
define_int_command(cmdSetValue, "Set Value Command", SetValue, , , , );
define_int_event(eventValue, "Value Event", , , , , );

// Variables
long value;                       // The VALUE that is used for all calculations.
Azande azande(azandeSocketClient);// Create Azande object with "Socket Client" as "Stream".

void setup() 
{
  value = 0;

  waitForSerialIfAny();

  if (WiFi.status() == WL_NO_SHIELD) 
  {
    debugPrintln("WiFi shield not present");
    while (true); // don't continue:
  }

  tryConnectToWifiNetwork();        // Blocks until connection succeed
  tryConnectToAzandeStudioServer(); // Blocks until connection succeed

  azande.begin(targetInfo);
  azande.add(cmdCalc);
  azande.add(cmdSetValue);
  azande.add(eventValue);
  azande.notify();
}

void loop() 
{
  if (!azandeSocketClient.connected()) 
  {
    // if the server is disconnected, try to re-connect.
    debugPrintln("Lost connection with server");
    tryConnectToAzandeStudioServer();
    azande.notify();
  }
  else
  {
  /*  A Client connected to 'Azande Studio Socket Server' 
   *  must send a message atleast once every ~5s.
   *  
   *  Because we do not have any continuous sending 
   *  we have to implement a heartbeat. Sending the dummy
   *  "x" app. once every 1.5s
   */
    azandeServerHeartBeatCounter++;
    if (azandeServerHeartBeatCounter > 150)
    {
      azandeServerHeartBeatCounter = 0;
      azandeSocketClient.print("x"); // Send heartbeat
      debugPrintln("Heartbeat Sent.");
    }
    
    azande.readStream(); // Let Azande handle the new data (if any).
  }

  delay(10);
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



void tryConnectToWifiNetwork()
{
  int status = WL_IDLE_STATUS;

  debugPrint("Attempting to connect to SSID: ");
  debugPrintln(ssid);
  status = WiFi.begin(ssid, pass);
  while (status != WL_CONNECTED) 
  {
    delay(10000); // Wait 10s
    debugPrint("Attempting to connect to SSID: ");
    debugPrintln(ssid);
    status = WiFi.begin(ssid, pass);
  }
  debugPrintln("Connected to wifi");
  printWiFiStatus();
}

void tryConnectToAzandeStudioServer()
{
  debugPrintln("Starting connection to Azande Studio Server...");
  azandeSocketClient.connect(myPcNetworkName, azandeStudioServerPortNr);
  while (!azandeSocketClient.connected())
  {
    debugPrintln("Failed to connect to Azande Studio Server.");
    delay(5000); // wait 5s
    debugPrintln("Retry connection to Azande Studio Server...");
    azandeSocketClient.connect(myPcNetworkName, azandeStudioServerPortNr);
  }

  debugPrint("Connected to server on ");
  debugPrintln(myPcNetworkName);
}

void waitForSerialIfAny()
{
  #ifdef USE_SERIAL_DEBUG
    // Use "Serial" for debug printing
    Serial.begin(9600);
    while (!Serial); // wait for serial port to connect. Needed for native USB port only
  #endif
}

void printWiFiStatus() 
{
  #ifdef USE_SERIAL_DEBUG
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
  
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("My IP Address: ");
    Serial.println(ip);
  
    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
  #endif
}

void debugPrint(char* txt)
{
  #ifdef USE_SERIAL_DEBUG
    Serial.print(txt);
  #endif
}

void debugPrintln(char* txt)
{
  #ifdef USE_SERIAL_DEBUG
    Serial.println(txt);
  #endif
}

