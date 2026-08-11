/* Gball
 * In this example we are demonstrating how to use the Azande Library together with the accelerometer ADXL345.
 * The ADXL senses acceleration force in 3 directions X, Y and Z.
 * 
 * Example Specification:
 * https://zeijlonsystems.se/products/azande/specification/ArduinoExampleGball.html
 * 
 * Following functions are demonstrated in this example:
 * - Azande
 * - Connect Wifi to Azande
 * - ADXL345, 3-Axis Digital Accelerometer
 * - Timer Interrupt with help from Timer5 library
 * 
 * Dependencies
 * Following libraries are used and need to be installed in your Arduino IDE:
 * - WiFi101                https://github.com/arduino-libraries/WiFi101
 * - SparkFun_ADXL345       https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
 * - Timer5                 https://github.com/michael71/Timer5
 * 
 * Hardware
 * In this example we have used:
 * - one MKR1000
 * - one ADXL345
 * - one Battery
 * 
 * The circuit: 
 *    ADXl345       Arduino Board     
 *    Pin           Pin             Type
 *    SCL           SCL             I²C Serial Clock
 *    SDA           SDA             I²C Serial Data
 *    Vcc           VCC             Supply
 *    Gnd           GND             Supply
 * 
 * 
 *  Install Azande Studio:
 *  https://zeijlonsystems.se/products/azande/specification/DownloadandInstallAzandeStudio.html
 */


#include <SparkFun_ADXL345.h>
#include <Azande.h>
#include <WiFi101.h>
#include <Timer5.h>
#include <math.h>
#include "arduino_secrets.h" // This file must be updated with your personal credentials.

// Default Calibration Parameters for your ADXL345 ==============
#define DefaultRange      16 // Range settings. Accepted values are 2,4,8 or 16 ==>> +/-2,4,8 or 16g range 
#define DefaultOffsetX    -21
#define DefaultOffsetZ    142
#define DefaultOffsetY    -4
#define DefaultGainMs2X   3.924567
#define DefaultGainMs2Y   3.822630
#define DefaultGainMs2Z   3.160041


// Config: Azande Studio Socket Server ===========================
// See Window/Settings/Socket Server in Azande Studio.
char myPcNetworkName[] = "ZEIJLON-SYSTEMS"; // Host Name
unsigned int azandeStudioServerPortNr = 41414;  // Port number

//#define USE_SERIAL_DEBUG // Comment this row if you don't want the debug texts on 'Serial'

// Config: Wi-Fi Connection ======================================
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

// Variables: Azande Studio Socket Server. =======================
WiFiClient azandeSocketClient;

// 'Main Menu' ============================================
// Azande Features
define_enum_command(cmdMainMenu, "Main Menu", MainMenu, 0,\
   define_enum_item(1, "Falling") \
   define_enum_item(2, "Spherical") \
   define_enum_item(3, "Setup") \
   define_enum_item(4, "Sleep") );
define_double_event(Xms2Event, "X", 6, "m/s²", -20, 20, "F2" );
define_double_event(Yms2Event, "Y", 6, "m/s²", -20, 20, "F2" );
define_double_event(Zms2Event, "Z", 6, "m/s²", -20, 20, "F2" );

// Main States
typedef enum
{
  State_MainMenu,
  State_FallTime,
  State_Spherical,
  State_SetupMenu,
  State_Calibrating,
  State_Sleeping
}StateType;
StateType state;

// 'Setup' =================================================
// 'Setup' Azande Features
define_enum_command(cmdSetupMenu, "Setup", SetupMenu, 0,\
   define_enum_item(1, "Calibration: START") \
   define_enum_item(2, "Calibration: END") \
   define_enum_item(3, "Use Default Calibration") \
   define_enum_item(4, "Use Saved Calibration") \
   define_enum_item(5, "BACK to Main Menu"));
define_text_event(textEvent, "Feedback", 0, 64 );

define_int_event(rawXevent, "Raw X", 6, , , ,  );
define_int_event(rawYevent, "Raw Y", 6, , , ,  );
define_int_event(rawZevent, "Raw Z", 6, , , ,  );

define_int_event(CalOffsetXevent, "X Offset", 8, , , ,  );
define_int_event(CalOffsetYevent, "Y Offset", 8, , , ,  );
define_int_event(CalOffsetZevent, "Z Offset", 8, , , ,  );

define_double_event(CalGainXevent, "X Gain", 9, , , , "F4"  );
define_double_event(CalGainYevent, "Y Gain", 9, , , , "F4"  );
define_double_event(CalGainZevent, "Z Gain", 9, , , , "F4"  );

define_enum_command(cmdSetRange, "Set Range", SetRange, 4,\
   define_enum_item(2,  "+/- 2G (20m/s²)") \
   define_enum_item(4,  "+/- 4G (40m/s²)") \
   define_enum_item(8,  "+/- 8G (80m/s²)") \
   define_enum_item(16, "+/- 16G (160m/s²)"));

// 'Setup' Text Messages
char * message;
char msgSetupMenu[] = "Setup";
char msgStartCalibration[] = "Calibration Started";
char msgSaveCalibration[] = "Calibration Saved";
char msgUseDefault[] = "Use Default";
char msgUseSaved[] = "Use Saved";
char msgRangeSet2g[] = "Range +/-2G";
char msgRangeSet4g[] = "Range +/-4G";
char msgRangeSet8g[] = "Range +/-8G";
char msgRangeSet16g[] = "Range +/-16G";

// 'Setup' Min and Max while calibrating
int rawMinX;
int rawMaxX;
int rawMinY;
int rawMaxY;
int rawMinZ;
int rawMaxZ;

// 'Setup' Saved values after calibration
int offsetXSaved;
int offsetYSaved;
int offsetZSaved;
double gainMs2XSaved;
double gainMs2YSaved;
double gainMs2ZSaved;


// 'Fall Time' ========================================================
// 'Fall Time' Azande Features
define_enum_command(FallMenuCmd, "Fall Time", FallMenu, 0,\
   define_enum_item(1, "BACK to Main Menu"));
define_int_event(FallTimeEvent, "Fall Time", 4, "ms", , ,  );
define_double_event(FallDistanceEvent, "Fall Distance", 5, "cm", , , "F1" );

// 'Fall Time' Config
#define FALL_UPPER 2.5 // When acceleration is more than FALL_UPPER, the fall have ended.
#define FALL_LOWER 1.5 // When acceleration is less than FALL_LOWER, the fall have started.

// 'Fall Time' Variables
volatile long interruptCounter; // incremented in timer interrupt
volatile long latestFallTime = 0;
volatile double latestFallDistance = 0;
bool isFalling = false;

// 'Spherical' ========================================================
// 'Spherical' Azande Features
define_enum_command(SphericalMenuCmd, "Spherical", SphericalMenu, 0,\
   define_enum_item(1, "BACK to Main Menu"));
define_double_event(PhiEvent, "PHI angle", 14, "°", 0, 180, "F1" );
define_double_event(ThetaEvent, "THETA angle", 14, "°", -180, 180, "F1" );
define_double_event(RadialEvent, "Radial Acceleration", 14, "m/s²", 0, 200, "F1" );

// 'Spherical' Variables
double SphericalPhi = 0;
double SphericalTeta = 0;
double SphericalR = 0;

Azande azande(azandeSocketClient);  // Create Azande object with "Socket Client" as "Stream".
ADXL345 adxl = ADXL345(); // USE FOR I2C COMMUNICATION

// Gravity Constants ====================================
#define OneGms2         9.81
#define OneHalfGms2     9.81 / 2.0

// Conversion Variables =================================
int offsetX = DefaultOffsetX;
int offsetY = DefaultOffsetY;
int offsetZ = DefaultOffsetZ;
double gainMs2X = DefaultGainMs2X;
double gainMs2Y = DefaultGainMs2Y;
double gainMs2Z = DefaultGainMs2Z;

// Read Values ==========================================
double ms2X = 0; // raw X value converted to m/s²
double ms2Y = 0; // raw Y value converted to m/s²
double ms2Z = 0; // raw Z value converted to m/s²
int rawX;
int rawY;
int rawZ;


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

  azande.begin();
  gotoMainMenu();
  
  adxl.powerOn();                     // Power on the ADXL345
  adxl.setRangeSetting(DefaultRange);           

  MyTimer5.begin(200);  // 200=for toggle every 5msec
  MyTimer5.attachInterrupt(Timer5_IRQ);
  MyTimer5.start();
}

void loop()
{
  if (state == State_Sleeping)
  {
    delay(500);
    ReadFromSensor();
    SphericalR = sqrt((ms2X * ms2X) + (ms2Y * ms2Y) + (ms2Z * ms2Z));
    if (SphericalR < FALL_UPPER) // If we are Free Falling wakeup
    {
      tryConnectToWifiNetwork();        // Blocks until connection succeed
      tryConnectToAzandeStudioServer(); // Blocks until connection succeed
      gotoMainMenu();
    }
  }
  else if (!azandeSocketClient.connected())
  {
    // if the server is disconnected, try to re-connect.
    debugPrintln("Lost connection with server");
    tryConnectToAzandeStudioServer();
    gotoMainMenu();
  }
  else
  {
    switch (state)
    {
      case State_Spherical:
        ReadFromSensor();
        SphericalR = sqrt((ms2X * ms2X) + (ms2Y * ms2Y) + (ms2Z * ms2Z));
        if (SphericalR != 0.0) // Avoid division with zero
        {
          SphericalPhi = (acos(ms2Z / SphericalR) * 180) / PI;
        }
        else
        {
          SphericalPhi = 0;
        }
        SphericalTeta = (atan2(ms2Y, ms2X) * 180) / PI ;
        azande.send(PhiEvent, SphericalPhi);
        azande.send(ThetaEvent, SphericalTeta);
        azande.send(RadialEvent, SphericalR);
        break;
        
      case State_SetupMenu:
        ReadFromSensor();
        azande.send(rawXevent, rawX);
        azande.send(rawYevent, rawY);
        azande.send(rawZevent, rawZ);
        azande.send(textEvent, message);
        break;

      case State_Calibrating:
        ReadFromSensor();
        azande.send(rawXevent, rawX);
        azande.send(rawYevent, rawY);
        azande.send(rawZevent, rawZ);
        azande.send(textEvent, message);

        if (rawX < rawMinX) rawMinX = rawX;
        if (rawY < rawMinY) rawMinY = rawY;
        if (rawZ < rawMinZ) rawMinZ = rawZ;
        if (rawX > rawMaxX) rawMaxX = rawX;
        if (rawY > rawMaxY) rawMaxY = rawY;
        if (rawZ > rawMaxZ) rawMaxZ = rawZ;
        break;

      case State_FallTime:
        azande.send(RadialEvent, SphericalR);
        azande.send(FallTimeEvent, latestFallTime);
        azande.send(FallDistanceEvent, latestFallDistance);
        break;

      default:
      case State_MainMenu:
        ReadFromSensor();
        azande.send(Xms2Event, ms2X);
        azande.send(Yms2Event, ms2Y);
        azande.send(Zms2Event, ms2Z);
    }

    azande.readStream();
  }

  delay(5);
}


void Timer5_IRQ(void) 
{
  if (state == State_FallTime)
  {
    ReadFromSensor();
    SphericalR = sqrt((ms2X * ms2X) + (ms2Y * ms2Y) + (ms2Z * ms2Z));
    
    if (isFalling)
    {
      interruptCounter++;
      if (SphericalR > FALL_UPPER)
      {
        isFalling = false;
        latestFallTime = interruptCounter * 5;

        // Calculate the distance of the fall. Note that we assume that the ball have velocity=0 when fall starts.
        latestFallDistance = 100.0 * (OneGms2 * ((double)latestFallTime / 1000.0) * ((double)latestFallTime / 1000.0)) / 2.0; 
      }
    }
    else
    {
      if (SphericalR < FALL_LOWER)
      {
        isFalling = true;
        interruptCounter = 0;
      }
    }
  }
}

void ReadFromSensor(void)
{
    adxl.readAccel(&rawX, &rawY, &rawZ);         // Read the accelerometer values and store them in variables declared above x,y,z
  
    // Calculate m/s² with help of calibration parameters.
    ms2X = (rawX - offsetX)/gainMs2X;         
    ms2Y = (rawY - offsetY)/gainMs2Y;
    ms2Z = (rawZ - offsetZ)/gainMs2Z;
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


void gotoMainMenu(void)
{
  azande.clear();
  azande.add(Xms2Event);
  azande.add(Yms2Event);
  azande.add(Zms2Event);
  azande.add(cmdMainMenu);
  azande.notify();
  state = State_MainMenu;
}

void gotoFallTime(void)
{
  azande.clear();
  azande.add(FallTimeEvent);
  azande.add(FallDistanceEvent);
  azande.add(FallMenuCmd);
  azande.notify();
  state = State_FallTime;
}

void gotoSphericalMenu(void)
{
  azande.clear();
  azande.add(SphericalMenuCmd);
  azande.add(PhiEvent);
  azande.add(ThetaEvent);
  azande.add(RadialEvent);
  azande.notify();
  state = State_Spherical;
}

void gotoSetupMenu(void)
{
  message = msgSetupMenu;
  azande.clear();
  azande.add(textEvent);
  azande.add(cmdSetupMenu);
  azande.add(cmdSetRange);
  azande.add(rawXevent);
  azande.add(rawYevent);
  azande.add(rawZevent);
  azande.add(CalOffsetXevent);
  azande.add(CalOffsetYevent);
  azande.add(CalOffsetZevent);
  azande.add(CalGainXevent);
  azande.add(CalGainYevent);
  azande.add(CalGainZevent);
  azande.notify();
  state = State_SetupMenu;
}


void MainMenu(long item)
{
  switch (item)
  {
    case 1: // "1. Fall Time"
      gotoFallTime();
      break;
    
    case 2: // "2. Spherical"
      gotoSphericalMenu();
      break;
    
    case 3: // "3. Setup"
      gotoSetupMenu();
      break;
    
    case 4: // "4. Sleep"
    state = State_Sleeping;
    WiFi.end(); // Disconnect and turn off WiFi.
    delay(5000); // Wait 5s to make sure Azande Studio removed the connection.
    break;
  }
}

void SetupMenu(long item)
{
  switch (item)
  {
    case 1: // "1. Start Calibration"
      state = State_Calibrating;
      message = msgStartCalibration;
      rawMinX = rawX;
      rawMaxX = rawX;
      rawMinY = rawY;
      rawMaxY = rawY;
      rawMinZ = rawZ;
      rawMaxZ = rawZ;
      break;
    
    case 2: // "2. Save Calibration"
      state = State_SetupMenu;
      message = msgSaveCalibration;

      // Calculate and save new calibration parameters
      offsetXSaved = (rawMinX + rawMaxX) / 2;
      offsetYSaved = (rawMinY + rawMaxY) / 2;
      offsetZSaved = (rawMinZ + rawMaxZ) / 2;
      gainMs2XSaved = (double)(rawMaxX - rawMinX) / OneHalfGms2;
      gainMs2YSaved = (double)(rawMaxY - rawMinY) / OneHalfGms2;
      gainMs2ZSaved = (double)(rawMaxZ - rawMinZ) / OneHalfGms2;

      // Use new parameters
      offsetX = offsetXSaved;
      offsetY = offsetYSaved;
      offsetZ = offsetZSaved;
      gainMs2X = gainMs2XSaved;
      gainMs2Y = gainMs2YSaved;
      gainMs2Z = gainMs2ZSaved;

      azande.send(CalOffsetXevent, offsetX);
      azande.send(CalOffsetYevent, offsetY);
      azande.send(CalOffsetZevent, offsetZ);
      azande.send(CalGainXevent, gainMs2X);
      azande.send(CalGainYevent, gainMs2Y);
      azande.send(CalGainZevent, gainMs2Z);
      break;
    
    case 3: // "3. Use Default Calibration"
      message = msgUseDefault;
      adxl.setRangeSetting(DefaultRange);  
      offsetX = DefaultOffsetX;
      offsetY = DefaultOffsetY;
      offsetZ = DefaultOffsetZ;
      gainMs2X = DefaultGainMs2X;
      gainMs2Y = DefaultGainMs2Y;
      gainMs2Z = DefaultGainMs2Z;
      break;
    
    case 4: // "4. Use Saved Calibration"
      message = msgUseSaved;
      // Use new parameters
      offsetX = offsetXSaved;
      offsetY = offsetYSaved;
      offsetZ = offsetZSaved;
      gainMs2X = gainMs2XSaved;
      gainMs2Y = gainMs2YSaved;
      gainMs2Z = gainMs2ZSaved;
      break;
    
    case 5: // "5. BACK to Main Menup"
      gotoMainMenu();
      break;
  }
}

void SphericalMenu(long item)
{
  switch (item)
  {
    case 1: // "BACK to Main Menu"
      gotoMainMenu();
      break;
  }
}

void FallMenu(long item)
{
  switch (item)
  {
    case 1: // "BACK to Main Menu"
      gotoMainMenu();
      break;
  }
}


void SetRange(long range)
{
  switch (range)
  {
    case 2:
      adxl.setRangeSetting(2);  
      message = msgRangeSet2g;
      break;
      
    case 4:
      adxl.setRangeSetting(4);  
      message = msgRangeSet4g;
      break;
      
    case 8:
      adxl.setRangeSetting(8);  
      message = msgRangeSet8g;
      break;

    default:
    case 16:
      adxl.setRangeSetting(16);  
      message = msgRangeSet16g;
      break;
  }
}

