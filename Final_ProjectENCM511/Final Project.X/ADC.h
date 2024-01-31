/* 
 * File:   ADC.h
 * Author: jsavo
 *
 * Created on November 29, 2023, 12:40 PM
 */

#ifndef ADC_H
#define	ADC_H

extern uint16_t ADC_value;                           
extern uint8_t scaled_ADC_value;  

void ADC_sample();
void ADC_display();
uint8_t scale_value(int ten_bit_input);
void bar_visual(uint16_t voltage_value);
void display_Dec_value(int ten_bit_value);

#endif	/* ADC_H */

