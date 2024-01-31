/*
 * File:   Init.c
 * Author: jsavo
 *
 * Created on October 31, 2023, 3:29 PM
 */


#include "xc.h"
#include "Init.h"

//Clock Init
void CLKinit(char clock_freq){
    uint16_t COSCNOSC;
    
    if(clock_freq == 8){
        COSCNOSC = 0x77;
        CLKDIVbits.RCDIV = 0;
    }
    else if(clock_freq == 1){
        COSCNOSC = 0x77;
        CLKDIVbits.RCDIV = 3;
    }
    else if(clock_freq == 500){
        COSCNOSC = 0x66;
        CLKDIVbits.RCDIV = 0;
    }
    else if(clock_freq == 31){
        COSCNOSC = 0x55;
        CLKDIVbits.RCDIV = 0;
    }
        
    
    //Setting Fosc = 1Mhz
    SRbits.IPL = 7;                  //Disable Interrupts
    //CLKDIVbits.RCDIV = 3;            //Divide by 8 (8MHz / 8 = 1MHz)
    __builtin_write_OSCCONH(0x77);   //Set 8Mhz clock  [ 0x66 for 500kHZ | 0x77 for 8Mhz ]
    __builtin_write_OSCCONL(0x01);   //Set CLK change bit 1 
    while(OSCCONbits.OSWEN==1){}     //Stay until CLK changes (CLK change bit goes LOW)
    SRbits.IPL = 0;                  //Enable interrupts
    
    //REFOCONbits.ROEN = 1;            //Enables Clock output on pin 18 for debugging
}

//Timer Inits
void TIMERinit(){
    //Timer2 init
    T2CONbits.T32 = 0;      //Set as 16-bit timer
    T2CONbits.TCS = 0;      //Use Internal clock
    T2CONbits.TCKPS = 0b10; //Prescaler of 64  (128us / 7812.5Hz with Clk at 1MHz)
    T2CONbits.TGATE = 0;    //Disable gating function
    T2CONbits.TSIDL = 0;    //Set to continuous mode
    //T2CONbits.TON = 1;    //Turn on timer
    
    //Timer 1 init
    T1CONbits.TCS = 0;      //Use Internal clock
    T1CONbits.TCKPS = 1;    //Setting pre-scaler of 8
    T1CONbits.TGATE = 0;    //Disable gating function
    T1CONbits.TSIDL = 1;    //Set to continuous mode
    T1CONbits.TSYNC = 0;    //Don;t Sync to external clk
}

//Interrupt Inits
void INTERRUPTinit(){
    INTCON1bits.NSTDIS = 0;    //Nesting interrupts enabled
    
    //Timer 2 interrupt
    IEC0bits.T2IE = 1;  //Enable Timer2 interrupt
    IPC1bits.T2IP = 2;  //Set Second Lowest priority
    IFS0bits.T2IF = 0;  //Setting int flag to 0
    
    //Timer 1 interrupt
    IEC0bits.T1IE = 1;
    IPC0bits.T1IP = 1;
    IFS0bits.T1IF = 0;
    
    //Change Notification interrupt
    CNEN1bits.CN0IE = 1;    //Enable CN interrupt for RA4
    CNEN1bits.CN1IE = 1;    //Enable CN interrupt for RB4
    CNEN2bits.CN30IE = 1;   //Enable CN interrupt for RA2
    IEC1bits.CNIE = 1;      //Enable CN interrupt
    IFS1bits.CNIF = 0;      //Clear int flag
    
    //ADC Interrupt 
//    IEC0bits.AD1IE = 1;     //Enable ADC Conversion Interrupt
//    IPC3bits.AD1IP = 7;     //Highest Priority Interrupt
//    IFS0bits.AD1IF = 0;     //Clear Interrupt Flag
    
}

//IO Inits
void IOinit(){
    //Analog Pin Functionality 
    AD1PCFG = 0xffff;       //Disabling Analog functionality of pins
    AD1PCFGbits.PCFG5 = 0;  //Set AN5 (pin 8) for Analog Read functions (ADC)
    
    //PORT I/O
    TRISA |= 0x0014;        //Set RA2 & RA4 to input
    TRISBbits.TRISB4 = 1;   //Set RB4 to input
    TRISBbits.TRISB7 = 0;   //Set RB7 to output
    TRISBbits.TRISB8 = 0;   //Set RB8 to output
    TRISBbits.TRISB9 = 0;   //Set RB9 to output
    
    //Enabling Pull-Ups for RA2, RA4, and RB4
    CNPU1 = 0x0003;   //CN0 & CN1
    CNPU2 = 0x4000;   //CN30
    
}

//ADC Init
void ADCinit(){
    //AD1CON1
    AD1CON1bits.ADSIDL = 1;      //Set to run continuously
    AD1CON1bits.FORM = 0b00;     //Set ADC output form to INTEGER
    AD1CON1bits.SSRC = 0b111;    //End Sampling & start conversion automatically (internally counter set in AD1CON3)
    AD1CON1bits.ASAM = 0;        //Sample when SAMP bit is 1 ;; bit = 1 Sample immediately after end of last conversion
    
    //AD1CON2 (Volt Ref, Int. Trigger, BUF, MUX select)
    AD1CON2bits.VCFG = 0b000;   //Set Refernece Voltage to VDD(3V3) and Vss(GND) of MCU
    AD1CON2bits.CSCNA = 0;
    //AD1CON2bits.SMPI = 0b0000;  //Trigger Interrupt at end of every ADC conversion
    AD1CON2bits.BUFM = 0;       //Set Buffer to one 16-word buffer
    AD1CON2bits.ALTS = 0;       //Use MUX-A settings
    
    //AD1CON3 (ADC Clock, Sampling Time)
    AD1CON3bits.ADRC = 0;         //ADC clock uses system clock (Fosc/2 = 500KHz)
    AD1CON3bits.ADCS =  0b111111;  //Use System CLK FOR SAMPLE TIME (Fad = (Fosc/2)*(1/64) = 7812.5kHz)  -> pg177 of Datasheet
    AD1CON3bits.SAMC = 0b11111;    //Set to Slowest Sampling time [Sampling time = 31*(1/Tad)]
        //Total ADC time per sampling = Sampling time + (12*Tad) ;  (conversion takes 12 clock cycles)
    
    //AD1CHS (MUX-A settings -> Positive and Negative Input of ADC)
    AD1CHSbits.CH0NA = 0;       //Negative input of ADC = Vr- (GND of MCU - set in AD1CON2.VCFG)
    AD1CHSbits.CH0SA = 0b0101;  //Positive input of ADC = AN5 (pin 8)
    
    //AD1CSSL (Analog pin selection)
    //D1CSSLbits.CSSL5 = 1;      //Enable input scanning for AN5
    AD1PCFGbits.PCFG5 = 0;
    
}