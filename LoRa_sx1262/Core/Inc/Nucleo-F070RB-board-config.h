/*!
 * @file Nucleo-F070RB-board-config.h
 * @brief STM32F070RB SX1262 Radio Board Pin Configuration
 */

 #ifndef __NUCLEO_F070RB_BOARD_CONFIG_H__
 #define __NUCLEO_F070RB_BOARD_CONFIG_H__
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include "stm32f0xx_hal.h"
 
 /*!
  * @name Radio Control Pins
  * @{
  */
 #define RADIO_NSS_PIN                                GPIO_PIN_12
 #define RADIO_NSS_PORT                               GPIOA
 #define RADIO_RESET_PIN                              GPIO_PIN_8
 #define RADIO_RESET_PORT                             GPIOA
 #define RADIO_BUSY_PIN                               GPIO_PIN_9
 #define RADIO_BUSY_PORT                              GPIOB
 #define RADIO_DIO1_PIN                               GPIO_PIN_8
 #define RADIO_DIO1_PORT                              GPIOB
 /** @} */
 
 /*!
  * @name SPI Configuration
  * @{
  */
 #define RADIO_SPI                                    SPI2
 #define RADIO_SPI_CLK_PIN                            GPIO_PIN_10
 #define RADIO_SPI_CLK_PORT                           GPIOB
 #define RADIO_SPI_MOSI_PIN                           GPIO_PIN_3
 #define RADIO_SPI_MOSI_PORT                          GPIOC
 #define RADIO_SPI_MISO_PIN                           GPIO_PIN_2
 #define RADIO_SPI_MISO_PORT                          GPIOC
 
 extern SPI_HandleTypeDef hspi2;  // Defined in main.c
 /** @} */
 
 /*!
  * @brief Helper macros for GPIO operations
  */
 #define RADIO_NSS_LOW()          HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_RESET)
 #define RADIO_NSS_HIGH()         HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_SET)
 #define RADIO_RESET_LOW()        HAL_GPIO_WritePin(RADIO_RESET_PORT, RADIO_RESET_PIN, GPIO_PIN_RESET)
 #define RADIO_RESET_HIGH()       HAL_GPIO_WritePin(RADIO_RESET_PORT, RADIO_RESET_PIN, GPIO_PIN_SET)
 #define RADIO_BUSY_READ()        HAL_GPIO_ReadPin(RADIO_BUSY_PORT, RADIO_BUSY_PIN)
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* __STM_BOARD_CONFIG_H__ */