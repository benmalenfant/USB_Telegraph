#include "Arduino.h"

#ifdef USER_BTN
const int buttonPin = USER_BTN; // input pin for pushbutton
#else
const int sounderPin = PB6;
const int sense1Pin = PA6;
const int sense2Pin = PA7;
#endif

int previousButtonState = HIGH; // for checking the state of a pushButton
int counter = 0;                // button push counter

void setup()
{
  // make the pushButton pin an input:
  // pinMode(buttonPin, INPUT_PULLUP);
  pinMode(sounderPin, OUTPUT);
  // initialize control over the keyboard:
  Serial.begin();

  //SANITY check
  digitalWrite(sounderPin,1);
  delay(500);
  digitalWrite(sounderPin,0);
  delay(500);
}

uint8_t sounder_status = 1;
void loop()
{
  if (Serial.available())
  {
    sounder_status = Serial.read();
    digitalWrite(sounderPin,sounder_status);

    int sens1_read = analogRead(sense1Pin);
    int sens2_read = analogRead(sense2Pin);

    if (sens1_read < 650 || sens2_read > 150)
    {
      Serial.write((uint8_t)1);
      Serial.flush();
    }
    else
    {
      Serial.write((uint8_t)0);
      Serial.flush();
    }
  }
  //delay(70);
  //delayMicroseconds(10000);
  /*
  Serial.print("S1 : ");
  Serial.print(analogRead(sense1Pin));
  Serial.print("|| S2 : ");
  Serial.println(analogRead(sense2Pin));
  digitalWrite(sounderPin, 0);
  */
}