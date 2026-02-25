#include "stm32f0xx_hal.h"
#include "Nucleo-F070RB-board-config.h"
#include "sx126x.h"
#include <string.h>
#include <stdio.h>
#include<stdlib.h>
#include "usart.h"
#include "sx126x_hal.h"
#include "spi.h"
/* Global Variables */

/* Global Variables */
#define PAYLOAD_LENGTH 64     // Define payload length
uint16_t irqStatus;
uint16_t TXPacketCount = 0;  // Tracks number of successfully sent packets
uint8_t tx_data[] = "Hello 0123456789";      //the message to send
char rx_buffer[10];  // Buffer for UART input
static volatile bool irq_fired = false;

/* Global Variables */
uint16_t RXPacketCount = 0;   // Tracks number of received packets
uint16_t errors = 0;          // Tracks number of reception errors
int8_t PacketRSSI = 0;        // Stores RSSI value
int8_t PacketSNR = 0;         // Stores SNR value

uint8_t rx_data[PAYLOAD_LENGTH];  // Buffer for received data
uint8_t rxLength = 0;
uint8_t rx_done = 1;
uint8_t tx_done = 1;
/* UART and SPI Handles */
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi2;
/* Function Prototypes */
void LoRa_Init(void);
void LoRa_Send(void);
void LoRa_Receive(void);
void Error_Handler(void);
/*!
 * @brief System Clock Configuration
 */
void SystemClock_Config(void);

/*!
 * @brief GPIO Initialization
 */
void MX_GPIO_Init(void);

/*!
 * @brief SPI2 Initialization
 */
void MX_SPI2_Init(void);
/* Function for UART Debugging */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, HAL_MAX_DELAY);
	return len;
}
/*!
 * @brief Main application
 */
int main(void) {
	// HAL Initialization
	HAL_Init();

	SystemClock_Config();
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_SPI2_Init();

	printf("I am ready\n");
	/* Initialize LoRa */
	LoRa_Init();
	printf("Enter \"tx\" to send data and enter \"rx\" to receive data: ");
	fflush(stdout);

	while (1) {
		printf("Enter \"tx\" to send data and enter \"rx\" to receive data: ");
		// Non-blocking reception of one character at a time
		HAL_UART_Receive(&huart2, (uint8_t*) rx_buffer, 2, HAL_MAX_DELAY);
		rx_buffer[2] = '\0';  // Null-terminate the received string for safety

		printf("\n[DEBUG] Received command: %s\n", rx_buffer); // Debug print to confirm reception

		if (strncmp(rx_buffer, "tx", 2) == 0) {
			printf("[INFO] Sending data via LoRa...\n");
			LoRa_Send();  // Start TX mode
		} else if (strncmp(rx_buffer, "rx", 2) == 0) {
			printf("[INFO] Receiving data via LoRa...\n");
			LoRa_Receive();  // Start RX mode
		} else {
			printf("[ERROR] Invalid command! Please enter \"tx\" or \"rx\".\n");
		}
	}
}

void LoRa_Init(void) {
	printf("[INFO] lora init!\n");

	sx126x_hal_reset(NULL);
	printf("[INFO] lora RESET!\n");
	sx126x_hal_wakeup(NULL);
	printf("[INFO] lora WAKEUP!\n");
	sx126x_set_reg_mode(NULL, SX126X_REG_MODE_DCDC);
	sx126x_set_dio2_as_rf_sw_ctrl(NULL, true);

	sx126x_set_dio3_as_tcxo_ctrl(NULL, SX126X_TCXO_CTRL_3_3V, 500);
	HAL_Delay(5);  // Small delay (~5ms) for TCXO stabilization
	sx126x_cal(NULL, SX126X_CAL_ALL);
	// Step 4: Wait for the TCXO to stabilize
	HAL_Delay(5);  // Small delay (~5ms) for TCXO stabilization;

	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
	printf("[INFO] lora STAND-BY!\n");
	sx126x_set_pkt_type(NULL, SX126X_PKT_TYPE_LORA);
	printf("[INFO] Setting sx1262 in LoRa mode!......\n");
	sx126x_hal_status_t set_lora_freq = sx126x_set_rf_freq(NULL, 868000000); // 868 MHz (Europe) or 915 MHz (US)
	printf("[INFO] Setting frequency.........!\n");
	if (set_lora_freq == 0) {
		printf("[INFO] RF frequency set successfully.\n");
	} else {
		printf("[ERROR] Failed to set RF frequency! Status: %d\n", set_lora_freq);
	}

	sx126x_pa_cfg_params_t pa_config = { .pa_duty_cycle = 0x02, .hp_max = 0x02,
			.device_sel = 0x00, .pa_lut = 0x01 };

	sx126x_set_pa_cfg(NULL, &pa_config);

	sx126x_set_tx_params(NULL, 14, SX126X_RAMP_200_US);

	sx126x_set_rx_tx_fallback_mode( NULL, FALLBACK_MODE);

	sx126x_cfg_rx_boosted( NULL, ENABLE_RX_BOOST_MODE);

	sx126x_mod_params_lora_t mod_params = { .sf = SX126X_LORA_SF7, // Spreading Factor 7
			.bw = SX126X_LORA_BW_125,    // Bandwidth 125 kHz = 0x04
			.cr = SX126X_LORA_CR_4_8,    // Coding Rate 4/5
			.ldro = 0                    // Low data rate optimization off
			};
	sx126x_hal_status_t set_lora_params = sx126x_set_lora_mod_params(NULL, &mod_params);
	printf("[INFO] Setting LoRa Params....!\n");
	if(set_lora_params == 0){
     printf("[INFO] Setting LoRa modulation parameters:\n");
     printf("[INFO] Spreading Factor: SF%d\n", mod_params.sf);
     printf("[INFO] Bandwidth: %d kHz\n", (mod_params.bw == SX126X_LORA_BW_125) ? 125 :
                                          (mod_params.bw == SX126X_LORA_BW_250) ? 250 :
                                          (mod_params.bw == SX126X_LORA_BW_500) ? 500 : 0);
     printf("[INFO] Coding Rate: 4/%d\n", (mod_params.cr + 4));
     printf("[INFO] LDRO: %s\n", (mod_params.ldro) ? "Enabled" : "Disabled");
	} else{
		printf("[ERROR] Failed to set LoRa modulation parameters! Status: %d\n", set_lora_params);
	}
	sx126x_pkt_params_lora_t pkt_params = { .preamble_len_in_symb = 8, // Preamble length (symbols)
			.header_type = SX126X_LORA_PKT_EXPLICIT,   // Explicit header
			.pld_len_in_bytes = 16,         // Payload length (adjust as needed)
			.crc_is_on = false,                         // Enable CRC
			.invert_iq_is_on = false                   // Normal IQ setup
			};
	sx126x_set_lora_pkt_params(
			NULL, &pkt_params);

	sx126x_set_lora_sync_word(NULL, 0x12);

}

/* Function to Send Data */
void LoRa_Send(void) {
	printf("[INFO] Initialized LoRa TX Function............\n");
	sx126x_set_buffer_base_address(NULL, 0x00, 0x80);
	printf("[INFO] Sending DATA............\n");
	sx126x_write_buffer(NULL, 0x00, tx_data, sizeof(tx_data));
	printf("[INFO] Send DATA size of '%d': %s\n", sizeof(tx_data), tx_data);
	sx126x_set_dio_irq_params(
	NULL, SX126X_IRQ_ALL,
			SX126X_IRQ_TX_DONE | SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT
					| SX126X_IRQ_HEADER_ERROR | SX126X_IRQ_CRC_ERROR,
			SX126X_IRQ_NONE, SX126X_IRQ_NONE);

	sx126x_clear_irq_status( NULL, SX126X_IRQ_ALL);

	/* Start Transmission */
	sx126x_set_tx(NULL, 5000); // 5000ms timeout
//	if (status == 0)
//	{
//		printf("[INFO] Tx is Successful!\n");
//	}
//	else{
//		printf("[ERROR] Tx has a trouble in transmistting..............");
//	}
//	sx126x_irq_mask_t irq_regs;
//	sx126x_status_t tx_status = sx126x_get_and_clear_irq_status( NULL, &irq_regs);
//	printf("Tx done\n");
//	//	apps_common_sx126x_irq_process( NULL);
	while (tx_done) {
		sx126x_get_irq_status(NULL, &irqStatus);  // Poll IRQ status

		if (irqStatus & SX126X_IRQ_TX_DONE) {
			// Clear IRQ flags and restart TX
			printf("[DEBUG] IRQ State = %d\n", SX126X_IRQ_TX_DONE);
			printf("[INFO] Tx is Successful!\n");
			sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
			tx_done = 0;

		}
		else if (irqStatus & SX126X_IRQ_TIMEOUT) {
			// TX timed out, restart TX
			printf("TX Timeout, restarting...\r\n");
			sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
			LoRa_Send();
		}
	}
}

/* Function to Receive Data */
void LoRa_Receive(void) {
	printf("[INFO] Lora RX Function initialised.....\n");
	// Enable RX interrupts (IRQ settings)
	sx126x_set_dio_irq_params(
	NULL, SX126X_IRQ_ALL,
			SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT | SX126X_IRQ_HEADER_ERROR
					| SX126X_IRQ_CRC_ERROR, SX126X_IRQ_NONE, SX126X_IRQ_NONE);
	sx126x_clear_irq_status(NULL, SX126X_IRQ_RX_DONE);

	sx126x_set_rx(NULL, 5000); // 5000ms timeout
	printf("Here:\n");
	while (rx_done) {
		sx126x_get_irq_status(NULL, &irqStatus);  // Poll IRQ status

		if (irqStatus & SX126X_IRQ_RX_DONE) {
			// Packet received successfully
			printf("[DEBUG] IRQ State = %d\n", SX126X_IRQ_RX_DONE);
			sx126x_rx_buffer_status_t rxStatus;
			sx126x_get_rx_buffer_status(NULL, &rxStatus);

			rxLength = rxStatus.pld_len_in_bytes;  // Get payload length
			uint8_t offset = rxStatus.buffer_start_pointer;

			// Read the received data into rxBuffer
			sx126x_read_buffer(NULL, offset, (uint8_t*)rx_buffer, rxLength);

			// Print to terminal
			char msg[300];
			snprintf(msg, sizeof(msg), "Received (%d bytes): %.*s\r\n",
					rxLength, rxLength, rx_buffer);
			printf(msg);

			// Clear IRQ flags and restart RX
			sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
			rx_done = 0;
		} else if (irqStatus & SX126X_IRQ_TIMEOUT) {
			// RX timed out, restart RX
			printf("RX Timeout, restarting...\r\n");
			sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
			LoRa_Receive();
		}

		// Small delay to avoid overloading the CPU (adjust as needed)
		HAL_Delay(10);
	}
}


/*!
 * @brief System Clock Configuration
 */
void SystemClock_Config(void) {
	// Configure your system clock here
}
void Error_Handler(void) {
	while (1) {
//	        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//	        HAL_Delay(100);
	}
	/* USER CODE END Error_Handler_Debug */
}
void EXTI4_15_IRQHandler(void) {
	if (__HAL_GPIO_EXTI_GET_IT(RADIO_DIO1_PIN) != RESET) {
		// Handle DIO1 interrupt
//        SX126xDioIrqHandler();
		__HAL_GPIO_EXTI_CLEAR_IT(RADIO_DIO1_PIN);
	}
}
