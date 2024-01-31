/* 
 * File:   Interrupts.h
 * Author: jsavo
 *
 * Created on October 31, 2023, 4:03 PM
 */

#ifndef INTERRUPTS_H
#define	INTERRUPTS_H

extern int CN_flag;

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void);

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void);

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void);

#endif	/* INTERRUPTS_H */

