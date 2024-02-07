#include "Arduino.h"
#include "Keyboard.h"

#ifdef USER_BTN
const int buttonPin = USER_BTN;          // input pin for pushbutton
#else
const int buttonPin = PB10;          // input pin for pushbutton
#endif

int previousButtonState = HIGH;   // for checking the state of a pushButton
int counter = 0;                  // button push counter

//#define key_code KEY_RIGHT_CTRL  //right control (wolfpack default)
#define key_code ' '

void setup() {
  // make the pushButton pin an input:
  pinMode(buttonPin, INPUT_PULLUP);
  // initialize control over the keyboard:
  Keyboard.begin();
}

void loop() {
  // read the pushbutton:
  int buttonState = digitalRead(buttonPin);

  if (buttonState != previousButtonState){
    if(buttonState == LOW){
      Keyboard.press(key_code);
    }
    else{
      Keyboard.release(key_code);
    }

  }
  // save the current button state for comparison next time:
  previousButtonState = buttonState;
}