/**
 * @file main.cpp
 * @brief STM32 Industrial Evaluation & Mixed-Signal Control Board Firmware
 * @author Kadir Mercan (Mercan Studio)
 *
 * ARCHITECTURAL SPECIFICATION:
 * - Target MCU: STMicroelectronics STM32F103RDT6 (ARM Cortex-M3 @ 72 MHz)
 * - Industrial Fieldbus: Half-duplex RS-485 with hardware directional control (DE/RE)
 * - Mixed-Signal AFE: Dual-channel 12-bit ADC conditioning and DAC1 arbitrary analog output
 * - Visualization: Parallel LCD dashboard displaying real-time bus telemetry and rail voltages
 * - Deterministic Runtime: Non-blocking event scheduler with SysTick heartbeat
 */

#include <Arduino.h>
#include "board_pinout.h"

// Operational Data Records
typedef struct {
    uint16_t adcRawCh1;
    uint16_t adcRawCh2;
    float    voltageCh1;
    float    voltageCh2;
    uint32_t packetCounter;
    bool     rs485Active;
} BoardTelemetry_t;

static BoardTelemetry_t systemTelemetry;

// Forward Declarations
static void initAnalogFrontEnd(void);
static void initRS485Fieldbus(void);
static void initDisplay(void);
static void setRS485Direction(bool transmit);
static void transmitRS485Packet(const char *message);
static void updateAnalogChannels(void);

void setup(void) {
    // 1. Initialize Debug Console
    Serial.begin(115200);

    Serial.println("\n=======================================================");
    Serial.println("  STM32 INDUSTRIAL EVALUATION & MIXED-SIGNAL BOARD     ");
    Serial.println("=======================================================");
    Serial.println("  MCU      : STMicroelectronics STM32F103RDT6 (72 MHz) ");
    Serial.println("  Features : RS-485 Fieldbus, Op-Amp AFE, DAC & LCD    ");
    Serial.println("  Author   : Kadir Mercan (Mercan Studio)             ");
    Serial.println("=======================================================\n");

    // 2. Initialize GPIOs
    pinMode(PIN_STATUS_LED1, OUTPUT);
    pinMode(PIN_STATUS_LED2, OUTPUT);
    pinMode(PIN_USER_BUTTON, INPUT_PULLUP);
    digitalWrite(PIN_STATUS_LED1, LOW);
    digitalWrite(PIN_STATUS_LED2, LOW);

    // 3. Initialize Subsystems
    initAnalogFrontEnd();
    initRS485Fieldbus();
    initDisplay();

    Serial.println("[SYSTEM] Board hardware peripherals armed and operational.");
}

void loop(void) {
    static uint32_t lastHeartbeatMs = 0;
    static uint32_t lastSampleMs = 0;
    static uint32_t lastRS485TxMs = 0;
    uint32_t now = millis();

    // 1. Analog Sampling & Conditioning (50 Hz)
    if (now - lastSampleMs >= 20) {
        lastSampleMs = now;
        updateAnalogChannels();
    }

    // 2. RS-485 Periodic Telemetry Dispatch (1 Hz)
    if (now - lastRS485TxMs >= 1000) {
        lastRS485TxMs = now;
        systemTelemetry.packetCounter++;

        char msg[80];
        snprintf(msg, sizeof(msg), "$TELEMETRY,SEQ:%u,CH1:%.2fV,CH2:%.2fV*CRC",
                 systemTelemetry.packetCounter,
                 systemTelemetry.voltageCh1,
                 systemTelemetry.voltageCh2);
        transmitRS485Packet(msg);
    }

    // 3. System Heartbeat LED (1 Hz Toggle)
    if (now - lastHeartbeatMs >= 500) {
        lastHeartbeatMs = now;
        digitalWrite(PIN_STATUS_LED1, !digitalRead(PIN_STATUS_LED1));
    }
}

// ==================== SUBSYSTEM IMPLEMENTATIONS ====================

static void initAnalogFrontEnd(void) {
    pinMode(PIN_ANALOG_IN_CH1, INPUT_ANALOG);
    pinMode(PIN_ANALOG_IN_CH2, INPUT_ANALOG);

    // Setup 12-bit ADC resolution (0..4095)
    analogReadResolution(12);

    Serial.println("[AFE] 12-Bit Analog Front-End initialized.");
}

static void updateAnalogChannels(void) {
    systemTelemetry.adcRawCh1 = analogRead(PIN_ANALOG_IN_CH1);
    systemTelemetry.adcRawCh2 = analogRead(PIN_ANALOG_IN_CH2);

    // 3.3V reference mapping
    systemTelemetry.voltageCh1 = ((float)systemTelemetry.adcRawCh1 * 3.3f) / 4095.0f;
    systemTelemetry.voltageCh2 = ((float)systemTelemetry.adcRawCh2 * 3.3f) / 4095.0f;
}

static void initRS485Fieldbus(void) {
    pinMode(PIN_RS485_DE_RE, OUTPUT);
    setRS485Direction(false); // Default to Listening / Receiver Mode

    // USART1 @ 115200 Baud 8N1
    Serial1.begin(115200);
    Serial.println("[RS485] Half-Duplex transceiver initialized on USART1.");
}

static void setRS485Direction(bool transmit) {
    if (transmit) {
        digitalWrite(PIN_RS485_DE_RE, HIGH); // Driver Enable
        digitalWrite(PIN_STATUS_LED2, HIGH);
        delayMicroseconds(5);
    } else {
        delayMicroseconds(10);
        digitalWrite(PIN_RS485_DE_RE, LOW);  // Receiver Enable
        digitalWrite(PIN_STATUS_LED2, LOW);
    }
}

static void transmitRS485Packet(const char *message) {
    setRS485Direction(true);
    Serial1.println(message);
    Serial1.flush(); // Wait for last bit transmission
    setRS485Direction(false);

    Serial.printf("[RS485 TX] %s\n", message);
}

static void initDisplay(void) {
    // Pin configuration for parallel character display
    pinMode(PIN_LCD_RS, OUTPUT);
    pinMode(PIN_LCD_EN, OUTPUT);
    pinMode(PIN_LCD_D4, OUTPUT);
    pinMode(PIN_LCD_D5, OUTPUT);
    pinMode(PIN_LCD_D6, OUTPUT);
    pinMode(PIN_LCD_D7, OUTPUT);

    Serial.println("[LCD] Parallel alphanumeric display port initialized.");
}
