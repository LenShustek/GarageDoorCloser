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
// 2017.08.24, L. Shustek. Add optional light/sound warning, and pushbutton for cancelling
// 2019.06.21, L. Shustek, Add optional lights to show the status of the switches
//------------------------------------------------------------------------------------------------
/*  (C) Copyright 2017,2019 Len Shustek

   This program is free software: you can redistribute it and/or modify it under the terms of
   version 3 of the GNU General Public License as published by the Free Software Foundation at
   http://www.gnu.org/licenses, with Additional Permissions under term 7(b) that the original
   copyright notice and author attibution must be preserved, and under term 7(c) that modified
   versions be marked as different from the original.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
   the GNU General Public License for more details.
*/

#define TIMEOUT_SECONDS  (unsigned long)(10*60)  // how long the door can stay open
#define CLOSETIME_SECONDS (unsigned long)20      // max time for the door to close
#define WARNTIME_SECONDS (unsigned long) 30      // how long we warn for before closing

#define FREQ_LOW   100
#define FREQ_HIGH  2000

#define LED 13           // light on the arduino nano board
#define RELAY 11         // relay control output
#define CLOSED_SWITCH 2  // "door closed" switch
#define OPEN_SWITCH 3    // "door open" switch
#define TEST_MODE 4      // "test mode" jumper
#define BUTTON 5         // "cancel close" pushbutton
#define BUTTON_LIGHT 6   // "door about to close" light
#define SPEAKER 7        // "door about to close" sounder
#define OPEN_LIGHT 9     // "door is open" switch is closed
#define CLOSED_LIGHT 10  // "door is closed" switch is closed

#define ACTIVE 0         // switches are active low

#include <TimerOne.h>

unsigned long timeout;

void setup() {
   pinMode(LED, OUTPUT);
   pinMode(RELAY, OUTPUT);
   pinMode(CLOSED_SWITCH, INPUT_PULLUP);
   pinMode(OPEN_SWITCH, INPUT_PULLUP);
   pinMode(TEST_MODE, INPUT_PULLUP);
   pinMode(BUTTON, INPUT_PULLUP);
   pinMode(BUTTON_LIGHT, OUTPUT);
   pinMode(SPEAKER, OUTPUT);
   pinMode(OPEN_LIGHT, OUTPUT);
   pinMode(CLOSED_LIGHT, OUTPUT);
   //digitalWrite(LED, 0);
   //digitalWrite(RELAY, 0);
   Timer1.initialize(100000);  // interrupt 10 times a second
   Timer1.attachInterrupt(do_status_lights);
   timeout = TIMEOUT_SECONDS;
   if (digitalRead(TEST_MODE) == ACTIVE)
      timeout /= 60;  // if test mode, convert timeout from minutes to seconds
}

void do_status_lights(void) {  // make the switch lights follow the switches
   static int count_interrupts = 0;
   if (++count_interrupts > 20) { // every two seconds, blink the lights off
      count_interrupts = 0;      // to show that we are alive
      digitalWrite(OPEN_LIGHT, 0);
      digitalWrite(CLOSED_LIGHT, 0); }
   else {
      digitalWrite(OPEN_LIGHT, 1 - digitalRead(OPEN_SWITCH));
      digitalWrite(CLOSED_LIGHT, 1 - digitalRead(CLOSED_SWITCH))  ; } }

void beep (int frequency, int duration_msec) {
   // we don't use tone() because it interferes with pin 3
   int halfperiod_usec = (long int) 500000 / (long int) frequency;
   unsigned long timerstart;
   timerstart = millis();
   while (millis() - timerstart < duration_msec) {
      digitalWrite(SPEAKER, 1);
      delayMicroseconds(halfperiod_usec);
      digitalWrite(SPEAKER, 0);
      delayMicroseconds(halfperiod_usec); } }

bool give_warning(void) { // return TRUE if close is cancelled
   unsigned long timerstart;
   unsigned int freq = FREQ_HIGH;
   int light = 0;
   int light_time = 0;
   timerstart = millis();
   while ((millis() - timerstart) / 1000UL < WARNTIME_SECONDS) {
      if (digitalRead(OPEN_SWITCH) != ACTIVE) // she closed the door
         goto cancelled;
      beep(freq, 10); // do sound for this many msec ("period")
      freq -= 20; // change frequency by this many hertz each period
      if (freq < FREQ_LOW) freq = FREQ_HIGH;
      if (++light_time > 20) { // change light every this many periods
         light ^= 1; // reverse light
         digitalWrite(BUTTON_LIGHT, light);
         light_time = 0; }
      if (digitalRead(BUTTON) == ACTIVE) { // button was pushed
cancelled:
         digitalWrite(BUTTON_LIGHT, 0);
         return true; } }
   digitalWrite(BUTTON_LIGHT, 0);
   return false; }

void pulse_button (void) {
   digitalWrite(RELAY, 1);
   delay(1000);
   digitalWrite(RELAY, 0);
   delay(1000); }

void loop() {
   unsigned long timerstart;
   if (digitalRead(OPEN_SWITCH) == ACTIVE) { // door has opened
      digitalWrite(LED, 1);
      timerstart = millis();  // start timing
      while (digitalRead(OPEN_SWITCH) == ACTIVE) {
         if ((millis() - timerstart) / 1000UL > timeout) { // open too long
            if (give_warning()) goto exit; // warn, and see if she cancels
            pulse_button();  // start closing it
            timerstart = millis();
            while (digitalRead(CLOSED_SWITCH) != ACTIVE) { // wait for it to close
               if ((millis() - timerstart) / 1000UL > CLOSETIME_SECONDS) { // took too long
                  pulse_button(); // do it again
                  goto exit; } } } }
exit: digitalWrite(LED, 0); } }
