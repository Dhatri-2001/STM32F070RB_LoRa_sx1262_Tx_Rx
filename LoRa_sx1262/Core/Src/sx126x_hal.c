#include "stm32f0xx_hal.h"
#include "Nucleo-F070RB-board-config.h"
#include "sx126x_hal.h"

// Declare SPI handle (defined in main.c or another source file)
extern SPI_HandleTypeDef hspi2;

/**
 * @brief Resets the SX1262 module using the RESET pin.
 *
 * The reset sequence:
 * 1. Set RESET pin LOW
 * 2. Wait for a small delay
 * 3. Set RESET pin HIGH
 */
sx126x_hal_status_t sx126x_hal_reset(const void* context) {
    RADIO_RESET_LOW();  // Drive RESET pin LOW to initiate reset
    HAL_Delay(10);      // Wait for 10ms (SX1262 reset requirement)
    RADIO_RESET_HIGH(); // Set RESET pin HIGH to complete reset
    HAL_Delay(5);       // Additional delay to ensure proper startup
    return SX126X_HAL_STATUS_OK;
}

/**
 * @brief Wakes up the SX1262 from sleep mode.
 *
 * The wake-up sequence:
 * 1. Pull NSS LOW
 * 2. Send a dummy byte (0x00) over SPI
 * 3. Pull NSS HIGH
 * 4. Wait until the BUSY pin goes LOW
 */
sx126x_hal_status_t sx126x_hal_wakeup(const void* context) {
    RADIO_NSS_LOW();  // Select SPI device
    uint8_t dummy_byte = 0x00;
    HAL_SPI_Transmit(&hspi2, &dummy_byte, 1, HAL_MAX_DELAY); // Send dummy byte
    RADIO_NSS_HIGH(); // Deselect SPI device
    while (RADIO_BUSY_READ() == GPIO_PIN_SET); // Wait for BUSY pin to go LOW
    return SX126X_HAL_STATUS_OK;
}

/**
 * @brief Writes data to the SX1262 via SPI.
 *
 * @param context Context pointer (unused in this implementation)
 * @param command Command buffer to be sent
 * @param command_length Length of the command buffer
 * @param buffer Data buffer to send
 * @param buffer_length Number of bytes to send
 *
 * The write sequence:
 * 1. Wait until BUSY pin is LOW
 * 2. Pull NSS LOW
 * 3. Send command bytes
 * 4. Send data buffer
 * 5. Pull NSS HIGH
 */
sx126x_hal_status_t sx126x_hal_write(const void* context, const uint8_t* command,
    const uint16_t command_length, const uint8_t* buffer,
    const uint16_t buffer_length) {
uint8_t dummy_rx[command_length + buffer_length]; // Dummy buffer to store received data
uint8_t tx_data[command_length + buffer_length];

// Combine command and buffer data into one array for transmission
memcpy(tx_data, command, command_length);
memcpy(tx_data + command_length, buffer, buffer_length);

while (RADIO_BUSY_READ() == GPIO_PIN_SET); // Wait for BUSY pin to be LOW

RADIO_NSS_LOW();  // Select SPI device
HAL_SPI_TransmitReceive(&hspi2, tx_data, dummy_rx, command_length + buffer_length, HAL_MAX_DELAY);
RADIO_NSS_HIGH(); // Deselect SPI device

return SX126X_HAL_STATUS_OK;
}



/**
 * @brief Reads data from the SX1262 via SPI.
 *
 * @param context Context pointer (unused in this implementation)
 * @param command Command buffer for reading
 * @param command_length Length of the command buffer
 * @param buffer Buffer to store received data
 * @param buffer_length Number of bytes to read
 *
 * The read sequence:
 * 1. Wait until BUSY pin is LOW
 * 2. Pull NSS LOW
 * 3. Send command bytes
 * 4. Read response into buffer
 * 5. Pull NSS HIGH
 */
sx126x_hal_status_t sx126x_hal_read(const void* context, const uint8_t* command,
        const uint16_t command_length, uint8_t* buffer,
        const uint16_t buffer_length) {
    uint8_t tx_dummy[command_length + buffer_length]; // Dummy TX buffer
    uint8_t rx_data[command_length + buffer_length];  // RX buffer to store received data

    memset(tx_dummy, 0x00, sizeof(tx_dummy)); // Fill with dummy bytes

    // Copy command into first part of tx_dummy
    memcpy(tx_dummy, command, command_length);

    while (RADIO_BUSY_READ() == GPIO_PIN_SET); // Wait for BUSY pin to be LOW

    RADIO_NSS_LOW();  // Select SPI device
    HAL_SPI_TransmitReceive(&hspi2, tx_dummy, rx_data, command_length + buffer_length, HAL_MAX_DELAY);
    RADIO_NSS_HIGH(); // Deselect SPI device

    // Copy only the received payload (skip the command part)
    memcpy(buffer, rx_data + command_length, buffer_length);

    return SX126X_HAL_STATUS_OK;
}
