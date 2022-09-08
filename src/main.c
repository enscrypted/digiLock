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
#include "launchpad.h"
#include "seg7.h"
#include "ras.h"
#include "seg7digit.h"
#include <math.h>
#include <time.h>


// The initial state of the 7-segment display: "00 00" with colon off
seg7Display_t seg7Display = {
    0, 0, 0, 0, 0
};

//global variables. knobArray has to be used during switch press, moved appropriately
//bools used and updated in multiple functions
int passwordLock[] = { 19, 5, 20, 25 };
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
int j = 0;
typedef enum {On,Off}OnOff_t;

void blinkError() {
    int j = 0;
    while(j < 5) {
        ledTurnOnOff(true /* red */, false /* blue */, false /* green */);
        waitUs(100000); // wait is in microseconds, 100k us = 100 ms
        ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
        waitUs(100000);
        j++;
    }
    return;
}
void blinkEntry() {
    ledTurnOnOff(false /* red */, true /* blue */, false /* green */);
    waitUs(100000);
    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
    return;
}

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
       // compare to combination number
       //uprintf("SW1 was pushed.\n\r"); Used to check which state of device is printing this statement.
       uprintf("New Combination position %d value: %d\n\r", j + 1, knobArray[0]);
       blinkEntry();
       //Compares current knobArray to stored password combination,
       passwordLock[j] = knobArray[0];
       j++;
       delay = 250; // debounce
       break;
   }
   //
    if(j > 3) {
      j = 0;
    }
    else {
        schdCallback(checkNewPass, time + delay);
    }
    return;
}


void blinkCorrect(uint32_t time) {
    unlocked = true;
    ledTurnOnOff(false /* red */, false /* blue */, true /* green */);
    if(setEnd) {
      endTime = time + 30000;
      setEnd = false;
    }
    if(canEnter) {
      int code = pbRead();
      if(code == 2) {
          if(firstPress) {
              uprintf("Enter new combination:\n\r");
              firstPress = false;
          }
          schdCallback(checkNewPass, time + 250);
      }
    }
    if(time < endTime) {
        if(unlocked) {
            schdCallback(blinkCorrect, time + 10);
            return;
        }
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
      uprintf("Current Combination is: ");
      int b = 0;
      while(b < 4) {
       uprintf("%d ", passwordLock[b]);
       ++b;
      }
      uprintf("\n\r");
    }
    return;
}

/* Event driven code for checking push button
*/
void checkPushButton(uint32_t time) {
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
       // compare to combination number
       //uprintf("SW1 was pushed.\n\r"); Used to check if SW1 worked as intended.
       uprintf("Value selected: %d\n\r", knobArray[0]);
        //clear must be made before selection can be inputted
        if(!cleared) {
            uprintf("Input was not cleared.\n\r");
            blinkError();
            correct = true;
            i = 0;
            delay = 250;
            break;
        }
       uprintf("Combination position: %d\n\r", i + 1);
       //Compares current knobArray to stored password combination, 
       if (knobArray[0] == passwordLock[i] && correct) { // && correct taken out since unused
           //uprintf("Correct value selected.\n\r");
           if(i == 3) {
               uprintf("Lock is now unlocked.\n\r");
               schdCallback(blinkCorrect, time + 10);
               //uprintf("Lock is locked\n");
               i = 0;
               cleared = false;
               correct = true;
               delay = 250;
               schdCallback(checkPushButton, time + delay);
               return;
           }
           blinkEntry();
       }
       else {
           correct = false;
           //uprintf("Incorrect value selected or has been before.\n\r");
           //Only stop once full combo has been inputted, otherwise someone picking it could easily find the combo with
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
   switch (code) {
   case 2:
       uprintf("Lock must be unlocked to set new combination.\n\r");
       delay = 250;
   }
   schdCallback(checkPushButton, time + delay);
}



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
    /*
    uprintf("%d (0) ", knobArray[0]);
    */
    knobArray[0] = 31 - (knobArray[0] / 128);
    if(knobArray[0] == 0 && leftClear) {
        cleared = true;
    }
    if(knobArray[0] == 31 && !leftClear) {
        cleared = true;
    }
    if(leftClear && knobArray[0] > prev && !cleared && !unlocked || leftClear && knobArray[0] < prev && cleared && knobArray[0] != 0 && !unlocked) {
        uprintf("Not moving RAS in correct direction.\n\r");
        blinkError();
        i = 0;
        correct = true;
        cleared = false;
    }
    if(!leftClear && knobArray[0] < prev && !cleared && !unlocked || !leftClear && knobArray[0] > prev && cleared && knobArray[0] != 31 && !unlocked) {
        uprintf("Not moving RAS in correct direction.\n\r");
        blinkError();
        i = 0;
        correct = true;
        cleared = false;
    }
    prev = knobArray[0];
    //uint16_t leftDig = 99 - knobArray[1] * 100 / 4096;
    //uint16_t rightDig = 99 - knobArray[0] * 100 / 4096;
    // Update clock and display, based on RAS
    seg7DigitUpdate(&seg7Display);
    seg7Display.d1 = knobArray[0] % 10;
    seg7Display.d2 = knobArray[0] / 10;
    seg7Display.d3 = 10; //allows blank slots for left 2 digits
    seg7Display.d4 = 10;


    schdCallback(rasUpdate, time + 50);
}


int main(void)


{
    lpInit();
    seg7Init();
    rasInit();

    uprintf("%s\n\r", "Lab Project: 4 Digit Combination Lock\n\r");
    //uprintf("Combination is: %d - %d - %d - %d\n\r", passwordLock[0], passwordLock[1], passwordLock[2], passwordLock[3]);    // Would not be displayed real use case

    // Update the clock display
    seg7DigitUpdate(&seg7Display);
    schdCallback(checkPushButton, 1000);
    // Schedule the first callback events for LED flashing and push button checking.
    // Those trigger callback chains. The time unit is millisecond.
    schdCallback(rasUpdate, 1000);

    // Run the event scheduler to process callback events
    while (true) {
        schdExecute();
    }
}
