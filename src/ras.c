/*
 *  RAS configuration
 */
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/adc.h>
#include "ras.h"


void rasInit() {


    //Enable the ADC0 module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //Configure the ADC0, sequencer #0
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

    //Configure the sequencer
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0,  ADC_CTL_CH7 | ADC_CTL_IE |  ADC_CTL_END );

    //Enable the sequence
    ADCSequenceEnable(ADC0_BASE, 0);


}
