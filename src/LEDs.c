/*
 * LEDs.c
 *
 *  Created on: Feb 23, 2019
 *      Author: swallen
 */

#include "LEDs.h"
#include "params.h"

static SPI_HandleTypeDef spi = {.Instance = SPI3 };

#define SPI_FRAME_SIZE 33
uint8_t ledTransmitBuffer[SPI_FRAME_SIZE];

TIM_TypeDef * ledTimInst = TIM3;

void LED_init() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//SIMPLE RGB LED INIT

	//set up the gpio pins for tim3

	/**TIM3 GPIO Configuration
	PC6     ------> TIM1_CH1
	PC7     ------> TIM1_CH2
	PB0     ------> TIM1_CH3
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//set up the timer
	__HAL_RCC_TIM3_CLK_ENABLE();

	//disable the timer
	ledTimInst->CR1 = 0;

	//set up the prescaler to give the correct frequency
	//this needs to be higher than the "fusion frequency" for POV displays, low enough that the BJT's/LEDs can handle
	ledTimInst->PSC = (MAIN_CLOCK/2)-1;//gives 2MHz. Given a value of 100 in ARR, that means a 20KHz cycle

	//channels 1/2/3 are active when TIM_CNT < CCRx.
	ledTimInst->CCMR1 = 0x6060;
	ledTimInst->CCMR2 = 0x0060;

	//enable capture/compare outputs
	ledTimInst->CCER = 0x1111;

	//make sure thee CCR registers are zeroed. These will set the duty cycle of the color channel later
	ledTimInst->CCR1 = 0;
	ledTimInst->CCR2 = 0;
	ledTimInst->CCR3 = 0;

	//we reset the timer at 100. 100 is used to make the value more understandable, it's a percent!
	ledTimInst->ARR = 100;

	//enable the timer
	ledTimInst->CR1 = 0x0001;

	//APA102C INTERFACE INIT

	/**SPI3 GPIO Configuration
	PC10    ------> LED_CLK
	PB0     ------> LED_DI
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Alternate = GPIO_AF7_SPI3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//init the SPI peripheral
    __HAL_RCC_SPI3_CLK_ENABLE();

	spi.Init.Mode = SPI_MODE_MASTER;
	spi.Init.Direction = SPI_DIRECTION_2LINES;
	spi.Init.DataSize = SPI_DATASIZE_8BIT;
	spi.Init.CLKPolarity = SPI_POLARITY_LOW;
	spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	spi.Init.NSS = SPI_NSS_SOFT;
	spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi.Init.TIMode = SPI_TIMODE_DISABLE;
	spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	spi.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&spi) != HAL_OK)
	{
		Error_Handler();
	}

	//set up the transmit buffer
	for(int i=0;i<SPI_FRAME_SIZE-1;i++) ledTransmitBuffer[i] = 0;//first four bytes are start frame, clear the rest of the frames too
	for(int i=0;i<7;i++) ledTransmitBuffer[i*4+4] = 0xFF;//set the three required bits for each led frame, plus the global brightness
	for(int i=7*4+4;i<SPI_FRAME_SIZE;i++) ledTransmitBuffer[i] = 0xFF;//set the end frames

}

void setSimpleLEDs(uint8_t r, uint8_t g, uint8_t b) {
	ledTimInst->CCR1 = r;
	ledTimInst->CCR2 = g;
	ledTimInst->CCR3 = b;
}

void updateLEDs(Frame_P f) {
	//map the frame data into the buffer
	for(int i=0;i<7;i++) {
		ledTransmitBuffer[i*4+5] = f->led[i].blue;
		ledTransmitBuffer[i*4+6] = f->led[i].green;
		ledTransmitBuffer[i*4+7] = f->led[i].red;
	}
	//send out the buffer
	HAL_SPI_Transmit(&spi, &ledTransmitBuffer[0], SPI_FRAME_SIZE, HAL_MAX_DELAY);
}
