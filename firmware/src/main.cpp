/**
 * @file main.cpp
 * @brief STM32 Industrial Evaluation Board Firmware with Modbus RTU Slave
 * @author Kadir Mercan (Mercan Studio)
 *
 * ARCHITECTURAL SPECIFICATION:
 * - Target MCU: STMicroelectronics STM32F103RDT6 (ARM Cortex-M3 @ 72 MHz)
 * - Industrial Fieldbus: Half-duplex RS-485 with hardware directional control (DE/RE)
 * - Industrial Protocol: Modbus RTU Slave (Device Address: 0x01, 115200 8N1)
 * - Mixed-Signal AFE: Dual-channel 12-bit ADC conditioning and DAC1 analog output
 * - Visualization: Status LEDs and serial telemetry diagnostics
 * - Deterministic Runtime: Non-blocking event scheduler with SysTick heartbeat
 */

#include <Arduino.h>
#include "board_pinout.h"

// Modbus RTU Constants
#define MODBUS_SLAVE_ADDR       0x01
#define MODBUS_MAX_FRAME_SIZE   64
#define RS485_BAUD_RATE         115200

// Operational Data Records
typedef struct {
    uint16_t adcRawCh1;
    uint16_t adcRawCh2;
    uint16_t voltageMvCh1;
    uint16_t voltageMvCh2;
    uint16_t dacSetpointMv;
    uint32_t packetCounter;
    uint32_t uptimeSeconds;
} BoardTelemetry_t;

static BoardTelemetry_t systemTelemetry;

// Forward Declarations
static void initAnalogFrontEnd(void);
static void initRS485Fieldbus(void);
static void setRS485Direction(bool transmit);
static void updateAnalogChannels(void);
static void processModbusRTU(void);
static uint16_t calculateCRC16(const uint8_t *buffer, uint16_t length);

void setup(void) {
    // 1. Initialize Debug & RS485 Serial
    Serial.begin(115200);

    Serial.println("\n=======================================================");
    Serial.println("  STM32 INDUSTRIAL EVALUATION & MIXED-SIGNAL BOARD     ");
    Serial.println("=======================================================");
    Serial.println("  MCU      : STMicroelectronics STM32F103RDT6 (72 MHz) ");
    Serial.println("  Fieldbus : Isolated RS-485 Modbus RTU Slave (Addr: 1)");
    Serial.println("  Features : Op-Amp AFE, 12-bit ADC, DAC1 Voltage Gen  ");
    Serial.println("  Author   : Kadir Mercan (Mercan Studio)              ");
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

    systemTelemetry.dacSetpointMv = 1500; // Default 1.50V output
    systemTelemetry.packetCounter = 0;
}

void loop(void) {
    static uint32_t lastTelemetryTime = 0;
    static uint32_t lastHeartbeatTime = 0;
    uint32_t now = millis();

    // 1. Check & Process Modbus RTU Frame from RS-485
    processModbusRTU();

    // 2. Sample Analog Inputs (every 50 ms)
    if (now - lastTelemetryTime >= 50) {
        lastTelemetryTime = now;
        updateAnalogChannels();
    }

    // 3. Heartbeat Indicator (1 Hz)
    if (now - lastHeartbeatTime >= 1000) {
        lastHeartbeatTime = now;
        systemTelemetry.uptimeSeconds++;
        digitalWrite(PIN_STATUS_LED1, !digitalRead(PIN_STATUS_LED1));
    }
}

static void initAnalogFrontEnd(void) {
    pinMode(PIN_ANALOG_IN_CH1, INPUT_ANALOG);
    pinMode(PIN_ANALOG_IN_CH2, INPUT_ANALOG);
    pinMode(PIN_DAC_OUT, OUTPUT);
    analogReadResolution(12);
}

static void initRS485Fieldbus(void) {
    pinMode(PIN_RS485_DE_RE, OUTPUT);
    setRS485Direction(false); // Default to RX listening
}

static void setRS485Direction(bool transmit) {
    digitalWrite(PIN_RS485_DE_RE, transmit ? HIGH : LOW);
    if (transmit) {
        delayMicroseconds(5); // Stabilization window
    }
}

static void updateAnalogChannels(void) {
    systemTelemetry.adcRawCh1 = analogRead(PIN_ANALOG_IN_CH1);
    systemTelemetry.adcRawCh2 = analogRead(PIN_ANALOG_IN_CH2);
    
    // Scale 12-bit ADC (0-4095) to 0-3300 mV
    systemTelemetry.voltageMvCh1 = (uint16_t)(((uint32_t)systemTelemetry.adcRawCh1 * 3300) / 4095);
    systemTelemetry.voltageMvCh2 = (uint16_t)(((uint32_t)systemTelemetry.adcRawCh2 * 3300) / 4095);
}

static void processModbusRTU(void) {
    if (Serial.available() < 8) return;

    uint8_t buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t idx = 0;

    while (Serial.available() && idx < MODBUS_MAX_FRAME_SIZE) {
        buffer[idx++] = Serial.read();
        delayMicroseconds(250); // Inter-character gap detection
    }

    if (idx < 8) return;

    // Validate Slave Address
    if (buffer[0] != MODBUS_SLAVE_ADDR) return;

    // Verify CRC-16
    uint16_t receivedCRC = buffer[idx - 2] | (buffer[idx - 1] << 8);
    uint16_t calculatedCRC = calculateCRC16(buffer, idx - 2);

    if (receivedCRC != calculatedCRC) return;

    digitalWrite(PIN_STATUS_LED2, HIGH); // Comm activity LED
    uint8_t funcCode = buffer[1];

    // Function Code 0x03: Read Holding Registers
    if (funcCode == 0x03) {
        uint16_t startReg = (buffer[2] << 8) | buffer[3];
        uint16_t regCount = (buffer[4] << 8) | buffer[5];

        uint8_t response[32];
        response[0] = MODBUS_SLAVE_ADDR;
        response[1] = 0x03;
        response[2] = regCount * 2; // Byte count

        uint16_t respIdx = 3;
        for (uint16_t i = 0; i < regCount; i++) {
            uint16_t val = 0;
            switch (startReg + i) {
                case 1: val = systemTelemetry.adcRawCh1; break;
                case 2: val = systemTelemetry.voltageMvCh1; break;
                case 3: val = systemTelemetry.adcRawCh2; break;
                case 4: val = systemTelemetry.voltageMvCh2; break;
                case 5: val = systemTelemetry.dacSetpointMv; break;
                case 6: val = (uint16_t)systemTelemetry.uptimeSeconds; break;
                default: val = 0; break;
            }
            response[respIdx++] = (val >> 8) & 0xFF;
            response[respIdx++] = val & 0xFF;
        }

        uint16_t crc = calculateCRC16(response, respIdx);
        response[respIdx++] = crc & 0xFF;
        response[respIdx++] = (crc >> 8) & 0xFF;

        setRS485Direction(true);
        Serial.write(response, respIdx);
        Serial.flush();
        setRS485Direction(false);

        systemTelemetry.packetCounter++;
    }
    // Function Code 0x06: Write Single Register
    else if (funcCode == 0x06) {
        uint16_t reg = (buffer[2] << 8) | buffer[3];
        uint16_t val = (buffer[4] << 8) | buffer[5];

        if (reg == 5) { // Update DAC output
            systemTelemetry.dacSetpointMv = val;
            // Write DAC1 output pin
            analogWrite(PIN_DAC_OUT, (val * 255) / 3300);
        }

        // Echo response
        setRS485Direction(true);
        Serial.write(buffer, 8);
        Serial.flush();
        setRS485Direction(false);
    }

    digitalWrite(PIN_STATUS_LED2, LOW);
}

static uint16_t calculateCRC16(const uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
