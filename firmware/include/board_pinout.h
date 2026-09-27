/**
 * @file board_pinout.h
 * @brief STM32 Industrial Evaluation Board Hardware Pin Mappings
 * @author Kadir Mercan (Mercan Studio)
 */

#ifndef BOARD_PINOUT_H
#define BOARD_PINOUT_H

// ==================== MCU CLOCK & IDENTIFIER ====================
#define TARGET_MCU              "STM32F103RDT6"
#define CORE_CLOCK_HZ           72000000UL  // 72 MHz PLL

// ==================== RS-485 INDUSTRIAL FIELDBUS ====================
// Routed to on-board RS-485 transceiver (MAX485 / SN65HVD)
#define PIN_RS485_TX            PA9         // USART1_TX
#define PIN_RS485_RX            PA10        // USART1_RX
#define PIN_RS485_DE_RE         PA8         // Driver Enable / Receiver Enable (Active-High TX)

// ==================== ANALOG FRONT-END (OP-AMP BUFFERED) ====================
// Conditioned analog input channels for 4-20mA / 0-10V industrial loops
#define PIN_ANALOG_IN_CH1       PA0         // ADC1_IN0
#define PIN_ANALOG_IN_CH2       PA1         // ADC1_IN1
#define PIN_DAC_OUT             PA4         // DAC_OUT1 (Arbitrary Waveform / Voltage Generation)

// ==================== PARALLEL / SPI LCD INTERFACE ====================
#define PIN_LCD_RS              PB0
#define PIN_LCD_EN              PB1
#define PIN_LCD_D4              PB12
#define PIN_LCD_D5              PB13
#define PIN_LCD_D6              PB14
#define PIN_LCD_D7              PB15

// ==================== USER INTERFACE & DIAGNOSTICS ====================
#define PIN_STATUS_LED1         PC13        // Onboard Activity Heartbeat
#define PIN_STATUS_LED2         PC14        // RS485 Comm / Bus Status
#define PIN_USER_BUTTON         PA15        // User Function Key

#endif // BOARD_PINOUT_H
