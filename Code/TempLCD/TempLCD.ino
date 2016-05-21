#include <OneWire.h>
#include <DallasTemperature.h>

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
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20) | _BV(WGM21) ; 
  TCCR2B = 0;
  OCR2B = 25  ;

  // Pre-heat the heater
  digitalWrite(WATER_PUMP, LOW);
  digitalWrite(HEATER, HIGH);
  delay(3000);
  digitalWrite(WATER_PUMP, HIGH);
}

int dummy = 0;

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
  double temperature = sensors.getTempCByIndex(0);

  // Check if we need to stop pumping water to let
  // the heater warm up
  float error = temperature - TARGET_TEMP;
  if(abs(error) > 5) {
    digitalWrite(WATER_PUMP, LOW);
    delay(5000);
    digitalWrite(WATER_PUMP, HIGH);
    delay(7000);
  }
  else
    digitalWrite(WATER_PUMP, HIGH);

  // Write data to serial for GUI / Debugging
  Serial.print("{\"hall_sensor\": ");
  Serial.print(analogRead(A0));
  Serial.print(", \"water_temp\": ");
  Serial.print(temperature);
  Serial.println("}");
 }
