/*
 * File:   main.c
 * Author: jsavo
 *
 * Created on October 31, 2023, 3:27 PM
 */
#pragma config FNOSC = FRCDIV           // Oscillator Select (8 MHz FRC oscillator with divide-by-N (FRCDIV))
#pragma config IESO = ON                // Internal External Switch Over bit (Internal External Switchover mode enabled (Two-Speed Start-up enabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON           // CLKO Enable Configuration bit (CLKO output signal is active on the OSCO pin)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)
#pragma config ICS = PGx2    

#include "xc.h"
#include "Init.h"
#include "Interrupts.h"
#include "state_machine.h"
#include "uart.h"

int main(void) {
    CLKinit(1);      //CLK init
    TIMERinit();     //Timer init
    INTERRUPTinit(); //Interrupt init
    IOinit();        //IO init
    ADCinit();       //ADC init
    InitUART2();     //UART init
    
    while(1){
        switch(state){
            case LOCKED_STATE:
                locked_state();
                break;
            case INPUT2_STATE:
                input2_state();
                break;
            case INPUT3_STATE:
                input3_state();
                break;
            case INPUT_NUMBER_STATE:
                input_number_state();
                break;
            case UNLOCKED_STATE:
                unlocked_state();
                break;
            case ERROR_STATE:
                error_state();
                break;
            case NEW_PASS_STATE:
                new_pass_state();
                break;
            case SECRET_STATE:
                secret_state();
                break;
                
        }
    }
    return 0;
}
