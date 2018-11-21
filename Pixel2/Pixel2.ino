#include <Encoder.h>
#include <EEPROM.h>

int leftmotorForward = 8;    // pin 8 --- left motor (+) green wire
int leftmotorBackward = 11; // pin 11 --- left motor (-) black wire
int leftmotorspeed = 9;     // pin 9 --- left motor speed signal
int currentPosition = -99999; //   intializing it with random negative value
long oldPosition = -99999;   //   intializing it with random negative value
long newPressure = -99999;   //   intializing it with random negative value
long oldPressure = -99999;   //   intializing it with random negative value

int PWM = 0;
int Inverse_PWM = 0;
int level = 0;
int correction = 0;

Encoder myEnc(2, 7);           //   avoid using pins with LEDs attached i.e. 13

//------------------------------------------------------

void setup() //---3 Pins being used are outputs---
{
  pinMode(leftmotorForward, OUTPUT);
  pinMode(leftmotorBackward, OUTPUT);
  pinMode(leftmotorspeed, OUTPUT);
  Serial.begin(9600);
  correction = EEPROMReadInt(0);  // reading the last position of motor from EEPROM to caliberate HallEffect sensor values

}

// ---Main Program Loop -----------------------------

void loop()
{

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
      Stop();
    }

  }


  if (Serial.available() > 0) {      //checking if any data is available from Xbee
    int value = Serial.read();

    if (value > 0 && value < 100)
      newPressure = value / 1.5;
  }

  if (newPressure != oldPressure) {
    oldPressure = newPressure;
    PWM = map(newPressure, 20, 99, 0, 255);   // speed for retraction
    Inverse_PWM = map(newPressure, 99, 3, 0, 255);   // speed for extraction
    level = map(newPressure, 3, 99, 6500, 0);
  }

  long newPosition = myEnc.read();  //checking the encoder to see if the position has changed
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    currentPosition = newPosition + correction; //caliberating the motor position
    EEPROMWriteInt(0, currentPosition);   //saving motor position in EEPROM

  }
}

//----- "Sub-rutine" Voids called by the main loop

void extendActuator()
{
  analogWrite(9, Inverse_PWM);
  digitalWrite(11, LOW); // Drives LOW outputs down first to avoid damage
  digitalWrite(8, HIGH);
}

void retractActuator()
{
  analogWrite(9, PWM);
  digitalWrite(8, LOW); // Drives LOW outputs down first to avoid damage
  digitalWrite(11, HIGH);
}

void Stop() // Sets speed pins to LOW disabling both motors
{
  digitalWrite(9, LOW);
  digitalWrite(11, LOW);
  digitalWrite(8, LOW);
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
