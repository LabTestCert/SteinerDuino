/*  WiFi Signal Strength
 *   This is a simple example demonstrating how to use 
 *   the Azande Library together with a WiFi Shield or a MKR1000.
 *   This example is designed for Arduino systems using 
 *   the WINC1500 Wi-Fi Module.
 *   
 *   Example Specification:
 *   https://zeijlonsystems.se/products/azande/specification/ArduinoExampleWiFiSignalStrength.html 
 *   
 *   This example use the WiFi101 Library.
 *   This WiFi101 Library allows you to use the Arduino WiFi Shield 101 and the MKR1000 board. 
 *   More details at: https://www.arduino.cc/en/Reference/WiFi101
 *   
 *   Features:
 *   One feature is implemented:
 *   "Wi-Fi Signal Strength": An 'Event Int32 Feature' sent to Azande Studio. 
 *   The event holds the Wi-Fi Signal Strength fetched from WiFi101 library.
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

// Azande ========================================================
// Azande Feature Macros
define_target_info(targetInfo, "WiFi Signal Strength", "1.0", "2018-02-25", "Testing");
define_int_event(eventRSSI, "Wi-Fi Signal Strength", , "dbm", -150, 0, );

// Variables
Azande azande(azandeSocketClient);  // Create Azande object with "Socket Client" as "Stream".
int sendTimeoutCounter = 0;

void setup() 
{
  waitForSerialIfAny();

  if (WiFi.status() == WL_NO_SHIELD) 
  {
    debugPrintln("WiFi shield not present");
    while (true); // don't continue:
  }

  tryConnectToWifiNetwork();        // Blocks until connection succeed
  tryConnectToAzandeStudioServer(); // Blocks until connection succeed

  azande.begin(targetInfo);
  azande.add(eventRSSI);
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
    sendTimeoutCounter++;
    if (sendTimeoutCounter > 10)
    {
      sendTimeoutCounter = 0;
      azande.send(eventRSSI, WiFi.RSSI());
    }
    
    azande.readStream(); // Let Azande handle the new data (if any).
  }

  delay(100);
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
  delay(100);
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

