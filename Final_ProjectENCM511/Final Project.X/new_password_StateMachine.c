/*
 * File:   state_machine.c
 * Author: jsavo
 *
 * Created on October 29, 2023, 6:44 PM
 */

#include "xc.h"
#include "state_machine.h"
#include "Interrupts.h"
#include "string.h"
#include "uart.h"
#include "ADC.h"
#include "new_password_StateMachine.h"


//**************************************************
// --- Variables ---
char password_state = INPUT1_STATE, User_input_flag = 0;  //State variable for password state-machine
char temp_input1, temp_input2, temp_input3, temp_number;  //Temporary storing for new user input password
//**************************************************
// --- Functions ---

//Resets the state-machine for next time entering NEW_PASS_STATE
void reset_NewPass_StateMachine(){
    password_state = INPUT1_STATE;
    User_input_flag = 0;
}

//***************************************************
// --- State machine ---

void new_pass_state_machine(){

    //New password state-machine
    while(state == NEW_PASS_STATE){
        switch(password_state){
            case INPUT1_STATE:  //First User input
                new_character_1();
                break;
                
            case INPUT2_STATE:  //Second User input
                new_charcter_2();
                break;
                
            case INPUT3_STATE:
                new_charcter_3();
                break;
                
            case INPUT_NUM_STATE:
                new_number();
                break;
                
            case SET_NEW_PASS_STATE:
                set_password();
                break;
                
            case PASS_SET_CANCEL_STATE:
                canceled_password_change();
                break;

        }
    }
}

//********************************************************
//  --- State-Machine State Functions ---

void new_character_1(){
    clear_line();   //Clear line of terminal
    Disp2String("Input 1st character: ");
                
    while(password_state == INPUT1_STATE){
        //IO and UART Interrupt checks
        if(CN_flag)
            IO_check(); 
        if(RXFlag){                 //If Rx interrupt triggered
            temp_input1 = RecvUartChar(); 
            User_input_flag = 1;
        }
                    
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            password_state = PASS_SET_CANCEL_STATE;
        else if(User_input_flag)    //User input -> next state of new_password
            password_state = INPUT2_STATE;
        }
    IO_clear();
    Flag_clear();
    User_input_flag = 0;            //Clear User_input_flag 
}

void new_charcter_2(){
    clear_line();   //Clear line of terminal
    Disp2String("Input 2nd character: ");
                
    while(password_state == INPUT2_STATE){
        //IO and UART Interrupt checks
        if(CN_flag)
            IO_check(); 
        if(RXFlag){                 //If Rx interrupt triggered
            temp_input2 = RecvUartChar(); 
            User_input_flag = 1;
        }
                    
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            password_state = PASS_SET_CANCEL_STATE;
        if(PB2){                    //PB2 pressed -> Go into secret state
            state = SECRET_STATE;
            IO_clear();
            Flag_clear();
            User_input_flag = 0;
            return;
        }
        else if(User_input_flag)    //User input -> next state of new_password
            password_state = INPUT3_STATE;
        }
    IO_clear();
    Flag_clear();
    User_input_flag = 0;            //Clear User_input_flag
}

void new_charcter_3(){
    clear_line();   //Clear line of terminal
    Disp2String("Input 3rd character: ");
                
    while(password_state == INPUT3_STATE){
        //IO and UART Interrupt checks
        if(CN_flag)
            IO_check(); 
        if(RXFlag){                 //If Rx interrupt triggered
            temp_input3 = RecvUartChar(); 
            User_input_flag = 1;
        }
                    
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            password_state = PASS_SET_CANCEL_STATE;
        else if(User_input_flag)    //User input -> next state of new_password
            password_state = INPUT_NUM_STATE;
        }
    IO_clear();
    Flag_clear();
    User_input_flag = 0;            //Clear User_input_flag
}

void new_number(){
    clear_line();           //Clear line of terminal
    Disp2String("Input new number - Press PB2 to confirm selection: \n\r");
    T2CONbits.TON = 1;      //Turn ON TIMER2
    delay_ms(500);          //Set ADC sampling to 0.5s (2Hz)
    
    while(password_state == INPUT_NUM_STATE){
        //ADC sampling and display
        ADC_sample();               //Take an ADC_sample
        ADC_display();              //Output ADC bar-graph and decimal value
        Idle();
        clear_line();               //Clear line for next ADC_display() call
        
        //IO Interrupt Check
        if(CN_flag)
            IO_check(); 
        
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            password_state = PASS_SET_CANCEL_STATE;
        else if(PB2){
            temp_number = scale_value(ADC_value);
            password_state = SET_NEW_PASS_STATE;
        }
        
    }
    IO_clear();
    Flag_clear();
    User_input_flag = 0;            //Clear User_input_flag
}

void set_password(){
    clear_terminal();
    Disp2String("Successfully set new password...\n\rReturning to locked state");
    T2CONbits.TON = 1;      //Turn ON TIMER2
    delay_ms(3000);         //Set a Delay for 3s
    
    //Change set password variables to stored temporary variables
    set_input1 = temp_input1;
    set_input2 = temp_input2;
    set_input3 = temp_input3;
    set_number = temp_number;
    
    Idle();
    //Change main state-machine state to locked
    state = LOCKED_STATE;
}

void canceled_password_change(){
    clear_terminal();
    Disp2String("PB1 Pressed - Canceled password change returning to locked state");
    LATBbits.LATB8 = 0;     //Turn OFF LED
    T2CONbits.TON = 1;      //Turn ON TIMER2
    delay_ms(3000);         //Set a Delay for 3s
    
   // Idle();
    Idle();
    //Change main state-machine state to locked
    state = LOCKED_STATE;
}