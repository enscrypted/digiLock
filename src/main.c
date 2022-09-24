/*
 * Digital Combination Lock written for Tiva-C tm4c123gh6pm board by Ken Mills
 *
 * Theoretical, not tested with actual lock and logic for unlocking a physical
 *
 * lock not included. All logic for combination set, reset, and input included
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Util/launchpad.h"
#include "seg7.h"
#include "ras.h"
#include "seg7digit.h"
#include "eprom.h"
#include <driverlib/eeprom.h>
#include <math.h>
#include <time.h>


// The initial state of the 7-segment display: "00"
seg7Display_t seg7Display = {
  0, 0, 10, 10, 0
};

//global variables. knobArray has to be used during switch press, moved appropriately
//bools used and updated in multiple functions
uint32_t passwordLock[4];
uint32_t addr = 0x0;
int i = 0;
uint32_t knobArray[1];
bool correct = true;
bool leftClear = true; //leftClear true for 1st two input, false for rightClear (last 2)
bool cleared = false; //not initially cleared until knobArray = 0
int prev = 31;        //initially set to max to avoid clear error for first input (as long as knobArray decrements still)
bool canEnter = true;
uint32_t endTime;     //used for unlock time, global so not reset every callback
bool setEnd = true;   //only update endTime when true
bool unlocked = false;
bool firstPress = true;
bool init_correct = false;
int j = 0;
typedef enum {On,Off}OnOff_t;

// logic for a new combo input success signal
void blinkReset() {
  int x = 0;
  while(x < 2) {
    ledTurnOnOff(false /* red */, false /* blue */, true /* green */);
    waitUs(100000); // wait is in microseconds, 100k us = 100 ms
    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
    waitUs(100000);
    x++;
  }
  return;
}

// logic for error signal
void blinkError() {
  int x = 0;
  while(x < 5) {
    ledTurnOnOff(true /* red */, false /* blue */, false /* green */);
    waitUs(100000); // wait is in microseconds, 100k us = 100 ms
    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
    waitUs(100000);
    x++;
  }
  return;
}

// logic for input signal
void blinkEntry() {
  ledTurnOnOff(false /* red */, true /* blue */, false /* green */);
  waitUs(100000);
  ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
  return;
}

// called once new combo is desired (SW2 press while unlocked)
// allows selection of 4 new numbers (displays them for debug purposes)
void checkNewPass(uint32_t time) {
  if(!unlocked) {
    return;
  }
  int delay = 10;
  // Read the pushbutton state; see pbRead() in launchpad.h
  int code = pbRead();
  //  s1 = 0,  s2 = 0,    m1 = 0,  m2 = 0;
  //  right 1, 2nd right, left 2, left 1
  switch (code) {
  case 1:
    uprintf("New Combination position %d value: %d\n\r", j + 1, knobArray[0]);
    blinkEntry();
    //Assigns new value to combo position
    passwordLock[j] = knobArray[0];
    j++;
    delay = 250; // debounce
    break;
  }
  // once all 4 numbers selected relock and send success indication
  if(j > 3) {
    unlocked = false;
    blinkReset();
  }
  // callback for next digit if not end
  else {
    schdCallback(checkNewPass, time + delay);
  }
  return;
}

// called once a correct combo is inputted
// transitions lock to unlocked state and listens
// for a new combo input selection (SW2 push)
// relocks lock after 30 seconds and outputs combo array to memory
// in case it was updated
void blinkCorrect(uint32_t time) {
  // only unlock on first call
  if(init_correct) {
    unlocked = true;
    init_correct = false;
  }
  if(setEnd) {
    endTime = time + 30000;
    setEnd = false;
  }
  // listens for SW2 push until SW2 is pushed
  if(canEnter) {
    int code = pbRead();
    if(code == 2) {
      canEnter = false;
      if(firstPress) {
        uprintf("Enter new combination:\n\r");
        firstPress = false;
      }
      j = 0;
      // shut off unlocked light
      ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
      schdCallback(checkNewPass, time + 250);
    }
  }
  // if still unlocked && 30 seconds hasn't elapsed callback
  if(time < endTime && unlocked) {
    schdCallback(blinkCorrect, time + 10);
    return;
  }
  //resets all needed variables to begin a new unlock sequence next time
  //prints out new combination
  else {
    setEnd = true;
    canEnter = true;
    firstPress = true;
    j = 0;
    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
    unlocked = false;
    uprintf("Lock is now locked.\n\r");
    // combination output for debug purposes
    uprintf("Current Combination is: ");
    int b = 0;
    while(b < 4) {
      uprintf("%d ", passwordLock[b]);
       ++b;
    }
    uprintf("\n\r");
    // write combo to memory
    EEPROMProgram(passwordLock, addr, sizeof(passwordLock));
  }
  return;
}

/* Event driven code for checking push button
*/
void checkPushButton(uint32_t time) {
  // wait until in locked state
  if(unlocked) {
    schdCallback(checkPushButton, time + 10);
    return;
  }
  // Read the pushbutton state; see pbRead() in launchpad.h
  int code = pbRead();
  uint32_t delay = 10;

  //  s1 = 0,  s2 = 0,    m1 = 0,  m2 = 0;
  //  right 1, 2nd right, left 2, left 1
  switch (code) {
  case 1:
    uprintf("Value selected: %d\n\r", knobArray[0]); // debug output
    //clear must be made before selection can be inputed
    if(!cleared) {
      uprintf("Input was not cleared.\n\r");
      blinkError();
      correct = true;
      i = 0;
      delay = 250;
      break;
    }
    uprintf("Combination position: %d\n\r", i + 1); // debug output
    //Compares current knobArray to stored password combination,
    if (knobArray[0] == passwordLock[i] && correct) {
      // if final number and all correct, unlock lock
      if(i == 3) {
        uprintf("Lock is now unlocked.\n\r");
        ledTurnOnOff(false /* red */, false /* blue */, true /* green */);
        init_correct = true;
        schdCallback(blinkCorrect, time + 250);
        i = 0;
        cleared = false;
        correct = true;
        delay = 250;
        schdCallback(checkPushButton, time + delay);
        return;
      }
      blinkEntry();
    }
    // update correctness tracker if incorrect, don't break current combo entry
    else {
      correct = false;
      //Only stop once full combo has been inputed, otherwise someone picking it could easily find the combo with
      //trial and error
      if(i >= 3) {
        uprintf("Incorrect combination entry.\n\r");
        i = 0;
        cleared = false;
        correct = true;
        blinkError();
        delay = 250;
        schdCallback(checkPushButton, time + delay);
        return;
      }
      blinkEntry();
    }
    //must be cleared again after number selected, next will be compared with next combo in stored password
    cleared = false;
    i++;
    delay = 250;
    break;
  }
  // SW2 press invalid in this function
  switch (code) {
    case 2:
      uprintf("Lock must be unlocked to set new combination.\n\r");
      delay = 250;
  }
  // runs for entirety of program
  schdCallback(checkPushButton, time + delay);
}


// reads current ras value, converts to proper range, and
// updates the display
void rasUpdate(uint32_t time)
{
  //Logic to check for proper combination turning
  if(i >= 2) {
    leftClear = false;
  }
  else {
    leftClear = true;
  }
  rasRead(knobArray); //function call for the array
  knobArray[0] = 31 - (knobArray[0] / 128); // conversion factor
  // check if clear has happened
  if(knobArray[0] == 0 && leftClear) {
    cleared = true;
  }
  if(knobArray[0] == 31 && !leftClear) {
    cleared = true;
  }
  // if it hasn't and incorrect movement taking place, reset current combo entry and signal an error
  if(leftClear && knobArray[0] > prev && !cleared && !unlocked || leftClear && knobArray[0] < prev && cleared && knobArray[0] != 0 && !unlocked) {
    uprintf("Not moving RAS in correct direction.\n\r"); // debug
    blinkError();
    i = 0;
    correct = true;
    cleared = false;
  }
  if(!leftClear && knobArray[0] < prev && !cleared && !unlocked || !leftClear && knobArray[0] > prev && cleared && knobArray[0] != 31 && !unlocked) {
    uprintf("Not moving RAS in correct direction.\n\r"); // debug
    blinkError();
    i = 0;
    correct = true;
    cleared = false;
  }
  // store current ras value to compare against next
  prev = knobArray[0];
  // Update clock and display, based on RAS
  seg7DigitUpdate(&seg7Display);
  seg7Display.d1 = knobArray[0] % 10;
  seg7Display.d2 = knobArray[0] / 10;
  // runs for entirety of program
  schdCallback(rasUpdate, time + 50);
}


int main(void)
{
  // initialize all peripherals
  lpInit();
  seg7Init();
  rasInit();
  epromInit();

  // set combo to one stored in memory
  EEPROMRead(passwordLock, addr, 16);

  // debug purposes
  uprintf("%d %d %d %d\n", passwordLock[0], passwordLock[1], passwordLock[2], passwordLock[3]);

  uprintf("%s\n\r", "digiLock\n\r");

  // Update the clock display
  seg7DigitUpdate(&seg7Display);
  // Schedule first callback for input reading functions
  schdCallback(checkPushButton, 1000);
  // Schedule the first callback events for LED flashing and push button checking.
  // Those trigger callback chains. The time unit is millisecond.
  schdCallback(rasUpdate, 1000);

  // Run the event scheduler to process callback events
  while (true) {
    schdExecute();
  }
}
