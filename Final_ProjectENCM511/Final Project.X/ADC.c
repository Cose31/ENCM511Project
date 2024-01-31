/*
 * File:   state_machine.c
 * Author: jsavo
 *
 * Created on October 29, 2023, 6:44 PM
 */

#include "xc.h"
#include "uart.h"
#include "ADC.h"

//***********************************************************************
// --- Variables ---
uint16_t ADC_value = 0;         //ADC variables  
uint8_t scaled_ADC_value;       //Scaled ADC value  

//***********************************************************************
// --- Functions ---

////ADC sampling & conversion function
void ADC_sample(){
    AD1CON1bits.ADON = 1;           //Turn ON ADC
    AD1CON1bits.SAMP = 1;           //Start Sampling

    while(AD1CON1bits.DONE == 0){}  //Stay here until conversion is done
    AD1CON1bits.ADON = 0;           //Turn OFF ADC
    AD1CON1bits.SAMP = 0;
    ADC_value = ADC1BUF0;           //Set global variable "ADC_value" to converted value
}

//Creates a Bar-graph, scales ADC input to 0-9, and displays scaled value
void ADC_display(){ 
    bar_visual(ADC_value);  //Creates bar_visual
    
    //ADC scaling and displaying decimal
    scaled_ADC_value = scale_value(ADC_value);    //Divides 2^10 by 103 to scale from 0-9
    //Disp2Dec(scaled_ADC_value);
    XmitUART2(' ',1);
    XmitUART2(scaled_ADC_value + 0x30,1);
}

//Scales 10-bit ADC value to integer ranging from 0-9
uint8_t scale_value(int ten_bit_input){
    return ten_bit_input / 103;      //Divides 2^10 by 103 to scale from 0-9
}

//Creates the Bar-graph display in the terminal based of ADC
void bar_visual(uint16_t voltage_value){
    uint16_t num_bars =  voltage_value * 9 /1023;  //[ADC_value * (Number of stars in graph)] / (2^10 - 1)
    Disp2String("\r");                              //Return to start of line
    for (int i = 0; i < num_bars ; i++) {
        Disp2String("*");
    }
}
