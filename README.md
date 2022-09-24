# digiLock
**Purpose:**     digitized locking mechanism resemblant of a combination lock

**Inputs:**      rotary angle sensor, two control buttons

**Outputs:**     one of 3 colors of LED in concept build, segmented LED display, lock/unlock signal in real-world implementation

**Functions:**   Input sequence read, combination comparison, lock/unlock, overwrite combination

digiLock is a conceptual application and has not been developed or tested with an actual locking mechanism

## Requirements
1. [CCS](https://www.ti.com/tool/CCSTUDIO)
2. [Tiva TM4C123GH6PM](https://www.ti.com/product/TM4C123GH6PM)
3. [Tiva TM4C SDK](https://www.ti.com/tool/SW-TM4C)
4. [Seeed Studio Grove Base](https://www.seeedstudio.com/Grove-Base-BoosterPack-p-2177.html)
5. [Seeed Studio Grove Rotary Angle Sensor](https://www.seeedstudio.com/Grove-Rotary-Angle-Sensor.html)
6. [Seeed Studio Grove 4 Digit Display](https://www.seeedstudio.com/Grove-4-Digit-Display.html)

## Installation
1. Connect the Tiva C Launchpad to the Grove Base
2. Connect the 4 digit display to port J10 on the Grove Base
3. Connect the RAS to port J11 on the Grove Base
4. Extract Project into CCS workspace, move all files from src and lib to root directory (can delete src and lib at this point)
5. Open the project source code in CCS
6. Right click on project and select Open Build Settings
7. Add *TivaWare Installation Path*\driverlib\ccs\Debug\driverlib.lib to Arm Linker files
8. Add *TivaWare Installation Path*\driverlib to Arm Linker Paths
9. Add *TivaWare Installation Path* to Compiler Include Options paths
10. Ensure "${CG_TOOL_ROOT}/lib" and "${CG_TOOL_ROOT}/include" are in Arm Linker Path, "libc.a" In Arm Linker Files, and that "${PROJECT_ROOT}" and "${CG_TOOL_ROOT}/include" are in the Compiler Include Options paths
11. Connect the Tiva Launchpad to the computer
12. Build and run the program


## Usage

#### Unlocking:
Since the rotary angle sensor does not have a full 360-degree range, the max and min values of the sensor are used to replicate the clearing mechanic between the first and 2nd digits of a physical combination lock. Also, a 4-digit combination is used instead of a 3 digit one. The ADC of the RAS is divided even into segments 0 – 31 for 32 total numbers (each section consists of 128 values), and all values stated hereon will be these interpreted values. To enter a combination, the RAS must first be at 0. The first number is found by moving the RAS right until the desired number is found. Pressing SW1 on the Launchpad selects number as input. After the first number, the RAS must be rotated back to 0 (cleared) before the 2nd number can be selected. After selecting the 2nd number, the RAS must be rotated to 31 (also cleared) before the 3rd number can be picked (note that leaving the RAS on 0 or 31 can still be used as combination numbers). The 3rd number is found by rotating the RAS left, after which it is cleared to 31 and then the 4th number is found the same way. When an input is taken, the LED on the board blinks blue. If all 4 inputted digits are correct, the LED will turn green for 30 seconds. If the digits are wrong or the input pattern wasn’t correct (inputting a digit before clearing if needed) the LED will blink red 5 times. 
#### Changing Combination:
If both the buttons on the board are pressed at the same time while the LED is on and green, the LED will turn off. A new combination will be inputted at this time. If the input pattern is correct (right, clear, right, clear, left, clear, left) then the LED will blink green 5 times. This indicates that a new combination has been saved for the lock. If the input pattern is incorrect, the LED will blink red 3 times and then turn green for 10 seconds, back into the “unlocked” state where the user could press the two buttons again to begin the combination changing sequence. If the input pattern is wrong a second time, the LED blinks red 5 times and will require the old combination to be put it to unlock and try again. This provides a (1/32)4 = 0.00009536743% chance of a combination being guessed, as well as no physical indications of the correct number being selected, which physical combination locks have via their locking pins.


## License


    digiLock is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    digiLock is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with MySoft. If not, see http://www.gnu.org/licenses/.
