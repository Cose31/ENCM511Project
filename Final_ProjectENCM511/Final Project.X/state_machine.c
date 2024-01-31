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
#include "memory_game.h"

//*****************************************************************************************
// --- Variables ---

//IO variables 
int PB1 = 0, PB2 = 0, PB3 = 0;                                            //Button variables
int delay_on, initial_value_PB1, initial_value_PB2, initial_value_PB3;    //debounce variables

//Password Variables
char set_input1 = 'a', set_input2 = 'b', set_input3 = 'c' ; //3-character passcode (initally set as "abc")
uint16_t set_number = 0;                                    //0-9 ADC input        (initally set as "0")

//Timer Variables
uint16_t count = 0;
char Timer2_blink = 1;

//Flags and state variables
int state = LOCKED_STATE;                //state value
int correct_input = 0, incorrect_input = 0;   //Input check flags


//*****************************************************************************************
//--- Functionality Functions ---

//Delay for TIMER2 Interrupt trigger
void delay_ms(unsigned int ms){    
    PR2 = ms / 0.128;   // PR2 = (desired time in ms) / ([(TIMER2 Prescaler) /(Fosc/2)]*1000)
    TMR2 = 0;           //Reseting count when delay is changed
}

//Checks digital input of RA2
void IO_check(){
    debounce();                 //debounce Button pressed
    PB1 = !initial_value_PB1;
    PB2 = !initial_value_PB2; 
    PB3 = !initial_value_PB3;
    
    CN_flag = 0;                //Clear Interrupt flag
}

//Button Debounce function for button inputs (RA2)
void debounce(){
    //TIMER1 delay select
    PR1 = 2500;         //Setting 40ms delay count for debounce -> PR1 = (Time in Secs)*(Fosc/2)/(TIMER1 Prescaler)
    
    //Take initial value of inputs
    initial_value_PB1 = PORTAbits.RA2;
    initial_value_PB2 = PORTBbits.RB4;
    initial_value_PB3 = PORTAbits.RA4;
    
    //Delay for button debounce - PR1 set in TIMERinit
    delay_on = 1;       //Set delay_on 
    T1CONbits.TON = 1;  //Turn on TIMER1 - set to trigger at 40ms
    while(delay_on){}   //TIMER1 interrupt sets delay_on = 0
    
    //Checking if initial value is equal to value after delay
    if (PORTAbits.RA2 == initial_value_PB1 &
        PORTBbits.RB4 == initial_value_PB2 &
        PORTAbits.RA4 == initial_value_PB3)    //If true leave debounce()
        return;
    else                                       
        debounce();     //If false redo debounce()
}

//Clear IO variables
void IO_clear(){
    PB1 = 0;
    PB2 = 0;
    PB3 = 0;
    ADC_value = 0;
}

//Clear ALL flags used in state-machine
void Flag_clear(){
    RXFlag = 0;             //UART_RX flag
    CN_flag = 0;            //CN_flag
    correct_input = 0;      //Correct input flag for switching state
    incorrect_input = 0;    //Incorrect input flag for switching state
    count = 0;              //Time-out count for switching state
}

//Clears terminal and returns cursor to top left of screen
void clear_terminal(){
    CLKinit(1); 
    /* This ANSI escape sequence clears the whole screen and takes us home*/
        XmitUART2(0x1b,1);
        XmitUART2('[',1);
        XmitUART2('2',1);
        XmitUART2('J',1);
        Disp2String("\033[H");
}

//Clear the current line the cursor is on in the terminal
void clear_line(){
    CLKinit(1); 
    Disp2String("\033[2K\r");
}

//Used to take user input via UART and check to given input
void UART_check(char compare_input){
    char UART_input;                //Variable used to store UART_RX input
    
    UART_input = RecvUartChar();    //Take 1-char user input
    
    //Check if UART input is correct
    if(UART_input == compare_input) //Set Correct_bit if user inputs correct character
        correct_input = 1;
    else                            //Set Incorrect_bit if user inputs incorrect character
        incorrect_input = 1;
}

//Gives a message to user before returning to locked after PB1 press
void return_to_lock_state(){
    state = LOCKED_STATE;       //Set state to LOCKED_STATE
    clear_terminal();           //Clear Terminal
    Disp2String("Pressed PB1 returning to locked state");
    LATBbits.LATB8 = 0;         //Turn OFF LED
    T2CONbits.TON = 1;          //Turn ON Timer
    delay_ms(3000);             //Set 3s delay
    Idle();                     //Return after delay or button press
    Idle();  
}


//*****************************************************************************************
//--- State-Machine Functions ---

void locked_state(){
    //Initial state code
    clear_terminal();           //Clears terminal
    Disp2String("Welcome! Enter 1st input: ");    //Print to terminal
    T2CONbits.TON = 0;          //Turn off Timer
    LATBbits.LATB8 = 1;         //Turn ON LED
    
    //While in this state code
    while(state == LOCKED_STATE){
        Idle();
        
        //IO and UART check after interrupt trigger
        if(RXFlag){                 //If Rx interrupt triggered
            UART_check(set_input1); }
        
        //Change State Logic
        if(correct_input)
            state = INPUT2_STATE;
        else if(incorrect_input)
            state = ERROR_STATE;        
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void input2_state(){
    //Initial state code
    clear_terminal();           //Clears terminal
    Disp2String("First input correct! \n\rEnter 2nd input: ");    //Print to terminal
    LATBbits.LATB8 = 1;         //Turn ON LED
    T2CONbits.TON = 1;          //Turn ON Timer
    delay_ms(500);              //Set blinking to 0.5s
    
    //While in this state code
    while(state == INPUT2_STATE){
        Idle();
        //IO and UART check after interrupt trigger
        if(CN_flag)
            IO_check(); 
        if(RXFlag)                 //If Rx interrupt triggered
            UART_check(set_input2); 
        if(count > 49)              //If count based off TIMER2 is 50 -> ERROR_STATE          
            incorrect_input = 1;
        
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            return_to_lock_state();
        else if(incorrect_input)    //Incorrect input or time-out -> Error state
            state = ERROR_STATE;
        else if(correct_input)      //Correct input -> next state (INPUT3_state)
            state = INPUT3_STATE;
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void input3_state(){
    //Initial state code
    clear_terminal();           //Clears terminal
    Disp2String("Second input correct! \n\rEnter 3rd input: ");    //Print to terminal
    LATBbits.LATB8 = 1;         //Turn ON LED
    T2CONbits.TON = 1;          //Turn ON Timer
    delay_ms(400);              //Set blinking to 0.4s
    
    //While in this state code
    while(state == INPUT3_STATE){
        Idle();
        //IO and UART check after interrupt trigger
        if(CN_flag)
            IO_check(); 
        if(RXFlag)                  //If Rx interrupt triggered
            UART_check(set_input3);
        if(count > 49)              //If count based off TIMER2 is 50 -> ERROR_STATE          
            incorrect_input = 1;
        
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            return_to_lock_state();
        else if(incorrect_input)    //Incorrect input or time-out -> Error state
            state = ERROR_STATE;
        else if(correct_input)      //Correct input -> next state (INPUT3_state)
            state = INPUT_NUMBER_STATE;
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear();  
}

//implement ADC 0-9 numbering, Bar display, and ADC with PB2 push button
void input_number_state(){
    //Initial state code
    clear_terminal();           //Clears terminal
    Disp2String("Third input correct! \n\rEnter number from 0-9 and press PB2 to confirm input: \n");    //Print to terminal
    LATBbits.LATB8 = 1;         //Turn ON LED
    T2CONbits.TON = 1;          //Turn ON Timer
    delay_ms(300);              //Set blinking to 0.3s
    
    //While in this state code
    while(state == INPUT_NUMBER_STATE){
        
        //IO and UART check after interrupt trigger
        if(CN_flag){
            IO_check(); 
            //Check if PB2 is pressed -> if true compare user number input to password input
            if(PB2 & (scaled_ADC_value == set_number))
                correct_input = 1;
            else
                incorrect_input = 1;       
        }
        if(count > 49)              //If count based off TIMER2 is 50 -> ERROR_STATE          
            incorrect_input = 1;
        
        //ADC sampling and display
        ADC_sample();               //Take an ADC_sample
        ADC_display();              //Output ADC bar-graph and decimal value
        Idle();
        clear_line();               //Clear line for next ADC_display() call
        
        //State Logic
        if(PB1)                     //PB1 pressed -> Locked state
            return_to_lock_state();
        else if(incorrect_input)    //Incorrect input or time-out -> Error state
            state = ERROR_STATE;
        else if(correct_input)      //Correct input -> next state (INPUT3_state)
            state = UNLOCKED_STATE;
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear(); 
}

void unlocked_state(){
    //Initial state code
    clear_terminal();
    Disp2String("Unlocked!\n\r-Press PB1 to re-lock device\n\r-Press PB2 to set new password");    //Print to terminal
    T2CONbits.TON = 0;          //Turn ON Timer
    LATBbits.LATB8 = 0;         //Turn ON LED
    
    //While in this state code
    while(state == UNLOCKED_STATE){
        Idle();
        //IO and UART check after interrupt trigger
        if(CN_flag){
            IO_check(); }
        
        //State Logic
        if(PB1)
            return_to_lock_state();
        if(PB2)
            state = NEW_PASS_STATE;
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear(); 
}

void error_state(){
    //Initial state code
    clear_terminal();
    Disp2String("Incorrect input! Please press PB2 to return");    //Print to terminal
    T2CONbits.TON = 0;          //Turn ON Timer
    LATBbits.LATB8 = 0;         //Turn ON LED
    
    while(state == ERROR_STATE){
        Idle();
        //IO and UART check after interrupt trigger
        if(CN_flag){
            IO_check(); }
        
        //State Logic
        if(PB2)
            state = LOCKED_STATE;

    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear(); 
}

void new_pass_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Setting new password...\n");    //Print to terminal
    T2CONbits.TON = 0;          //Turn ON Timer
    LATBbits.LATB8 = 0;         //Turn ON LED
    
    //State-machine
    reset_NewPass_StateMachine();   //resets state-machine for use
    new_pass_state_machine();       //Enter New Password state-machine
    
    //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void secret_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Secret Mode");    //Print to terminal
    Timer2_blink = 0;              //Turn off timer 2 blinking
    
    //State-machine
    reset_memory_game();        //Reset all variables for game
    MemoryGame_StateMachine();
    
    //Executed code when leaving state
    Timer2_blink = 1;
    IO_clear();
    Flag_clear();
}




