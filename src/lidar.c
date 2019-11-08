/*
 * lidar.c
 *
 *  Created on: Mar 22, 2019
 *      Author: swallen
 */

#include "lidar.h"

uint8_t lidarDataBuf [8];

static SPI_HandleTypeDef spi2 = {.Instance = SPI2 };

void initLidar() {
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**Lidar GPIO Configuration
    PB12    ------> SPI2_NSS
    PB13    ------> SPI2_SCK
    PB14    ------> TDC_INTB
    PB15    ------> LIDAR_ON
    PC1     ------> SPI2_MOSI
    PC2     ------> SPI2_MISO
    PC6     ------> TDC_EN
    */

    //Init gpio for NSS, SCK
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //init gpio for mosi, miso
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //init gpio for lidar_on
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //init gpio for tdc_en
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //init gpio for tdc_intb
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


	spi2.Init.Mode = SPI_MODE_MASTER;
	spi2.Init.Direction = SPI_DIRECTION_2LINES;
	spi2.Init.DataSize = SPI_DATASIZE_8BIT;
	spi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	spi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	spi2.Init.NSS = SPI_NSS_SOFT;
	spi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	spi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi2.Init.TIMode = SPI_TIMODE_DISABLE;
	spi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	spi2.Init.CRCPolynomial = 10;

	HAL_SPI_Init(&spi2);

	//make sure we can talk to the TDC7200. INT_MASK bit 0 should always be set
	//trace_printf("D:%d\n", readSPIReg8Blocking(spi2, TDC7200_INT_MASK));
}

uint8_t readSPIReg8Blocking(SPI_HandleTypeDef spi, uint8_t addr) {
	uint8_t data = addr;// | TDC7200_SPI_WRITE;
	HAL_SPI_Transmit(&spi, &data, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&spi, &data, 1, HAL_MAX_DELAY);
	return data;
}

void turnSourceOn() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
}

void turnSourceOff() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
}
