/*
 * speed.c
 *
 *  Created on: Apr 29, 2014
 *      Author: Axel
 */

#include "TFC/TFC.h"
#include "FSL/FSL.h"

extern Event event;

extern Corrector corrector;

void init_speed(void){

	/* STRUCTURE */
	event.brake = FALSE;
	event.straight = FALSE;
	event.finishline = FALSE;
}


void speed_fsm(void){

	static state_e state = Follow_full;
	//static uint8 slowBlocked = TRUE;

	switch(state){

	case Follow_full :
		ihm_led(1,-1,-1,-1);
		corrector.angle.proportional = SPEED_PROPORTIONAL_MAX;
		corrector.angle.derivative = SPEED_DERIVATIVE_MAX;

		FAST;

		if (event.brake) {
			state = Brake;
			//TIMER_BRAKE_START
			lowPowerTimer_setTime(TIME_BRAKE);
		}
		break;

	case Brake :
		ihm_led(-1,1,-1,-1);
		//Attention pwm a -100 + voir correcteur des frein 
		corrector.angle.proportional = SPEED_PROPORTIONAL_MAX;
		corrector.angle.derivative = SPEED_DERIVATIVE_MAX;

		BRAKE;

		if (TIMER_FINISH) {
			TIMER_CLEAR_FLAG;
			lowPowerTimer_setTime(TIME_LOCK);
			state = Follow_slow;
		}
		break;


	case Follow_slow :
		//slowBlocked = TRUE;
		ihm_led(-1,-1,1,-1);
		corrector.angle.proportional = SPEED_PROPORTIONAL_MIN;
		corrector.angle.derivative = SPEED_DERIVATIVE_MIN;

		SLOW;
		/*
		if(TIMER_FINISH){
			slowBlocked = FALSE;
		}
		*/

		if (event.straight && TIMER_FINISH) {
			state = Acceleration;
			//TIMER_ACCELERATION_START
			lowPowerTimer_setTime(TIME_ACCELERATION);
		}
		break;

	case Acceleration :
		ihm_led(-1,-1,-1,1);
		corrector.angle.proportional = SPEED_PROPORTIONAL_ACC;
		corrector.angle.derivative = SPEED_DERIVATIVE_ACC;

		ACCELERATION;

		if (TIMER_FINISH) {
			TIMER_CLEAR_FLAG;
			state = Follow_full;
		}


		if (event.brake) {
			//Clear current timer
			TIMER_RESET_TIME;
			state = Brake;
			//TIMER_BRAKE_START
			lowPowerTimer_setTime(TIME_BRAKE);
		}	
		break;

	default :
		state = Follow_full;
		break;


	}

}

