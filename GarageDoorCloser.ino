//------------------------------------------------------------------------------------------------
//   Garage door auto-closer
//
// This uses an Arduino Nano to deect when a garage door has been open for too long, and if
// so, simulates a press to the garage door's "close" pushbutton. 
//
// The inputs are two magnetic reed sensors: one reports if the door is completely open,
// and one if the door is completely closed.
//
//------------------------------------------------------------------------------------------------
// 2017.05.26, L. Shustek. Built for Harry Saal.
// 2017.07.09, L. Shustek. Add jumper which shortens delay for testing.
//------------------------------------------------------------------------------------------------
/*  (C) Copyright 2017, Len Shustek

This program is free software: you can redistribute it and/or modify it under the terms of 
version 3 of the GNU General Public License as published by the Free Software Foundation at 
http://www.gnu.org/licenses, with Additional Permissions under term 7(b) that the original 
copyright notice and author attibution must be preserved, and under term 7(c) that modified 
versions be marked as different from the original. 

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
the GNU General Public License for more details. 
*/

#define TIMEOUT_SECONDS  (unsigned long)(15*60)  // how long the door can stay open
#define CLOSETIME_SECONDS (unsigned long)20      // max time for the door to close

#define LED 13           // light on the arduino nano board
#define RELAY 11         // relay control output
#define CLOSED_SWITCH 2  // "door closed" switch
#define OPEN_SWITCH 3    // "door open" switch
#define TEST_MODE 4      // "test mode" jumper
#define ACTIVE 0         // switches are active low

unsigned long timeout;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(CLOSED_SWITCH, INPUT_PULLUP);
  pinMode(OPEN_SWITCH, INPUT_PULLUP);
  pinMode(TEST_MODE, INPUT_PULLUP);
  digitalWrite(LED, 0);
  digitalWrite(RELAY, 0);
  timeout = TIMEOUT_SECONDS;
  if (digitalRead(TEST_MODE) == ACTIVE)
	  timeout /= 60;  // if test mode, convert timeout from minutes to seconds
}

void hardware_test(void) {
  while (1) {
    if (digitalRead(CLOSED_SWITCH) == ACTIVE)
      digitalWrite(LED, 1);
    else digitalWrite(LED, 0);
    if (digitalRead(OPEN_SWITCH) == ACTIVE)
      digitalWrite(RELAY, 1);
    else digitalWrite(RELAY, 0);
  }
}
void pulse_button (void) {
  digitalWrite(RELAY, 1);
  delay(1000);
  digitalWrite(RELAY, 0);
  delay(1000);
}
void loop() {
  unsigned long timerstart;
  if (digitalRead(OPEN_SWITCH) == ACTIVE) { // door has opened
    digitalWrite(LED, 1);
    timerstart = millis();  // start timing
    while (digitalRead(OPEN_SWITCH) == ACTIVE) {
      if ((millis() - timerstart) / 1000UL > timeout) { // open too long
        pulse_button();  // start closing it
        timerstart = millis();
        while (digitalRead(CLOSED_SWITCH) != ACTIVE) { // wait for it to close
          if ((millis() - timerstart) / 1000UL > CLOSETIME_SECONDS) { // took too long
            pulse_button(); // do it again
            goto exit;
          }
        }
      }
    }
exit: digitalWrite(LED, 0);
  }
}
