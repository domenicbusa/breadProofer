/*
   AutoPID BasicTempControl Example Sketch

   This program reads a dallas temperature probe as input, potentiometer as setpoint, drives an analog output.
   It lights an LED when the temperature has reached the setpoint.
*/
#include <AutoPID.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//pins
//#define POT_PIN A0
#define OUTPUT_PIN D7 //A1
#define TEMP_PROBE_PIN D2 //5
#define LED_PIN D8 //6

#define TEMP_READ_DELAY 800 //can only read digital temp sensor every ~750ms
#define SERIAL_WRITE_DELAY 1000 //ms

//pid settings and gains
#define TARGET_TEMP_F 85
#define OUTPUT_MIN 0
#define OUTPUT_MAX 255
#define KP 4
#define KI 0.05
#define KD 0

double temperature, setPoint, outputVal;

OneWire oneWire(TEMP_PROBE_PIN);
DallasTemperature temperatureSensors(&oneWire);

//input/output variables passed by reference, so they are updated automatically
AutoPID myPID(&temperature, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate; //tracks clock time of last temp update

unsigned long lastSerialWrite; //tracks clock time of last temp update

//call repeatedly in loop, only updates after a certain time interval
//returns true if update happened
bool updateTemperature() {
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
    temperature = temperatureSensors.getTempFByIndex(0); //get temp reading
    lastTempUpdate = millis();
    temperatureSensors.requestTemperatures(); //request reading for next time
    return true;
  }
  return false;
}//void updateTemperature

//returns true if update happened
bool serialWrite() {
  if ((millis() - lastSerialWrite) > SERIAL_WRITE_DELAY) {
    lastSerialWrite = millis();
    Serial.print(temperature);
    Serial.print(", output : ");
    Serial.print(outputVal);
    Serial.print(", ");
    Serial.print((outputVal/255)*100);
    Serial.println("%");
    return true;
  }
  return false;
}//void serialWrite


void setup() {
  //pinMode(POT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  temperatureSensors.begin();
  temperatureSensors.requestTemperatures();
  while (!updateTemperature()) {} //wait until temp sensor updated

  //if temperature is more than 4 degrees below or above setpoint, OUTPUT will be set to min or max respectively
  myPID.setBangBang(15);
  //set PID update interval to 4000ms
  myPID.setTimeStep(1000);

  Serial.begin(9600);
  Serial.println("SETUP DONE");

}//void setup


void loop() {
  updateTemperature();
  setPoint = TARGET_TEMP_F;
  myPID.run(); //call every loop, updates automatically at certain time interval
  analogWrite(OUTPUT_PIN, outputVal);
  digitalWrite(LED_PIN, outputVal>0); //light up LED when heating is being requested
  serialWrite(); // write data to serial port
}//void loop
