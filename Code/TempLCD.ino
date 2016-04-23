/**
* Pressto PID Temperature Control Program
* 
* This code uses the temperature value obtained from a OneWire sensor to 
* produce a signal controlling the heater block.
*
* Author: Miro Markaravanes <miromarkarian[at]gmail[dot].com>
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <PID_v1.h>

// PID variables
double setPoint, input, output;

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 6

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 2, 3, 4, 5);

// PID Controller
// P is: 1
// I is: 3
// D is: 0.5
// Control mode is: DIRECT
PID myPID(&input, &output, &setPoint, 1, 3, 0.5, DIRECT);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(9, OUTPUT); 
  pinMode(10, OUTPUT);
  Serial.begin(9600);
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  // Set the target temperature to 90 C
  setPoint = 95;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  digitalWrite(10, LOW);
  digitalWrite(9, HIGH);
  delay(5000);
  digitalWrite(10, HIGH);
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  input = sensors.getTempCByIndex(0);
  //myPID.Compute();

  // Cut the output in case our temperature is over the set point
  // to keep water from hitting the boiling point
  //if(input > setPoint)
  //  output = 0;

  float error = input - setPoint;
  if(abs(error) > 5) {
    digitalWrite(10, LOW);
    delay(5000);
    digitalWrite(10, HIGH);
    delay(10000);
  }
  else
    digitalWrite(9, HIGH);
  
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(sensors.getTempCByIndex(0));
  lcd.setCursor(0,1);
  lcd.print("Error: ");
  lcd.print(error);
  Serial.println(sensors.getTempCByIndex(0));
}


