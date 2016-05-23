#include <OneWire.h>
#include <DallasTemperature.h>

// Non-blocking Delay Functions
// https://www.element14.com/community/groups/arduino/blog/2014/06/05/a-non-blocking-delay#comment-36267
#define DELAY_DECLARE(name) volatile unsigned long delay_variable_##name  
#define DELAY_SET(name, time) delay_variable_##name = millis() + time  
#define DELAY_DONE(name) (delay_variable_##name <= millis())  

// Stepper Motor Pins
#define MS1 0
#define MS2 1
#define DIR 2
#define STEP 3

// Piston Control Pins
#define UP_KEY 12
#define DOWN_KEY 13

// Temperature Sensor Pin
#define TEMP_SENSOR 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TEMP_SENSOR);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Target Water Temperature
#define TARGET_TEMP 45;

// Water Pump Pin
#define WATER_PUMP 7

// Heater Pin
#define HEATER 8

bool isGoingUp = false;
bool isGoingDown = false;

DELAY_DECLARE(heating);
DELAY_DECLARE(pouring);

void setup()
{  
  Serial.begin(9600);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, LOW);

  // Piston Control Keys
  pinMode(UP_KEY, INPUT);
  pinMode(DOWN_KEY, INPUT);

  pinMode(HEATER, OUTPUT);
  pinMode(WATER_PUMP, OUTPUT);

  // Initialize Hardware Fast PWM on Timer2
  // http://playground.arduino.cc/Main/TimerPWMCheatsheet
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20) | _BV(WGM21) ; 
  TCCR2B = 0;
  OCR2B = 25;
  
  // Print status to serial
  Serial.println("{\"status\": \"pre-heating\"}");

  // Pre-heat the heater
  digitalWrite(WATER_PUMP, LOW);
  digitalWrite(HEATER, HIGH);
  delay(3000);
  digitalWrite(WATER_PUMP, HIGH);
}

void loop()
{ 
  // Start PWM to move piston if necessary
  if(digitalRead(UP_KEY) == LOW && !isGoingUp) {
    isGoingDown = false;
    isGoingUp = true;
    TCCR2B = (TCCR2B & 0b11111000) | 0x03;
    digitalWrite(DIR, LOW);
    delay(500);
    TCCR2B = (TCCR2B & 0b11111000) | 0x02;
  } else if(digitalRead(DOWN_KEY) == LOW && !isGoingDown) {
    isGoingDown = true;
    isGoingUp = false;
    TCCR2B = (TCCR2B & 0b11111000) | 0x03;
    digitalWrite(DIR, HIGH);
    delay(500);
    TCCR2B = (TCCR2B & 0b11111000) | 0x02;
  } else if(digitalRead(UP_KEY) == HIGH && digitalRead(DOWN_KEY) == HIGH) {
    isGoingDown = false;
    isGoingUp = false;
    TCCR2B = 0;
  }
  
  // Send command to get temperatures
  sensors.requestTemperatures();
  // Retrieve the current water temperature
  double temperature = sensors.getTempCByIndex(0);
  
  // Use non-blocking heating to avoid locking up the piston mechanism and the other functionalities
  if(DELAY_DONE(heating)) {
  	if(DELAY_DONE(pouring) {
  	  // Check if we need to stop pumping water to let
  	  // the heater warm up
  	  float error = temperature - TARGET_TEMP;
  	  if(abs(error) > 5) {
  	    // Turn off the water pump and let the heater run for 5 seconds
  	    // Schedule the pump to run for 7 seconds after the heating
        digitalWrite(WATER_PUMP, LOW);
		DELAY_SET(heating, 5000);
		DELAY_SET(pouring, 12000);
      } else {
        // Temperature is within the limits
        // Keep the pump on
        digitalWrite(WATER_PUMP, HIGH);
      }
  	} else {
  	  // Heating is finished, but pumping water hasn't yet
  	  // Keep the pump on
      digitalWrite(WATER_PUMP, HIGH);
  	}
  } else {
    // Heating hasn't finished yet
    // Keep the pump off
    digitalWrite(WATER_PUMP, LOW);
  }

  // Write data to serial for GUI / Debugging
  Serial.print("{\"status\": \"running\"");
  Serial.print(", \"hall_sensor\": ");
  Serial.print(analogRead(A0));
  Serial.print(", \"water_temp\": ");
  Serial.print(temperature);
  Serial.println("}");
 }
