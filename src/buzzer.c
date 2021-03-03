/*
 * buzzer.c
 *
 *  Created on: 10/09/2015
 *      Author: IanB
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "buzzer.h"

/*
Buzzer connected to PC0

TCC0:1

CH0CTRL
CH0MUX


CCExEN
OCx

Px0
OC0A
OCALS

SMT-1028-S-R Buzzer, resonant at 2830Hz
*/

char BUZZER=OFF;
char ticks=0;
char buzzerCount;
ISR(TCC0_OVF_vect)
{
	if (BUZZER)
		PORTC.OUTTGL = 1 << buzzerPin;

}


ISR(TCC0_CCA_vect)
{

}


void InitBuzzer(void)
{
  /*in frequency waveform generation
   * fFRQ = fclkper/(2N(CCA + 1))
   *
   * N = prescaler
   *
   * PER = fclk/(fout*2N) - 1
   *
   *     = 18432000/(2830 * 2 * 1) - 1
   *     = 3256
   *
   * In PWM mode
   * fFRQ = fclkper/(N(PER + 1))
   * 8KHz
   *     = 18432000/(8000 * 1) - 1
   *     = 2303
   *
   * 24KHz
   *     = 18432000/(8000 * 1) - 1
   *     = 767
   *
   */

  TCC0.CTRLA = TC_CLKSEL_OFF_gc;
  TCC0.CTRLB = (0 << TC0_CCDEN_bp | 0 << TC0_CCCEN_bp | 0 << TC0_CCBEN_bp | 1 << TC0_CCAEN_bp |
		   TC_WGMODE_NORMAL_gc /*TC_WGMODE_FRQ_gc*/ /*TC_WGMODE_SS_gc*/ );
  TCC0.CTRLC = (0 << TC0_CMPD_bp | 0 << TC0_CMPC_bp | 0 << TC0_CMPB_bp | 0 << TC0_CMPA_bp);
  TCC0.CTRLD = (TC_EVACT_OFF_gc | 0 << TC0_EVDLY_bp | TC_EVSEL_OFF_gc);
  TCC0.CTRLE = 0x00; /* Byte mode = normal */
  //TCC0.CTRLF = 0;

  TCC0.PER = 3256 ;
  TCC0.CCABUF = 0;

  TCC0.CNT = 0;

  TCC0.INTCTRLA = TC_ERRINTLVL_OFF_gc | TC_OVFINTLVL_LO_gc;
  TCC0.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_LO_gc;
  TCC0.INTFLAGS = TC0_CCAIF_bm | TC0_OVFIF_bm;

  TCC0.CTRLA = TC_CLKSEL_DIV1_gc;

  PORTC.DIRSET = 0x01;
  IntTimer();
}


/*
 * TCD0 Timer use for beeps
 * set TCD0.PER for different value for different time interval.
 * TCD0.PER =1800 for 100ms time intercal
 * should call IntTimer before calling any beep method
 *
 */
void IntTimer(void){
	TCD0.CTRLA=TC_CLKSEL_OFF_gc;
	TCD0.CTRLB=TC_WGMODE_NORMAL_gc;		// counter in normal operation
	TCD0.INTCTRLA=TC_OVFINTLVL_HI_gc;
	TCD0.PER = 1800; //18000 for 1 second
	TCD0.CTRLA=TC_CLKSEL_DIV1024_gc;

}


ISR(TCD0_OVF_vect){

	if(buzzerCount>0){
		if(BUZZER){
			BUZZER=OFF;
			buzzerCount--;
		}
		else{
			BUZZER=ON;
		}
	}
	else{
		TCD0.CTRLA=TC_CLKSEL_OFF_gc;
		TCD0.PER = 1800; //500ms
		TCD0.CTRLA=TC_CLKSEL_DIV1024_gc;
	}

}
void beep(void){
	buzzerCount=1;
}

void beep_twice(void)
{
	buzzerCount=2;
}

void beep_thrice(void)
{
	buzzerCount=3;
}

void beep_long(void)
{

	TCD0.CTRLA=TC_CLKSEL_OFF_gc;
	TCD0.PER = 9000; //500ms
	TCD0.CTRLA=TC_CLKSEL_DIV1024_gc;
	beep();

}
void beep_error(void){
	beep_long();
	buzzerCount=3;
}

void setBuzzer(byte state){
	switch (state){
	case BUZZER_OFF:
		BUZZER=OFF;
		break;
	case BUZZER_BEEP_ONCE:
		beep();
		break;
	case BUZZER_BEETTWICE:
		beep_twice();
		break;
	case BUZZER_BEEP_THRICE:
		beep_thrice();
		break;
	case BUZZER_BEEP_LONG:
		beep_long();
		break;
	case BUZZER_BEEP_ERROR:
		beep_error();
		break;

	}

}
