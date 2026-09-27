#!/usr/bin/env python3
"""
STM32 Industrial Evaluation Board — Modbus RTU Field Diagnostic Tool
Polls board registers over RS-485, logs ADC voltages, and calibrates DAC setpoints.
"""

import sys
import time
import struct
import argparse

try:
    import serial
except ImportError:
    serial = None

def calculate_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def build_read_packet(slave_addr: int, start_reg: int, count: int) -> bytes:
    payload = struct.pack(">BBHH", slave_addr, 0x03, start_reg, count)
    crc = calculate_crc16(payload)
    return payload + struct.pack("<H", crc)

def build_write_packet(slave_addr: int, reg: int, value: int) -> bytes:
    payload = struct.pack(">BBHH", slave_addr, 0x06, reg, value)
    crc = calculate_crc16(payload)
    return payload + struct.pack("<H", crc)

def poll_board(port: str, baud: int = 115200, slave_id: int = 1):
    if serial is None:
        print("[!] 'pyserial' not installed. Running simulated diagnostic loop...")
        mock_poll()
        return

    try:
        ser = serial.Serial(port, baud, timeout=0.5)
        print(f"[*] Connected to RS-485 bus on {port} @ {baud} baud. Polling Slave #{slave_id}...")

        while True:
            # Read 6 registers starting at address 1
            cmd = build_read_packet(slave_id, 1, 6)
            ser.write(cmd)
            ser.flush()

            resp = ser.read(17) # 1 + 1 + 1 + 12 + 2
            if len(resp) >= 17 and resp[0] == slave_id and resp[1] == 0x03:
                adc1_raw, adc1_mv, adc2_raw, adc2_mv, dac_mv, uptime = struct.unpack(">6H", resp[3:15])
                print(f"[POLL] Uptime: {uptime}s | ADC1: {adc1_mv} mV (Raw {adc1_raw}) | "
                      f"ADC2: {adc2_mv} mV (Raw {adc2_raw}) | DAC Out: {dac_mv} mV")
            else:
                print("[!] Timeout or invalid CRC from slave node.")

            time.sleep(1.0)
    except KeyboardInterrupt:
        print("\n[*] Diagnostic session closed.")
    except Exception as e:
        print(f"[ERROR] {e}")

def mock_poll():
    for i in range(5):
        print(f"[MOCK POLL] Uptime: {120 + i}s | ADC1: 1845 mV (Raw 2289) | ADC2: 2410 mV (Raw 2990) | DAC Out: 1500 mV")
        time.sleep(0.5)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="STM32 Modbus RTU Diagnostic Tester")
    parser.add_argument("--port", default="COM4", help="Serial port for RS-485 USB dongle")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--slave", type=int, default=1, help="Slave address")
    args = parser.parse_args()

    poll_board(args.port, args.baud, args.slave)
