/* 
 * File:   new_password_StateMachine.h
 * Author: jsavo
 *
 * Created on November 30, 2023, 7:58 PM
 */

#ifndef NEW_PASSWORD_STATEMACHINE_H
#define	NEW_PASSWORD_STATEMACHINE_H

//**************************************************
// --- MACROS ---
#define INPUT1_STATE 0
#define INPUT2_STATE 1
#define INPUT3_STATE 2
#define INPUT_NUM_STATE 3
#define SET_NEW_PASS_STATE 4
#define PASS_SET_CANCEL_STATE 5

//**************************************************
// --- Variables ---
extern char password_state, User_input_flag;             //State variable for password state-machine
extern char temp_input1, temp_input2, temp_input3, temp_number;  //Temporary storing for new user input password

//**************************************************
// --- Functions ---
void reset_NewPass_StateMachine();
//***************************************************
// --- State machine ---
void new_pass_state_machine();
//********************************************************
//  --- State-Machine State Functions ---
void new_character_1();
void new_character_2();
void new_character_3();
void new_number();
void set_password();
void canceled_password_change();

#endif	/* NEW_PASSWORD_STATEMACHINE_H */

