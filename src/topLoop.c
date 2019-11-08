/*
 * topLoop.c
 *
 * this contains the main state machine logic for the bot
 *
 *  Created on: Mar 8, 2019
 *      Author: swallen
 */

#include "topLoop.h"

uint8_t mainState = 0;//we set to an invalid state so the loop is forced to properly set up in safe state

uint32_t jukeStartTime = 0;

uint32_t loopTime = 0;//in us

uint8_t stickSwitchDebounce = 0;

uint8_t led_on = 0;
uint32_t led_time = 0;

void loop() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//USER INITIALIZATION FUNCTIONS
	initTimers();
	initAnimations();
	initADCs();
	initSerial();
	initWatchdog();
	initMotors();
	initAccelerometer();
	//initLidar();

	//turnSourceOn();

	//INFINITE LOOP
	while(1) {
		feedWatchdog();
		loopTime = getMicroseconds();

		receiveSerial();

		//make sure we are still connected to the controller
		if(!isControlled() && mainState != STATE_SAFE){
			setState(STATE_SAFE);
		}

		//switch debouncing functions
		if(stickSwitchDebounce && !getStickSwitch()) stickSwitchDebounce = 0;

		switch(mainState) {
		case(STATE_SAFE):
			//This state is for fault conditions, and for when we don't have communication with the controller.
			if(isControlled()) setState(STATE_IDLE);
			break;

		case(STATE_IDLE):
			//this state is when we have communications with the controller, but it's telling us to stay disabled
			if(isEnabled()) {
				if(commandedThrottle() > 10) {
					setState(STATE_SPIN);
				} else {
					setState(STATE_DRIVE);
				}
			}

			if(getStickSwitch() && !stickSwitchDebounce) {
				stickSwitchDebounce = 1;
				setState(STATE_PREJUKE);
			}

			setMotorSpeed(MOTOR1, 0);//make damn sure the motor is stopped
			setMotorSpeed(MOTOR2, 0);
			runMotors();

			if((getMilliseconds() - led_time) > 1000) {
				if(led_on) {
					setSimpleLEDs(50, 50, 0);
					led_on = 0;
				} else if(!led_on) {
					setSimpleLEDs(100, 100, 0);
					led_on = 1;
				}
				led_time = getMilliseconds();
			}
			break;

		case(STATE_PREJUKE):
			//this state is like idle, but we are going to do a juke maneuver if we go to spin mode next
			if(isEnabled()) {
				if(commandedThrottle() > 10) {
					jukeStartTime = getMicroseconds();
					setState(STATE_JUKE);
				} else {
					setState(STATE_DRIVE);
				}
			}

			if(getStickSwitch() && !stickSwitchDebounce) {
				stickSwitchDebounce = 1;
				setState(STATE_IDLE);
			}

			setMotorSpeed(MOTOR1, 0);//make damn sure the motor is stopped
			setMotorSpeed(MOTOR2, 0);
			runMotors();

			if((getMilliseconds() - led_time) > 1000) {
				if(led_on) {
					setSimpleLEDs(50, 50, 0);
					led_on = 0;
				} else if(!led_on) {
					setSimpleLEDs(100, 100, 0);
					led_on = 1;
				}
				led_time = getMilliseconds();
			}
			break;

		case(STATE_JUKE):
			if(getMicroseconds() - jukeStartTime > 500000) {
				setState(STATE_SPIN);
			}

			runMotors();
			break;

		case(STATE_DRIVE):
			//this state drives the robot in standard arcade mode
			if(!isEnabled()) setState(STATE_IDLE);
			if(commandedThrottle() > 10) {
				setState(STATE_SPIN);
				continue;
			}

			setMotorSpeed(MOTOR1, -1*(getThumbX() - getThumbY()/2));
			setMotorSpeed(MOTOR2, getThumbX() + getThumbY()/2);
			runMotors();
			break;

		case(STATE_SPIN):
			//this state drives the robot in meltybrain mode
			if(!isEnabled()) setState(STATE_IDLE);
			if(commandedThrottle() <= 10) {
				setState(STATE_DRIVE);
				continue;
			}

			runHybridSensing();
			//runBeacon();

			//this is the master heading variable. If we want to change what sensor combinations are feeding us
			//our heading, we do that in the following line
			int16_t heading = getHybridAngle();
			//int16_t heading = getAccelAngle();
			//int16_t heading = getBeaconAngle();

			runPOVAnimation(heading);

			uint16_t speed = ((int16_t) (commandedThrottle())/2)*getDirSwitch();

			//first check if the melty throttle is high enough for translation
			if (getMeltyThrottle() > 10) {

				//calculate the distance between the current heading and the commanded direction
				int16_t diff = 180 - abs(abs(getMeltyAngle() - heading) - 180);

				int16_t deviation = 40*getDirSwitch();

				//now check if we are pointed more towards the commanded direction or the opposite
				if(diff < 90) {
				  //we are pointing towards the commanded heading, forward pulse
				  setMotorSpeed(MOTOR1, speed-deviation);
				  setMotorSpeed(MOTOR2, speed+deviation);
				} else {
				  //we are pointing opposite the commanded heading, reverse pulse
				  setMotorSpeed(MOTOR1, speed+deviation);
				  setMotorSpeed(MOTOR2, speed-deviation);
				}

				//top/bottom animation
				if(diff < 10) {
					setSimpleLEDs(0, 0, 100);
				} else if(heading < 180) {
					setSimpleLEDs(0, 100, 0);
				} else {
					setSimpleLEDs(100, 0, 0);
				}
			} else {
				//if we aren't translating, just run the motors at the throttle speed
				setMotorSpeed(MOTOR1, speed);
				setMotorSpeed(MOTOR2, speed);

				//top/bottom
				if(heading < 180) {
					setSimpleLEDs(0, 100, 0);
				} else {
					setSimpleLEDs(100, 0, 0);
				}
			}

			break;

		default:
			setState(STATE_SAFE);
			break;
		}

		if(mainState != STATE_SPIN) {
			runTimeAnimation();
		}
	}
}

/* setState()
 * arguments:
 * 	newState: the new state that you wish to transition the bot into
 * returns:
 *  nothing
 *
 *  Use this to change what state the bot is in. This function handles all relevant re-initialization
 *  for every state.
 */

void setState(uint8_t newState) {
	switch(newState) {
	default:


	case(STATE_SAFE):
		mainState = newState;
		setSimpleLEDs(100, 0, 0);
#if defined(MC_DRV8320) || defined(MC_ONESHOT125)
		disableMotors();
#endif
		break;


	case(STATE_IDLE):
		mainState = newState;
		setSimpleLEDs(100, 100, 0);
#ifdef MC_ONESHOT125
		setMotorSpeed2(MOTOR1, 0);
		setMotorSpeed2(MOTOR2, 0);
#endif

#ifdef MC_DRV8320
		setMotorBrake(MOTOR1, MOTOR_BRAKE_ON);
		setMotorBrake(MOTOR2, MOTOR_BRAKE_ON);
#endif
		break;


	case(STATE_PREJUKE):
		mainState = newState;
		setSimpleLEDs(100, 100, 0);
		break;


	case(STATE_JUKE):
		mainState = newState;
		setSimpleLEDs(0, 100, 0);

		setMotorSpeed(MOTOR1, 400);
		setMotorSpeed(MOTOR2, -400);

#ifdef MC_DRV8320
		setMotorBrake(MOTOR1, MOTOR_BRAKE_OFF);
		setMotorBrake(MOTOR2, MOTOR_BRAKE_OFF);
#endif

#if defined(MC_DRV8320) || defined(MC_ONESHOT125)
		enableMotors();
#endif
		break;


	case(STATE_DRIVE):
		mainState = newState;
		setSimpleLEDs(0, 100, 0);
#ifdef MC_DRV8320
		setMotorBrake(MOTOR1, MOTOR_BRAKE_OFF);
		setMotorBrake(MOTOR2, MOTOR_BRAKE_OFF);
#endif

#if defined(MC_DRV8320) || defined(MC_ONESHOT125)
		enableMotors();
#endif
		break;


	case(STATE_SPIN):
		mainState = newState;
#ifdef MC_DRV8320
		setMotorBrake(MOTOR1, MOTOR_BRAKE_OFF);
		setMotorBrake(MOTOR2, MOTOR_BRAKE_OFF);
#endif

#if defined(MC_DRV8320) || defined(MC_ONESHOT125)
		enableMotors();
#endif
		break;


	}

	//assuming the state change was successful, change the animation pattern
	if(newState == mainState) setAnimation(mainState);
}
