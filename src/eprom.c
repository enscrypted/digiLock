// Allows access to the board's EEPROM storage, which is used for
// the lock combination

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/eeprom.h>
#include "Util/launchpad.h"
#include "eprom.h"

void epromInit()
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();
}
