/* Steiner Tunnel Logger
 * Authored: Dominik P. 2020
 * Modified: Kaden B. 2021-01-20
 * Modified: Kaden B. 2025-05-08
 * Modified: Kaden B. 2026-04-01
 *  After moving the tunnel into a new building after 1 year being taken apart, there were issues getting the
 *  control system working. The PCB doesn't connect all its ground planes together; relying on the MCU to do that,
 *  but the new MCU we got to replace the broken one wasn't doing that, so I had to solder in a wire. Also,
 *  all the timer subroutines should return TRUE. They weren't all doing that before.
 * 
 * Description:
 *  Log sensoor values needed for E84, S102, and S102.2 tests at the Steiner tunnel.
 *  
 * Analog Sensors: (Read through two external 12-bit ADCs reading from 0V to ~4.9V)
 *  Thermocouple 1      The exposed thermocouple at the end of the tunnel
 *  Thermocouple 2      The burried thermocouple at the end of the tunnel
 *  Thermocouple 3      The burried thermocouple part way down the tunnel
 *  Linear Transducer   A potentiometer with a mechanical connection to a linear rail that extends down the length of the tunnel
 *  Lux meter           A photometer monitoring the smoke levels produced inside the tunnel
 *  A                   Available channel
 *  B                   Available channel
 *  C                   Available channel
 *  Pressure            The draft pressure behind the window
 * 
 * Notes:
 */

#include <SPI.h>                // I think this is for the pressure sensor?
#include <Wire.h>               // I2C serial
#include <arduino-timer.h>      // Timers
#include <Pushbutton.h>         // Pushbutton debounce
#include "RTClib.h"             // Real-time clock
#include "FS.h"                 // File system
#include "SD.h"                 // SD Card
#include <Adafruit_GFX.h>       // OLED graphics
#include <Adafruit_SSD1306.h>   // OLED driver
#include <ADS1115_WE.h>         // External ADC
#include "SDP6x.h"              // Pressure sensor
#include <PID_v1.h>             // PID library
#include "AnalogSensor.h"       // Supersampling and calibration

#define FIRMWARE_VERSION "FT_V9.1_SD_Ignore - Edited 2026-04-01"
#define FIRMWARE_BUILD_TIMESTAMP "Built: " __DATE__ " " __TIME__

#define RED_led 15
#define GREEN_led 4
#define ORANGE_led 2

/******  Buttons  ******/
#define BUTTON_PIN 25
#define OLED_BUTTON_PIN 26
Pushbutton BTN_Red(BUTTON_PIN);
Pushbutton BTN_Black(OLED_BUTTON_PIN);
/****** /Buttons  ******/

/******  Draft Potentiometer  ******/
/* A knob/POT on the right-hand side of the blue 3D printed control box
 * controls the set point for the PID controller on the draft/damper system.
 * These variables set the min. and max. setpoints for the draft system,
 * as controlled by the knob/POT.
 */
#define DRAFT_CTRL_MIN_SETPOINT 0.05 // Units in inches of water column
#define DRAFT_CTRL_MAX_SETPOINT 0.10
/****** /Draft Potentiometer  ******/

/******  OLED display  ******/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/****** /OLED display  ******/

/***********  ADC  ***********/
#define I2C_ADDRESS_1 0x48
#define I2C_ADDRESS_2 0x4A
ADS1115_WE adc1(I2C_ADDRESS_1);
ADS1115_WE adc2(I2C_ADDRESS_2);
/*********** /ADC  ***********/

/***********  PID  ***********/
// PWM properties
const int freq = 16000;
const int PWMChannel_1 = 0;
const int PWMChannel_2 = 1;
const int resolution = 8;

const int PWMPin_1 = 32;
const int PWMPin_2 = 33;

const int Ppin = 35;
const int Ipin = 34;
const int Dpin = 36;

#define SETPOINT_POT_PIN 39

double Setpoint, Input, Output_positive, Output_negative;
double Kp = 10, Ki = 0, Kd = 0;
PID myPIDpositive(&Input, &Output_positive, &Setpoint, Kp, Ki, Kd, DIRECT);
PID myPIDnegative(&Input, &Output_negative, &Setpoint, Kp, Ki, Kd, REVERSE);
/*********** /PID  ***********/

RTC_DS3231 rtc;

AnalogSensor *S0_Thermocouple_1;
AnalogSensor *S1_Thermocouple_2;
AnalogSensor *S2_Thermocouple_3;
AnalogSensor *S3_LinearTransducer;
AnalogSensor *S4_SmokePhotometer;
AnalogSensor *S5_OpenAdcChannel;
AnalogSensor *S6_OpenAdcChannel;
AnalogSensor *S7_OpenAdcChannel;
AnalogSensor *S8_DraftPressure;
AnalogSensor *SensorArray[9];

/******  Reporting  *****/
String dataString = "";
String errorString = "";
int record_ticks = 1;     // When recording starts, this is set to zero. It increments every time the sensor values are printed.
bool record = 1;          // True while recording (testing)
int oled_screen = 1;      // Which page is displayed on the OLED
/****** /Reporting  *****/

/******  Timer  ******/
auto timer = timer_create_default();
/****** /Timer  ******/

/******  User Warnings  ******/
float low_limits[] = { -10.0, -10.0, -10.0, -0.1, 0.1, -0.1, -0.1, -0.1};
float high_limits[] = {2000.0, 2000.0, 2000.0, 3.0, 5.0, 5.0, 5.0, 5.0};
bool errors_array[] = {0, 0, 0, 0, 0, 0, 0, 0};
int errorCount = 0;
int orange_blinking = 0;
bool SD_error = 0;
bool error_state = 0;
float position_limit = 1.1; // The orange light turns on when the linear rail passes this point
/******  User Warnings  ******/

/******  Confirm Data Stop ******/
// Require two presses of the red button to stop the data. These variables handle that logic.
bool  ConfirmingStopData = false;
long  ConfirmingStopData_timestamp_millis = 0;
int   ConfirmingStopData_timeout_ms = 3000; // How long does the user have to confirm the data stop
short ConfirmingStopData_countDown_s = 3; // Helps give the user info about the data stop countdown

  /******  Remote Debugging... ******/
  bool fakeButtonPress = false;
  bool getFakeButtonPress(){
    bool foo = fakeButtonPress;
    fakeButtonPress = false;
    return foo;
  }
  bool pressFakeButton(void*){ fakeButtonPress = true; }
  /****** /Remote Debugging... ******/
/****** /Confirm Data Stop ******/

/****** Serial Command Buffer ******/
String serialInputBuffer = "";
/****** /Serial Command Buffer ******/

void setup()
{
  SerialInit();
  Serial.println("Booting...");
  Serial.println(FIRMWARE_VERSION);
  Serial.println(FIRMWARE_BUILD_TIMESTAMP);
  RtcInit();
  OledInit();
  AdcInit();

  // Default pin states
  pinMode(RED_led, OUTPUT);
  pinMode(GREEN_led, OUTPUT);
  pinMode(ORANGE_led, OUTPUT);
  digitalWrite(RED_led, LOW);
  digitalWrite(GREEN_led, LOW);
  digitalWrite(ORANGE_led, LOW);

  SensorSetup();
  PID_Setup();

  //TIMERS
  timer.every(200, T_SlowSensors);
  timer.every(100,  T_FastSensors); // We miss a LOT of events if we try to make this faster than 10Hz right now (2025-07)
  timer.every(100, PID_control);
  
  timer.every(1000, errors_check);
  timer.every(1000, data_String);
  timer.every(1000, OLED_and_PIDsetting);

  // Used to test the 'confirm data stop' feature remotely
  //timer.at(05000, pressFakeButton);
  //timer.at(10000, pressFakeButton);
  //timer.at(12500, pressFakeButton);

  Serial.println("Ready");
}

void SensorSetup(){
  // AnalogSensor(NumOfCalPoints, GainCorrection, OffsetCorrection, RollingBufferSize)
  SensorArray[0] = S0_Thermocouple_1   = new AnalogSensor(5, 1.0233, 1.1, 5 ); // Last Cal 2025-07
  SensorArray[1] = S1_Thermocouple_2   = new AnalogSensor(5, 1.0248, 2.2, 13); // Last Cal 2025-07
  SensorArray[2] = S2_Thermocouple_3   = new AnalogSensor(5, 1.0166, 0.8, 13); // Last Cal 2025-07
  SensorArray[3] = S3_LinearTransducer = new AnalogSensor(5, 1, 0, 5);
  SensorArray[4] = S4_SmokePhotometer  = new AnalogSensor(5, 1, 0, 10);
  SensorArray[5] = S5_OpenAdcChannel   = new AnalogSensor(5, 1, 0, 5);
  SensorArray[6] = S6_OpenAdcChannel   = new AnalogSensor(5, 1, 0, 5);
  SensorArray[7] = S7_OpenAdcChannel   = new AnalogSensor(5, 1, 0, 5);
  //SensorArray[8] = S8_DraftPressure    = new AnalogSensor(5, 1.1125, -0.015, 10); // Last cal. 2021-03-23 - Kaden
  SensorArray[8] = S8_DraftPressure    = new AnalogSensor(5, 1, 0, 10); // 2026-08-11 Reset the cal
}

void SerialInit(){
  Serial.begin(115200);
  delay(50);
  Wire.begin();
  delay(50);
}

void RtcInit(){
  rtc.begin();
  delay(50);
  
  //rtc.adjust(DateTime(2021, 1, 20, 18, 16, 0));
  if (rtc.lostPower()) {
    // Following line sets the RTC with an explicit date & time
    // for example to set January 27 2017 at 12:56 you would call:
    // rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0));
    rtc.adjust(DateTime(2000, 1, 1, 1, 0, 0));
  }
}

DateTime RtcNow(){
  DateTime time = rtc.now();

  

  return time;
}

void OledInit(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(50);
}

void AdcInit(){
  adc1.init();
  delay(50);
  adc2.init();
  delay(50);
  SDP6x.SetSensorResolution(RESOLUTION_12BIT);
  adc1.setVoltageRange_mV(ADS1115_RANGE_6144);
  adc2.setVoltageRange_mV(ADS1115_RANGE_6144);
}

void PID_Setup(){
  delay(50);
  // configure PWM functionalitites
  ledcSetup(PWMChannel_1, freq, resolution);
  ledcSetup(PWMChannel_2, freq, resolution);
  
  // attach the channels to the GPIO to be controlled
  ledcAttachPin(PWMPin_1, PWMChannel_1);
  ledcAttachPin(PWMPin_2, PWMChannel_2);

  Setpoint = 0.076;

  myPIDpositive.SetMode(AUTOMATIC);
  myPIDnegative.SetMode(AUTOMATIC);
  myPIDpositive.SetOutputLimits(0.0, 240.0);
  myPIDnegative.SetOutputLimits(0.0, 240.0);
}

void loop()
{
  timer.tick();

  if (BTN_Red.getSingleDebouncedRelease() || getFakeButtonPress())
  {    Serial.println("test");
    if(record){
      if(!ConfirmingStopData){
        Serial.println("Press the red button again to confirm data stop...");
        Serial.println("3 seconds...");
        ConfirmingStopData = true;
        ConfirmingStopData_timestamp_millis = millis();
      }
      else{
        Serial.println("Data stop confirmed.");
        ConfirmingStopData = false;
        record = false;
      }
    }
    else {
      record = true;
      record_ticks = 0;
    }


    // At the beginning/end of a test, turn on the warning light for the linear transducer to help remind us to put it back
    if ( record && (S3_LinearTransducer->GetConvertedValue() > position_limit) ) orange_blinking = 1;
  }

  // Count down to cancel the data stop
  if(ConfirmingStopData){
    int elapsed = millis() - ConfirmingStopData_timestamp_millis;

    if(ConfirmingStopData_countDown_s > 2 && elapsed > 1000){
      Serial.println("2 seconds...");
      ConfirmingStopData_countDown_s = 2;
    }
    if(ConfirmingStopData_countDown_s > 1 && elapsed > 2000){
      Serial.println("1 second...");
      ConfirmingStopData_countDown_s = 1;
    }

    if(elapsed > ConfirmingStopData_timeout_ms){
      Serial.println("Data stop cancelled.");
      ConfirmingStopData = false;
      ConfirmingStopData_countDown_s = ConfirmingStopData_timeout_ms / 1000;
    }

  }


  if (BTN_Black.getSingleDebouncedRelease()){
    if (oled_screen < 3) oled_screen++;
    else oled_screen = 1;
  }

  // Non-blocking serial command reader. Accumulates characters until newline, then dispatches.
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialInputBuffer.length() > 0) {
        handleSerialCommand(serialInputBuffer);
        serialInputBuffer = "";
      }
    } else {
      serialInputBuffer += c;
      if (serialInputBuffer.length() > 64) serialInputBuffer = ""; // guard against runaway input
    }
  }
}

// Handle a complete serial command line.
//
// Supported commands:
//   SETTIME:YYYY-MM-DD HH:MM:SS   Set the RTC to the specified date and time.
//                                  Example: SETTIME:2026-04-01 15:30:00
//   VERSION                        Print the firmware version string.
void handleSerialCommand(String cmd) {
  cmd.trim();

  if (cmd.startsWith("SETTIME:")) {
    // Expected format: SETTIME:YYYY-MM-DD HH:MM:SS  (27 chars total)
    String ts = cmd.substring(8);
    if (ts.length() < 19 || ts[4] != '-' || ts[7] != '-' || ts[10] != ' ' || ts[13] != ':' || ts[16] != ':') {
      Serial.println("ERROR: Bad SETTIME format. Use SETTIME:YYYY-MM-DD HH:MM:SS");
      return;
    }
    int yr  = ts.substring(0,  4).toInt();
    int mo  = ts.substring(5,  7).toInt();
    int dy  = ts.substring(8,  10).toInt();
    int hr  = ts.substring(11, 13).toInt();
    int mn  = ts.substring(14, 16).toInt();
    int sc  = ts.substring(17, 19).toInt();
    rtc.adjust(DateTime(yr, mo, dy, hr, mn, sc));
    DateTime now = rtc.now();
    Serial.print("RTC set to: ");
    Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

  } else if (cmd == "VERSION") {
    Serial.println(FIRMWARE_VERSION);
    Serial.println(FIRMWARE_BUILD_TIMESTAMP);

  } else {
    // Swallow unknown commands silently so stray characters don't spam the log.
  }
}

bool PID_control(void*)  {
  Input = S8_DraftPressure->GetConvertedValue();  // NOTE: This is a 1s moving average. Maybe we should change this to the instantanious value instead, or at least a 100ms moving average.
                                                  // I think the best thing to do would be to make a second AnalogSensor object for the draft pressure that has less filtering, specifically for PID.
                                                  // I like the 1s moving average for reporting, since that's a 1s log rate, but for PID the value should be closer to the instantanious RDG.

  if (Setpoint > Input) {
    myPIDpositive.SetTunings(Kp, Ki, Kd);
    myPIDpositive.Compute();
    ledcWrite(PWMChannel_1, Output_positive);
    ledcWrite(PWMChannel_2, 0.0);
  }
  else if (Setpoint < Input) {
    myPIDnegative.SetTunings(Kp, Ki, Kd);
    myPIDnegative.Compute();
    ledcWrite(PWMChannel_2, Output_negative);
    ledcWrite(PWMChannel_1, 0.0);
  }

  return true;
}

int fastSensorCtr = 0;
bool T_FastSensors(void*){
  //if(fastSensorCtr++%10 == 0){Serial.println("fs");}
  //Serial.println("fs");

  S4_SmokePhotometer->AddMeasurement(AdcRead(4));
  S8_DraftPressure  ->AddMeasurement(SDP6x.GetPressureDiff() / 249.0889);

  return true;
}

bool T_SlowSensors(void*){
  S0_Thermocouple_1  ->AddMeasurement( (AdcRead(0) * 2.3617 - 1.25) * 200); // Most of this equation comes from AdaFruit. The 2.3617 thing is because of a voltage divider I had to implement to extend the range.
  S1_Thermocouple_2  ->AddMeasurement( (AdcRead(1) * 2.3617 - 1.25) * 200);
  S2_Thermocouple_3  ->AddMeasurement( (AdcRead(2) * 2.3617 - 1.25) * 200);
  S3_LinearTransducer->AddMeasurement(  AdcRead(3) );
  S5_OpenAdcChannel  ->AddMeasurement(  AdcRead(5) );
  S6_OpenAdcChannel  ->AddMeasurement(  AdcRead(6) );
  S7_OpenAdcChannel  ->AddMeasurement(  AdcRead(7) );
  return true;
}

double AdcRead(unsigned short AdcCh){
  ADS1115_WE adc;

  if(AdcCh < 4) { adc = adc1; }
  else          { adc = adc2; AdcCh -= 4; }
  
  /* Set the ADC inputs to be compared
   * 0x4000 ->  compares 0 with GND
   * 0x5000 ->  compares 1 with GND
   * 0x6000 ->  compares 2 with GND
   * 0x7000 ->  compares 3 with GND
  */
  adc.setCompareChannels( (ADS1115_MUX)((AdcCh + 4) << 12) ); // This stupid function wants me to send it a fucking ENUM for each possible channel. Fuck that noise. Assholes.
  
  adc.startSingleMeasurement();
  while (adc.isBusy());
  
  return adc.getResult_V();
}

bool data_String(void*) {
  DateTime time = rtc.now();
  if (record_ticks == 0) {
    dataString = "";
    dataString += ".....Record started at:....";
    dataString += String(time.timestamp(DateTime::TIMESTAMP_TIME));
    dataString += " on ";
    dataString += String(time.timestamp(DateTime::TIMESTAMP_DATE));
    dataString += ";";
    dataString += "\n";
    dataString += "#;Date and Time;Temp 1 [C]; Temp 2 [C]; Temp 3 [C]; Potentiometer [V]; Photocell [V]; Sensor 2 [V]; Sensor 3 [V]; Sensor 4 [V]; Tunnel Pressure [INWC]";
    dataString += "\n";
  } else {
    dataString = "";
    
    // Add leading spaces so that the numbers are always in the same positions on the screen
    if(record_ticks < 10  ) dataString += " ";
    if(record_ticks < 100 ) dataString += " ";
    if(record_ticks < 1000) dataString += " ";
    
    dataString += record_ticks;
    dataString += "; ";
    dataString += String(time.timestamp(DateTime::TIMESTAMP_DATE));
    dataString += " ";
    dataString += String(time.timestamp(DateTime::TIMESTAMP_TIME));
    dataString += ";    ";

    dataString += String(S0_Thermocouple_1  ->GetConvertedValue(), 1);dataString += "; ";     if(S0_Thermocouple_1->GetConvertedValue() < 100) dataString += " ";
    dataString += String(S1_Thermocouple_2  ->GetConvertedValue(), 1);dataString += "; ";     if(S1_Thermocouple_2->GetConvertedValue() < 100) dataString += " ";
    dataString += String(S2_Thermocouple_3  ->GetConvertedValue(), 1);dataString += ";    ";  if(S2_Thermocouple_3->GetConvertedValue() < 100) dataString += " ";
    dataString += String(S3_LinearTransducer->GetConvertedValue(), 3);dataString += ";    ";
    dataString += String(S4_SmokePhotometer ->GetConvertedValue(), 3);dataString += ";    ";
    dataString += String(S5_OpenAdcChannel  ->GetConvertedValue(), 0);dataString += "; ";
    dataString += String(S6_OpenAdcChannel  ->GetConvertedValue(), 0);dataString += "; ";
    dataString += String(S7_OpenAdcChannel  ->GetConvertedValue(), 0);dataString += ";    ";
    dataString += String(S8_DraftPressure   ->GetConvertedValue(), 3);
  }

  if (record == 1) {
    //SD.begin();
    Serial.println(dataString);
    //appendFile(SD, "/Fire_tunnel_datalog.txt", dataString.c_str());
    digitalWrite(GREEN_led, HIGH);
    record_ticks++;
  }
  else {
    digitalWrite(GREEN_led, LOW);
    //SD.end();
    record_ticks = 1;
  }
  return true;
}

bool errors_check(void*) {
  CheckChannelLimits();

  if (errorCount + SD_error != 0)  {
    error_state = 1;
    digitalWrite(RED_led, !digitalRead(RED_led));
    
    if (SD_error == 1) {
      // oled_screen = 5;
    } else {
      if (oled_screen == 1) oled_screen = 4;
      else                  oled_screen = 1;
    }
  }
  else if (error_state == 1 && errorCount + SD_error == 0)  {
    digitalWrite(RED_led, LOW);
    oled_screen = 1;
    error_state = 0;
  }

  if (orange_blinking == 1 && S3_LinearTransducer->GetConvertedValue() > position_limit)  {
    digitalWrite(ORANGE_led, !digitalRead(ORANGE_led));
  } else if (S3_LinearTransducer->GetConvertedValue() < position_limit)  {
    digitalWrite(ORANGE_led, LOW);
    orange_blinking = 0;
  }
  return true;
}

void CheckChannelLimits(){
  errorCount = 0;
  errorString = "";
  
  for (int i = 0; i < 8; i++) {  
    if (SensorArray[i]->GetConvertedValue() < low_limits[i]) {
      errors_array[i] = 1;
      errorString += i;
      errorString += ";";
    } else if (SensorArray[i]->GetConvertedValue() > high_limits[i]) {
      errors_array[i] = 1;
      errorString += i;
      errorString += ";";
    }
    else {
      errors_array[i] = 0;
    }
    errorCount += errors_array[i];
  }
}

bool OLED_and_PIDsetting(void*) {
  Setpoint = DRAFT_CTRL_MIN_SETPOINT + (analogRead(SETPOINT_POT_PIN) * ( (DRAFT_CTRL_MAX_SETPOINT - DRAFT_CTRL_MIN_SETPOINT) / 4095.0000));

  Kp = analogRead(Ppin) * (10000.0 / 4095.0);
  Ki = analogRead(Ipin) * (10000.0 / 4095.0);
  Kd = analogRead(Dpin) * (10000.0 / 4095.0);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  //oled_screen = 2;

  switch (oled_screen)  {
    case 1:
      display.print("T1:"); display.print(S0_Thermocouple_1  ->GetConvertedValue(), 1); display.print(" T2:");  display.println(S1_Thermocouple_2  ->GetConvertedValue(), 1);
      display.print("T3:"); display.print(S2_Thermocouple_3  ->GetConvertedValue(), 1); display.print(" Pot:"); display.println(S3_LinearTransducer->GetConvertedValue(), 3);
      display.print("PhC:"); display.print(S4_SmokePhotometer->GetConvertedValue(), 3); display.print(" Ch6:"); display.println(S5_OpenAdcChannel  ->GetConvertedValue(), 3);
      display.print("Ch7:"); display.print(S6_OpenAdcChannel ->GetConvertedValue(), 3); display.print(" Ch8:"); display.println(S7_OpenAdcChannel  ->GetConvertedValue(), 3);
      break;
    case 2:
      display.print("Kp:"); display.println(Kp / 1000, 2);
      display.print("Ki:"); display.println(Ki / 1000, 2);
      display.print("Kd:"); display.println(Kd / 1000, 2);
      display.print("SP:"); display.println(Setpoint, 4);
      break;
    case 3:
      display.print("Out 1:"); display.println(Output_positive, 0);
      display.print("Out 2:"); display.println(Output_negative, 0);
      display.print("Pressure"); display.println(S8_DraftPressure->GetConvertedValue(), 4);
      display.print("SP:"); display.println(Setpoint, 4);
      break;
    case 4:
      display.println("Check channels: ");
      //display.setCursor(0, 8);
      display.print(errorString);

      break;
    case 5:
      display.print("Check SD card!");

      break;
  }
  display.display();
  return true;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    //Serial.println("Failed to open file for writing");
    SD_error = 1;
    return;
  }
  if (file.print(message)) {
    //Serial.println("File written");
    SD_error = 0;
  } else {
    //Serial.println("Write failed");
    SD_error = 1;
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    //Serial.println("Failed to open file for appending");
    SD_error = 1;
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
    SD_error = 0;
  } else {
    SD_error = 1;
    SD.end();
    //Serial.println("Append failed");
    SD.begin();
    Serial.flush();
    return;
  }
  file.close();
  SD_error = 0;
}
