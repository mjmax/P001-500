/*
 * buzzer.h
 *
 *  Created on: 11/09/2015
 *      Author: IanB
 */

#ifndef BUZZER_H_

#include <stdbool.h>
#include "sci.h"

#define buzzerPin 0
#define OFF 0
#define ON 1

typedef enum {BUZZER_OFF = 0, BUZZER_BEEP_ONCE, BUZZER_BEETTWICE, BUZZER_BEEP_THRICE, BUZZER_BEEP_LONG, BUZZER_BEEP_ERROR, BUZZER_BEEP_MAX}buzzerStatus_t;

extern char BUZZER;

void InitBuzzer(void);

void BuzzerOn(void);
void BuzzerOff(void);

void IntTimer(void);
void beep(void);
void beep_twice(void);
void beep_thrice(void);
void beep_long(void);
void beep_error(void);
void setBuzzer(byte state);
#endif /* BUZZER_H_ */
