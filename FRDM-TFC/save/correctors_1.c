#include "TFC/TFC.h"
#include "FSL/FSL.h"

/// // Global variables
// Position of the middle of the line (255 = not found)
extern Line line;

// Speed corrector variables
float speedCoefLow;
float speedCoefHigh;
float speedDutyLeft;
float speedDutyRight;

// Angle corrector variables
long double angle_sum;
long double angle_derivative;
long double angle_order;

float angle_integrator_factor;
float angle_derivator_factor;
float angle_proportional_factor;

// Save the last error angle
char last_error;

void init_correctors(void){
	// Speed corrector variables
	speedCoefLow = (SPEED_DUTY_MAX - SPEED_DUTY_MIN)/(64 - SPEED_OFFSET_LOW);
	speedCoefHigh = (SPEED_DUTY_MAX - SPEED_DUTY_MIN)/(64 - SPEED_OFFSET_HIGH);
	speedDutyLeft = 0;
	speedDutyRight = 0;

	// Angle corrector variables
	angle_sum = 0;
	angle_derivative = 0;
	angle_order = 0;

	angle_integrator_factor = ANGLE_INTEGRATOR_FACTOR;
	angle_derivator_factor = ANGLE_DERIVATOR_FACTOR;
	angle_proportional_factor = ANGLE_PROPORTIONNAL_FACTOR;

	// Save the last error angle
	last_error = 0;
}

/**
 * Speed manager function (compute the duty cycle of the motors
 * Correcteur proportionnel
 */
void speed_manager(void){
	if(line.found == 1){
		//Computing the duty value for a line detected on the right
		if((line.position - 64) > 0){
			speedDutyLeft  = SPEED_DUTY_MAX - ((char)line.position - 64)*speedCoefHigh;
			speedDutyRight = SPEED_DUTY_MAX - ((char)line.position - 64)*speedCoefLow;
		}
		//Computing the duty value for a line detected on the left
		else{
			speedDutyLeft  = SPEED_DUTY_MAX - (64 - (char)line.position)*speedCoefLow;
			speedDutyRight = SPEED_DUTY_MAX - (64 - (char)line.position)*speedCoefHigh;
		}
	}
	//Checking if the line has been found by the camera
	else{
		speedDutyLeft = SPEED_NOLINE;
		speedDutyRight = SPEED_NOLINE;
	}

	/* set the pwm for Motors */
	setMotorPWM(speedDutyLeft,speedDutyRight); //security is enable
}

/**
 * Angle manager function
 * Correcteur proportionnel integral
 * 
 * \param error : the mesured error to correct using this corrector
 * 
 * @consigne : valeur de l'angle
 * @mesure : l'angle
 * @commande : PWM pour obtenir l'angle
 * 
 * /!\ Time between each call of the function is a factor of the resulting angle,
 * 		it's strongly recommended to use a timer to manage the time between each
 * 		call.
 * 
 */

void angle_manager(void){
	if(line.found == 1){
	angle_corrector((char)line.position - 64 , 1 , 0 , 1); //angle_corrector((char)line.position - 64);

	}
	
	else{
		if(line.last_direction == right){
			setServoAngle(ANGLE_NO_LINE);
			//angle_sum = ANGLE_INTEGRATOR_SUM_MAX;
		}
		else if(line.last_direction == middle){
			setServoAngle(0);
			//angle_sum = 0;
		}
		else{
			setServoAngle(-ANGLE_NO_LINE);
			//angle_sum = ANGLE_INTEGRATOR_SUM_MIN;
		}
	}
}

void angle_corrector(char error , uint8 proportional , uint8 integral , uint8 derivative){
	/*
	if (speedDutyLeft <= 10 ) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_10;
	}else if (speedDutyLeft > 10 && speedDutyLeft <= 20) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_20;
	}else if (speedDutyLeft > 20 && speedDutyLeft <= 30) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_30;
	}else if (speedDutyLeft > 30 && speedDutyLeft <= 40) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_40;
	}else if (speedDutyLeft > 40 && speedDutyLeft <= 50) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_50;
	}else if (speedDutyLeft > 50 && speedDutyLeft <= 60) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_60;
	}else if (speedDutyLeft > 60 && speedDutyLeft <= 70) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_70;
	}else if (speedDutyLeft > 70 && speedDutyLeft <= 80) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_80;
	}else if (speedDutyLeft > 80 && speedDutyLeft <= 90) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_90;
	}else if (speedDutyLeft > 90 && speedDutyLeft <= 100) {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_100;
	}else {
		angle_proportional_factor = ANGLE_PROPORTIONNAL_FACTOR;
	}
	 */	

	angle_order = 0;

	if (proportional == 1) {
		angle_order += angle_proportional_factor * (error);
	}

	if (integral == 1) {
		angle_sum += ANGLE_SUM_FACTOR * (error);

		//Checking if it's still in the max and min defined
		if(angle_sum > ANGLE_INTEGRATOR_SUM_MAX){
			angle_sum = ANGLE_INTEGRATOR_SUM_MAX;
		}
		else if(angle_sum <  ANGLE_INTEGRATOR_SUM_MIN){
			angle_sum = ANGLE_INTEGRATOR_SUM_MIN;
		}

		angle_order += angle_integrator_factor * angle_sum;
	}

	if (derivative == 1) {
		angle_derivative = error - last_error;

		//Remember the last position (error)
		last_error = error;
		
		angle_order += angle_derivator_factor * angle_derivative;
	}

	setServoAngle(angle_order);

}
