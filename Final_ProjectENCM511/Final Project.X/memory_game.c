/*?
 * File:   state_machine.c
 * Author: jsavo
 *
 * Created on December 2nd, 2023, 6:44 PM
 */

#include "xc.h"
#include "Init.h"
#include "state_machine.h"
#include "Interrupts.h"
#include "string.h"
#include "uart.h"
#include "ADC.h"
#include "new_password_StateMachine.h"
#include "memory_game.h"
#include <stdlib.h>

//*****************************************************************
//--- Variables ---
uint8_t randomized_array [12];               //Array of size 12 for game LED outputs and user checking
int seed = 0;
uint8_t number_of_stages = 0, stage_count = 0;  //Number of stages of game to run through -> set by user, & stage tracker
uint8_t game_state = SECRET_STATE_ENTER;              //State choosing variable
uint16_t blink_time = 0;                        //Global blink time variable

uint8_t level_scaled_value;                     //Value scaled from 1-3 for game mode selection????  ?????l?????€??D??????5?9   ???Ü???9????
        
//*****************************************************************
//--- Functions ---

//Reset memory game for next time entering
void reset_memory_game(){
    number_of_stages = 0;
    stage_count = 0;
    game_state = SECRET_STATE_ENTER;
    blink_time = 0;
    level_scaled_value = 0;
}

//Randomizes "randomized_array" with values from 0-1 or 0-2 depending on selection
void randomize_array(uint8_t number_range){
    uint8_t modulus_value;  //Modulus value for determining inputs between 0-2
    
    srand(seed);     //Seeding random number generator
    
    //Select if modulus value is 2 or 3 depending on number of LEDs being used(2 or 3)
    if(number_range == 2)
        modulus_value = 2;
    else
        modulus_value = 3;
    
    //For loop for replacing all the LED values
    for(int i = 0; i != 13; i++){
        randomized_array[i] = rand() % modulus_value;
    }
}

//Sets the number of stages from user input, and displays the value (from 3-12)
void number_of_stage_display(){ 
    CLKinit(1);                                       //Set CLK to 1MHz
    //ADC scaling and displaying decimal
    number_of_stages = scale_value(ADC_value) + 3;    //Divides 2^10 by 103 + 3 to scale from 3-12
    //Disp2Dec(scaled_ADC_value);
    if(number_of_stages < 10){
        XmitUART2(' ',1);
        XmitUART2(number_of_stages + 0x30,1);
    }
    else if (number_of_stages > 9){
        XmitUART2(' ',1);
        XmitUART2('1',1);
        XmitUART2((number_of_stages - 10) + 0x30,1);
    }
    CLKinit(31);                                    //Set CLK to 31Khz
}

void level_stage_display(){
    CLKinit(1);                                       //Set CLK to 1MHz
    //ADC scaling for difficulty selection
    level_scaled_value = level_select_scale(ADC_value);
    
    //Display the current selected mode based of placement of ADC
    if(level_scaled_value == 1)
        Disp2String("Easy Mode");
    else if(level_scaled_value == 2)
        Disp2String("Medium Mode");
    else if(level_scaled_value == 3)
        Disp2String("Hard Mode");
    CLKinit(31);                                    //Set CLK to 31Khz
}

//Scales 10-bit ADC value to integer ranging from 1-3
uint8_t level_select_scale(int ten_bit_input){
    return (ten_bit_input / 342) + 1;      //Divides 2^10 by 341 + 1 to scale from 1-3
}

//Blinks the chosen LED(1,2 or 3) for the amount of time determined by PR1 (TIMER1)
void blink_LED(uint8_t LED_select){
    uint16_t LED_off_time = 15000;
    
    //LED OFF Delay - Same delay method as used in debounce function
    PR1 = LED_off_time;                           //Set TIMER1 for blink time -> set in last state
    delay_on = 1;
    T1CONbits.TON = 1;
    while(delay_on){}
    
    
    //Turn on chosen LED
    if(LED_select == 0)
        LATBbits.LATB7 = 1; //Turn LED_1 ON
    else if(LED_select == 1)
        LATBbits.LATB8 = 1; //Turn LED_2 ON
    else if(LED_select == 2)
        LATBbits.LATB9 = 1; //Turn LED_3 ON
    
    //LED ON delay - Same delay method as used in debounce function
    PR1 = blink_time;                           //Set TIMER1 for blink time -> set in last state
    delay_on = 1;
    T1CONbits.TON = 1;
    while(delay_on){}
    
    //Turn off LEDS
    LATB = 0x0000; //Turn off all LEDS   
    
}

void read_user_input(uint8_t correct_answer){
    //Stay in loop until user input
    while(1){
        Idle();
        //User input check
        if(CN_flag){
            IO_check();
            
            //Clear flags and IO vars. if double input occurs 
            if(!PB1 & !PB2 & !PB3){
                IO_clear();
                Flag_clear();
                read_user_input(correct_answer);
            }
        
            //Check if user-input is correct
            if(PB1 & (correct_answer == 0))
                break;
            else if(PB2 & (correct_answer == 1))
                break;
            else if(PB3 & (correct_answer == 2))
                break;
            else{
                incorrect_input = 1;
                break;
            }
        }
    }
}

//Delays code for 2s given no other interrupts
void two_second_delay(){
    T2CONbits.TON = 1;          //Turn ON Timer
    LATBbits.LATB8 = 0;         //Turn OFF LED
    delay_ms(2000);             //Sets delay of 2s - If you find this go to line 225 and go right...
    Idle();
    Idle();
    T2CONbits.TON = 0;          //Turn OFF Timer
}

//*****************************************************************
//--- State-Machine ---

void MemoryGame_StateMachine(){
    while(state == SECRET_STATE){
        switch(game_state){
            case SECRET_STATE_ENTER:
                secret_state_enter();
                break;
            case EASY_STATE:
                easy_state();
                break;
            case MEDIUM_STATE:
                medium_state();
                break;
            case HARD_STATE:
                hard_state();
                break;
            case PLAYING_STATE:
                playing_state();
                break;
            case LOSS_STATE:
                loss_state();
                break;
            case WIN_STATE:
                win_state();
                break;
            case GAME_CANCEL_STATE:
                game_cancel_state();
                break;
        }
    }
}

//*****************************************************************
//--- State-Machine Functions ---

void secret_state_enter(){
    //Initial state code
    reset_memory_game();        //Reset all variables for game
    clear_terminal();           //Clear terminal
    Disp2String("Secret Mode \n\rThis is a memory game, LEDs will light up in a specific order"
            " once they are done you must press the corresponding buttons in the same order to win the game\n\n\r"
            "Choose difficulty level - Press PB2 to confirm selection:\n\r");    //Print to terminal
    T2CONbits.TON = 1;          //Turn ON Timer
    delay_ms(250);             //ADC sampling r
    LATBbits.LATB8 = 0;         //Turn OFF LED
    
    //Using Timer1 to seed random number gen
    CLKinit(31);            //Set Clock to low-power mode (Fosc = 31Khz)
    PR1 = 62500;
    IEC0bits.T1IE = 0;      //Disable TIMER1 Interrupt
    T1CONbits.TON = 1;      //Turn Timer 1 ON
    
    //While in state code
    while(game_state == SECRET_STATE_ENTER){
        ADC_sample();                       //Sample ADC
        level_stage_display();              //Display the current selected difficulty mode
        Idle();                             //Delay here until TIMER2 interrupt trigger -> set by delay_ms()                                                                                                                     ... you dumb ;p
        clear_line();                       //Clear current line
        
        //IO_Check
        if(CN_flag){
            //Seeding code
            seed = TMR1;                    //Seed seed to whatever TMR1 is at time of user triggered interrupt
            T1CONbits.TON = 0;      //Turn Timer 1 ON
            IEC0bits.T1IE = 1;      //Enable TIMER1 Interrupt
            TMR1 = 0;               //Reset TIMER1 count
            //IO check
            IO_check();
        }
        
        //State Logic
        if(PB1)                             //PB1 pressed
            game_state = GAME_CANCEL_STATE;
        else if(level_scaled_value == 1 & PB2)   //level value is 1 & PB2 pressed
            game_state = EASY_STATE;
        else if(level_scaled_value == 2 & PB2)   //level value is 2 & PB2 pressed
            game_state = MEDIUM_STATE;
        else if(level_scaled_value == 3 & PB2)   //level value is 3 & PB2 pressed
            game_state = HARD_STATE;
    }
    //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void easy_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Easy Mode\n\r Select number of stages to complete - Press PB2 to confirm selection:\n\r");
    randomize_array(2);          //Randomizes "randomized_array" with values from 0-1 (2 LEDs are used)
    LATBbits.LATB8 = 0;         //Turn OFF LED
    
    //While in state code
    while(game_state == EASY_STATE){
        ADC_sample();                       //Sample ADC - still at 2Hz from last state
        number_of_stage_display();          //Display the current number of stages selected and set global var. for # of stages
        Idle();                             //Delay here until TIMER2 interrupt trigger -> set by delay_ms()
        clear_line();
        
        if(CN_flag)
            IO_check();
        
        //State Logic
        if(PB1)                             //PB1 pressed
            game_state = GAME_CANCEL_STATE;
        else if(PB2)
            game_state = PLAYING_STATE;     
    }
    //Executed code when leaving state
    blink_time = 62500;    //Set TIMER1 delay for 0.5s LED flashes for game
    IO_clear();
    Flag_clear();
}

void medium_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Medium Mode\n\rSelect number of stages to complete - Press PB2 to confirm selection:\n\r");
    randomize_array(3);         //Randomizes "randomized_array" with values from 0-2 (3 LEDs are used)
    LATBbits.LATB8 = 0;         //Turn OFF LED
    
    //While in state code
    while(game_state == MEDIUM_STATE){
        ADC_sample();                       //Sample ADC - still at 2Hz from last state
        number_of_stage_display();          //Display the current number of stages selected and set global var. for # of stages
        Idle();                             //Delay here until TIMER2 interrupt trigger -> set by delay_ms()
        clear_line();
        
        if(CN_flag)
            IO_check();
        
        //State Logic
        if(PB1)                             //PB1 pressed
            game_state = GAME_CANCEL_STATE;
        else if(PB2)
            game_state = PLAYING_STATE;     
    }
    //Executed code when leaving state
    blink_time = 62500;    //Set TIMER1 delay for 0.5s LED flashes for game
    IO_clear();
    Flag_clear();
}

void hard_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Easy Mode\n\rSelect number of stages to complete - Press PB2 to confirm selection:\n\r");
    randomize_array(3);          //Randomizes "randomized_array" with values from 0-2 (3 LEDs are used)
    LATBbits.LATB8 = 0;         //Turn OFF LED
    
    //While in state code
    while(game_state == HARD_STATE){
        ADC_sample();                       //Sample ADC - still at 2Hz from last state
        number_of_stage_display();          //Display the current number of stages selected and set global var. for # of stages
        Idle();                             //Delay here until TIMER2 interrupt trigger -> set by delay_ms()
        clear_line();
        
        if(CN_flag)
            IO_check();
        
        //State Logic
        if(PB1)                             //PB1 pressed
            game_state = GAME_CANCEL_STATE;
        else if(PB2)
            game_state = PLAYING_STATE;     
    }
    //Executed code when leaving state
    blink_time = 31250;    //Set TIMER1 delay for 0.25s LED flashes for game
    IO_clear();
    Flag_clear();
}

void playing_state(){
    CLKinit(1);                 //Set Fosc = 1Mhz
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Game starting in 2 seconds... \n\rWatch closely!");
    two_second_delay();         //Delay before starting blinking and clearing terminal
    clear_terminal();
    Disp2String("Now!");
    
    //While in state code
    while(game_state == PLAYING_STATE){
        //Stay in loop for number of stages chosen by user -> Increases pattern by 1 each repetition of loop
        while(stage_count != number_of_stages){
        //---- LED BLINKING ----
            //Loop for blinking LED
            for(uint8_t i = 0; i != stage_count + 1; i++)
                blink_LED(randomized_array[i]);         //Blink LED in order chosen by randomized array
        
        // ---- USER-INPUT READING ---
 
            //Loop for checking user-inputs
            for(uint8_t i = 0; i != stage_count + 1; i++){
                IO_clear();                                 //Clear IO variables before checking user-inputs  
                Flag_clear();                               //Clear Flag vars before checking user-inputs
                read_user_input(randomized_array[i]);   //Check if user input match the order of randomized array  
                if(incorrect_input == 1){               //return from function if incorrect input
                    game_state = LOSS_STATE;
                    IO_clear();
                    Flag_clear();
                    return;
                }
            }
        
        //---- END OF STAGE CODE ----
            stage_count ++;                             //Increase stage_count by 1
            clear_terminal();
            Disp2String("Good Job! Next stage starting in 2 seconds...");
            two_second_delay();                         //Delay 2s before clearing and staring next stage
            clear_terminal();
            Disp2String("Now!");
        }
        //Executed code when leaving state
        game_state = WIN_STATE;         //Enter WIN_STATE if made it out of loop
        IO_clear();
        Flag_clear();
    }  
}

void loss_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("Input incorrect squence: \n\r-Press PB1 to return to locked state\n\r"
            "-Press PB2 to return to difficulty selection");
    
    //While in state code
    while(game_state == LOSS_STATE){
        //IO check
        if(CN_flag)
            IO_check();
        
        //State Logic
        if(PB1){                        //If PB1 pressed
            return_to_lock_state();     //return to lock state message
            IO_clear();
            Flag_clear();
            return;
        }
        else if(PB2)
            game_state = SECRET_STATE_ENTER;
    }
     //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void win_state(){
    //Initial state code
    clear_terminal();           //Clear terminal
    Disp2String("You Won!!! \n\r-Press PB1 to return to locked state\n\r"
            "-Press PB2 to return to difficulty selection");
    
    //While in state code
    while(game_state == WIN_STATE){
        //IO check
        if(CN_flag)
            IO_check();
        
        //State Logic
        if(PB1){                        //If PB1 pressed
            return_to_lock_state();     //return to lock state message
            state = LOCKED_STATE;
            IO_clear();
            Flag_clear();
            return;
        }
        else if(PB2)
            game_state = SECRET_STATE_ENTER;
    }
     //Executed code when leaving state
    IO_clear();
    Flag_clear();
}

void game_cancel_state(){
    clear_terminal();
    Disp2String("PB1 Pressed - Canceled game returning to locked state");
    LATBbits.LATB8 = 0;     //Turn OFF LED
    T2CONbits.TON = 1;      //Turn ON TIMER2
    delay_ms(3000);         //Set a Delay for 3s
    
   // Idle();
    Idle();
    //Change main state-machine state to locked
    state = LOCKED_STATE;
}