/* 
 * File:   state_machine.h
 * Author: jsavo
 *
 * Created on October 31, 2023, 3:45 PM
 */

#ifndef STATE_MACHINE_H
#define	STATE_MACHINE_H

//State-Machine MACROs
#define LOCKED_STATE 0
#define INPUT2_STATE 1
#define INPUT3_STATE 2
#define INPUT_NUMBER_STATE 3
#define UNLOCKED_STATE 4
#define ERROR_STATE 5
#define NEW_PASS_STATE 6
#define SECRET_STATE 7

//*****************************************************************************************
// --- Variables ---
extern int initial_value_PB1, initial_value_PB2, initial_value_PB3;   
extern int PB1, PB2, PB3; 
extern int delay_on;                                 
extern char set_input1, set_input2, set_input3;  
extern uint16_t set_number;                                    
extern uint16_t count;
extern char Timer2_blink;
extern int state;                                
extern int correct_input, incorrect_input;  

//*****************************************************************************************
//--- Functionality Functions ---
void delay_ms();
void IO_check();
void IO_clear();
void Flag_clear();
void debounce();
void clear_terminal();
void clear_line();
void UART_check(char compare_input);

//*****************************************************************************************
//--- State-Machine Functions ---
void locked_state();
void input2_state();
void input3_state();
void input_number_state();
void unlocked_state();
void error_state();
void new_pass_state();
void secret_state();


#endif	/* STATE_MACHINE_H */

