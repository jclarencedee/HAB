/*
This code is intended for final impplementation into the Flightmodel of the HAB Capstone Project.
Team members:
Bemmann, Olivier; Corsi, Christopher; Dee, Jan Clarence; Drudi, Lisa; Papanagiotou, Emmanuel; Romita, Jonathan; Trudel, Carole-Anne.

REQUIRED LIBRARIES
Dallas Temperature: https://www.hacktronics.com/code/DallasTemperature.zip
OneWire: https://www.hacktronics.com/code/OneWire.zip
ADXL345

USEFUL RESOURCES
Temperature Sensor OneWire Display
https://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html

Temperature Sensor Address Display
https://www.pjrc.com/teensy/td_libs_OneWire.html

GY80 IMU Sensor
http://www.forkrobotics.com/2013/06/using-the-gy80-10dof-module-with-arduino/
*/

/*
**************** 
INITIALIZATION
****************
Global Vaiables:
logfile: Actual File that is saved inside the SD Card
filename: Name of the file inside the SD Card
T1, T2 & T3: Address of each temperature sensor

Pins required
SD Card: Pin 4
Temperature Sensor: Digital Pin 5
IMU Sensor: SDA, SCL
RTC: SDA, SCL
LED: PIN 13
*/
#include <TimeLib.h> //For Time Logging
#include <SPI.h> //For Time logging
#include <SD.h> //SD Card Library
#include <OneWire.h> //For temperature sensors, we are using a one wire protocol
#include <DallasTemperature.h> //For temperature sensors

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

//The SD Card is set to Pin 4
#define cardSelect 4

//For Temperature Sensor, Use a One Wire Bus System
#define ONE_WIRE_BUS 5
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//For the temperature Sensors, we will use a OneWire Bus.
//This allows us to use only one digital pin for up to 10 temperature sensors. In order to do this, the user needs to determine the device addresses of each temperature sensor
//In our case, the adresses are as follows:
//R=28 DD B 59 5 0 0 9E 
//R=28 FB E6 58 5 0 0 23
//R=28 7 FA 58 5 0 0 71 

DeviceAddress T1 = { 0x28, 0xDD, 0xB, 0x59, 0x5, 0x0, 0x0, 0x9E };
DeviceAddress T2 = { 0x28, 0xFB, 0xE6, 0x58, 0x5, 0x0, 0x0, 0x23 };
DeviceAddress T3 = { 0x28, 0x7, 0xFA, 0x58, 0x5, 0x0, 0x0, 0x71 };

//GLOBAL VARIABLES
File logfile;
char filename[15];

/*
**************** 
SETUP
****************
1 - Set Input and Output Pins
2 - Initialize the temperature sensors
3 - Create new LOG text File
*/

void setup(void) {
  Serial.begin(9600);

  //Only on some arduino's will you be able to print from setup.
  Serial.println("\r\nBegin Code");
  pinMode(13, OUTPUT);

  //FOR TEMPERATURE SENSORS
  //Start up the library
  sensors.begin();
  // set the resolution to 10 bit
  sensors.setResolution(T1, 10);
  sensors.setResolution(T2, 10);
  sensors.setResolution(T3, 10);
    
  //See if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }

  //Create a NEW text file
  strcpy(filename, "FLIGHT00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  //Open the created text file
  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    error(3);
  }

  Serial.print("Writing to "); 
  Serial.println(filename);
  Serial.println("Ready!");

  //Close log file for now, reopen it later
  logfile.close();
}

/*
**************** 
MAIN LOOP
****************
1 - Open Log File
2 - Set the Time
3 - Write on Serial Monitor
4 - Write on the File (INCOMPLETE)*************
  a - Time
  b - Temperature
5 -Close Log fILE
*/
void loop() {
  
  delay(2000);
  //Open Log File
  logfile = SD.open(filename, FILE_WRITE);
  
  //Set Time 
  if(Serial.available() != 0);
    processSyncMessage();

  // Write on Serial Monitor
  //Display Time + Temperature
  logtime();
  sensors.requestTemperatures();
  logtemperature(T1);
  Serial.print("\n\r");
  logtemperature(T2);
  Serial.print("\n\r");
  logtemperature(T3);
  Serial.print("\n\r");
  Serial.print("~~~\n\r");

  //Close File
  logfile.close();
}

/*
**************** 
Custome Functions
****************
logtime
Inputs: N/A
Outputs: N/A
Function: Displays the time in Serial Monitor

printDigits
Inputs: Integer
Outputs: N/A
Function: Utility function for digital clock display (It prints preceding colon and leading 0 for time display)

processSyncMessage
Inputs: N/A
Outputs: N/A
Function: If time sync available from serial port, update time and return true (ie Allows the user to input a custom time)

logtemperature
Inputs: DeviceAddress
Outputs: N/A
Function: Displays the temperature for Temperature Sensors which have PREDETERMINED addresses

error
Input: Integer
Outputs: N/A
Functions: Blinks an error message with an error code
*/

void logtime(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void processSyncMessage() {
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of header & 10 ASCII digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      setTime(pctime);   // Sync Arduino clock to the time received on the serial port
    }  
  }
}

void logtemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Serial.print("C: ");
    Serial.print(tempC);
    Serial.print(" F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
  }
}

void error(int errno) {
  Serial.println("Error: ");
  Serial.println(errno);
  Serial.println("/n");
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}
