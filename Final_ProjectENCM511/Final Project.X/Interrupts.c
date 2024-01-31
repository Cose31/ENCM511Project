/*
 * File:   Interrupts.c
 * Author: jsavo
 *
 * Created on October 29, 2023, 4:48 PM
 */


#include <p24F16KA102.h>

#include "xc.h"
#include "Interrupts.h"
#include "state_machine.h"

//******************************************************************
// --- Variables ---
int CN_flag = 0; 


//******************************************************************
// --- ISR ---

//TIMER1 Interrupts
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0;     //Clearing Interrupt flag
    delay_on = 0;          //Setting to break from debounce loop
    T1CONbits.TON = 0;     //Turn off TIMER1   
    TMR1 = 0;              //Clearing TIMER2 count
}

//TIMER2 Interrupts
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    if(Timer2_blink == 1)   //Toggle LED if true
        LATBbits.LATB8 = ~LATBbits.LATB8;   //Toggle LED]
    count = count + 1;                      //Increase count by 1
    TMR2 = 0;                               //Clearing TIMER2 count
    IFS0bits.T2IF = 0;                      //Clearing Interrupt flag
}

//Change Notification Interrupt (RA2, RA4, & RB4)
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    if(!PORTAbits.RA2 || !PORTBbits.RB4 || !PORTAbits.RA4)  //If PB1 pressed -> set flag
        CN_flag = 1;        //Setting Global variable "CN_flag"
      
    
    IFS1bits.CNIF = 0;  //Clearing interrupt flag
} 
    
//ADC Interrupt - NOT BEING USED)
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void){
    
    //ADC_value = ADC1BUF0; 
    IFS0bits.AD1IF = 0; //Clearing interrupt flag
}

