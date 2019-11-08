/*
 * photodiode.c
 *
 *  Created on: Mar 8, 2019
 *      Author: swallen
 */

#include "adcs.h"

ADC_HandleTypeDef adc1Handle = {.Instance = ADC1};

void initADCs() {
    __HAL_RCC_ADC1_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/**ADC GPIO Configuration
	PA6     ------> VBATT_MEAS
	PA7     ------> IR_BEACON_OUT
	*/

	ADC_ChannelConfTypeDef sConfig = {0};
	adc1Handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	adc1Handle.Init.Resolution = ADC_RESOLUTION_12B;
	adc1Handle.Init.ScanConvMode = DISABLE;
	adc1Handle.Init.ContinuousConvMode = ENABLE;
	adc1Handle.Init.DiscontinuousConvMode = DISABLE;
	adc1Handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	adc1Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	adc1Handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adc1Handle.Init.NbrOfConversion = 1;
	adc1Handle.Init.DMAContinuousRequests = DISABLE;
	adc1Handle.Init.EOCSelection = DISABLE;
	HAL_ADC_Init(&adc1Handle);

	//we set up to read the beacon by default
	sConfig.Channel = ADC_CHANNEL_7;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&adc1Handle, &sConfig);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_ADC_Start(&adc1Handle);
}

uint16_t getVBatt() {
	//TODO
	return 0;
}

uint16_t getPhotoDiode() {
	return (uint16_t) ADC1->DR;
}

#define BEACON_STATE_WAIT 0
#define BEACON_STATE_EDGE 1
#define BEACON_STATE_RECIEVED 2
uint8_t beaconState = BEACON_STATE_WAIT;

uint32_t edgeTime = 0;
uint32_t beaconLastTime = 0;
uint32_t usPerDeg = 0;

#define MAX_USPERDEG 600000UL
#define MIN_BEACON_THRESH 400

void runBeacon() {
	uint16_t beaconMeas = getPhotoDiode();

	switch(beaconState) {
	case BEACON_STATE_WAIT :
		//check for new readings
		if(beaconMeas > MIN_BEACON_THRESH) {
			beaconState = BEACON_STATE_EDGE;
			edgeTime = getMicroseconds();
		}
		break;
	case BEACON_STATE_EDGE:
		//double check the packet
		if(beaconMeas > MIN_BEACON_THRESH) { //if the beacon is still high
			if(getMicroseconds() - edgeTime > 2000UL) {//and 0.2ms has passed
				usPerDeg = (edgeTime - beaconLastTime) / 360UL;//we report a real edge
				beaconLastTime = edgeTime;
				if(usPerDeg >= MAX_USPERDEG) usPerDeg = 0;
				beaconState = BEACON_STATE_RECIEVED;
			}
		} else {
			beaconState = BEACON_STATE_WAIT;
		}
		break;

	case BEACON_STATE_RECIEVED :
		//check for the current packet to end. Wait at least the amount of time for the bot to complete a half revolution at 1000RPM
		if(beaconMeas < MIN_BEACON_THRESH/2) {
			beaconState = BEACON_STATE_WAIT;
		}
		break;
	default :
		beaconState = BEACON_STATE_WAIT;
		break;
	}

	//reset the algorthim if it's been too long since we've seen an edge
	if(getMicroseconds() - beaconLastTime > 500000UL) usPerDeg = 0;
}

int16_t getBeaconAngle() {
	//return (HAL_GetTick()/100) % 360;
	if(usPerDeg != 0) {
		return (int16_t) ((((getMicroseconds() - beaconLastTime) / usPerDeg) + 180) % 360);
	}
	return 0;
}

uint32_t getUsPerDeg() {
	return usPerDeg;
}
