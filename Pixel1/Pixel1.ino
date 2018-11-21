#include <Encoder.h>
#include <EEPROM.h>
const int relay1 = 12;
const int relay2 = 9;

const int Pressure = A5;// Pressure Sensing
int PWM = 0;
int Inverse_PWM = 0;
int level = 0;

Encoder myEnc(2, 7); //  Set up the linear actuator encoder using pins which support interrupts, avoid using pins with LEDs attached i.e. 13
long oldPosition  = -99999; //   intializing it with random negative value
long oldPressure  = -99999; //   intializing it with random negative value
int currentPosition  = -99999; //   intializing it with random negative value
int correction = 0;

void setup() {


  pinMode(Pressure, INPUT); //Pressure Sensor
  pinMode(relay1, OUTPUT); //Initiates Motor Channel A pin
  pinMode(relay2, OUTPUT); //Initiates Brake Channel A pin
  Serial.begin(9600);
  correction = EEPROMReadInt(0); // reading the last position of motor from EEPROM to later caliberate HallEffect sensor values


}

void loop() {


  if (oldPressure > 19 &&  currentPosition > level )
  {
    retractActuator();
  }
  else
  {
    if (currentPosition < level - 100)
    {
      extendActuator();
    }
    else
    {
      stopActuator();
    }

  }



  long newPressure = analogRead(Pressure) / 10; //checking if the pressure has changed

  if (newPressure != oldPressure) {
    oldPressure = newPressure;
    PWM = map(newPressure, 20, 99, 0, 255);  // speed for retraction
    Inverse_PWM = map(newPressure, 99, 3, 0, 255);  // speed for extraction
    level = map(newPressure, 10, 99, 6300, 0);
  }


  long newPosition = myEnc.read();  //check the encoder to see if the position has changed
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    currentPosition = newPosition + correction; //caliberating the motor position
    EEPROMWriteInt(0, currentPosition); //saving motor position in EEPROM

  }

  char c = newPressure;
  Serial.write(c); // sending pressure value through Xbee


}

void extendActuator() {
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, LOW);
  analogWrite(3, Inverse_PWM);
}

void retractActuator() {
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  analogWrite(3, PWM);
}

void stopActuator() {
  digitalWrite(relay2, HIGH);
  analogWrite(3, 0);
}

void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}

int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}
