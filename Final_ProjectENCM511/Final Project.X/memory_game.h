/* 
 * File:   memory_game.h
 * Author: jsavo
 *
 * Created on December 3, 2023, 1:24 AM
 */

#ifndef MEMORY_GAME_H
#define	MEMORY_GAME_H

//*****************************************************************
//--- MACROS ---
#define SECRET_STATE_ENTER 0
#define EASY_STATE 1
#define MEDIUM_STATE 2
#define HARD_STATE 3
#define PLAYING_STATE 4
#define LOSS_STATE 5
#define WIN_STATE 6
#define GAME_CANCEL_STATE 7

//*****************************************************************
//--- Variables ---
extern uint8_t randomized_array [12];    
extern int seed;
extern uint8_t number_of_stages, stage_count;  
extern uint8_t game_state;           
extern uint16_t blink_time;                       

extern uint8_t level_scaled_value;  

//*****************************************************************
//--- Functions ---

void reset_memory_game();
void randomize_array(uint8_t number_range);
void number_of_stage_display();
void level_stage_display();
uint8_t level_select_scale(int ten_bit_input);
void blink_LED(uint8_t LED_select);
void read_user_input(uint8_t correct_answer);
void two_second_delay();

//*****************************************************************
//--- State-Machine ---
void MemoryGame_StateMachine();

//*****************************************************************
//--- State-Machine Functions ---
void secret_state_enter();
void easy_state();
void medium_state();
void hard_state();
void playing_state();
void loss_state();
void win_state();
void game_cancel_state();

#endif	/* MEMORY_GAME_H */

