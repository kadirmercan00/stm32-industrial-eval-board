# STM32 Industrial Mixed-Signal Evaluation & Control Board

<div align="center">
  <img src="docs/images/stm32_eval_board_3d.jpg" alt="STM32 Evaluation Board 3D PCB Render" width="600" style="border-radius: 8px; box-shadow: 0 4px 20px rgba(0,0,0,0.4);" />
  <p><em>Custom 2-Layer Altium Designer Hardware: STMicroelectronics STM32F103RDT6 + RS-485 + Op-Amp Conditioned Mixed-Signal AFE</em></p>
</div>

[![License: Proprietary Showcase](https://img.shields.io/badge/License-Proprietary_Portfolio_Showcase-red.svg)](LICENSE)
[![Hardware: Altium Designer](https://img.shields.io/badge/EDA-Altium_Designer_2-layer-A5915F.svg?logo=altiumdesigner)](docs/schematics/STM32_EVAL_BOARD_Schematic.pdf)
[![MCU: STM32F103](https://img.shields.io/badge/MCU-STMicroelectronics_STM32F103RDT6-03234B.svg?logo=stmicroelectronics)](https://www.st.com/)
[![Fieldbus: RS-485](https://img.shields.io/badge/Fieldbus-Half--Duplex_RS--485-FF6600.svg)](#industrial-rs-485-fieldbus-subsystem)
[![Analog: Op-Amp AFE](https://img.shields.io/badge/AFE-Operational_Amplifier_Signal_Conditioning-blue.svg)](#precision-analog-front-end-afe)

> [!WARNING]
> **PROPRIETARY PORTFOLIO SHOWCASE NOTICE**  
> This repository is published strictly for technical evaluation, circuit review, and architectural demonstration as part of **Kadir Mercan's (Mercan Studio)** engineering portfolio.  
> **All rights reserved.** Reproduction, commercial PCB fabrication, software compilation for production, or redistribution without prior written authorization is strictly prohibited under the terms of the included [LICENSE](LICENSE).

---

## 🎯 Executive Summary

A mixed-signal industrial development and evaluation board architected around the **STMicroelectronics STM32F103RDT6** (ARM Cortex-M3 @ 72 MHz, 384 KB Flash, 64 KB SRAM). 

Engineered for rugged fieldbus automation and precision instrumentation, the board combines **hardware-isolated half-duplex RS-485 communication**, an active **Operational Amplifier Analog Front-End (AFE)** for sensor signal conditioning (0-10V / 4-20mA scaling), digital-to-analog output generation, an alphanumeric parallel display port, and an onboard wide-input switching step-down converter.

---

## 🏛️ System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                 STM32 INDUSTRIAL EVALUATION BOARD HARDWARE                  │
├──────────────────────────────────────┬──────────────────────────────────────┤
│   Analog & Instrumentation           │   Digital & Communication Interfaces │
├──────────────────────────────────────┼──────────────────────────────────────┤
│ • 12-Bit ADC Conditioned Channels    │ • Half-Duplex RS-485 Transceiver     │
│   (Active Op-Amp Low-Pass Filtering) │   (Hardware Direction Control DE/RE) │
│ • 12-Bit DAC Analog Waveform Out     │ • Alphanumeric Display Interface     │
│ • Switching Step-Down Buck Regulator │ • SWD Hardware Debug Header          │
│   (Wide 12V/24V Industrial Input)    │ • User Heartbeat LEDs & Buttons      │
└──────────────────────────────────────┴──────────────────────────────────────┘
```

---

## 🔬 Hardware Engineering Highlights

### 1. Precision Analog Front-End (AFE)
Raw industrial sensor loops (such as pressure transducers and 4-20mA current loops) produce hazardous common-mode noise and voltage spikes:
* The board incorporates an op-amp conditioning stage that attenuates, level-shifts, and filters analog inputs into the STM32's native $0 - 3.3\,\text{V}$ range.
* Integrated second-order low-pass active filtering eliminates $50/60\,\text{Hz}$ mains hum and switching noise before entering the 12-bit ADC.

### 2. Industrial RS-485 Fieldbus Subsystem
* Routed to a dedicated transceiver (MAX485 / SN65HVD family) with TVS clamping diodes on differential lines $A$ and $B$.
* Directional flow control (`DE`/`RE`) is managed deterministically via GPIO `PA8`, guaranteeing minimum turnaround latency and zero frame collisions.

### 3. Complete Electrical Schematics
Designed with modular hierarchical sheets in Altium Designer:
* Multi-sheet schematic document: [`docs/schematics/STM32_EVAL_BOARD_Schematic.pdf`](docs/schematics/STM32_EVAL_BOARD_Schematic.pdf).

---

## 📁 Repository Structure

```
stm32-industrial-eval-board/
├── docs/
│   ├── images/
│   │   └── stm32_eval_board_3d.jpg          # High-resolution Altium 3D PCB render
│   └── schematics/
│       └── STM32_EVAL_BOARD_Schematic.pdf   # Complete multi-sheet electrical schematic
├── firmware/
│   ├── include/
│   │   └── board_pinout.h                   # STM32F103 pin assignments & clock macros
│   ├── src/
│   │   └── main.cpp                         # RS-485 driver, 12-bit ADC conditioning & LCD
│   └── platformio.ini                       # PlatformIO toolchain (ststm32 platform)
├── LICENSE                                  # Strict Proprietary Portfolio Showcase License
└── README.md                                # Comprehensive engineering documentation
```

---

## 👨‍💻 Engineering & Contact

**Kadir Mercan**  
*Senior Embedded Systems, Hardware (PCB) & IoT Engineer*  
- **GitHub:** [@kadirmercan00](https://github.com/kadirmercan00)  
- **Brand:** Mercan Studio  
- **Core Competencies:** Altium Designer Mixed-Signal Layout, ARM Cortex-M Firmware, Industrial Fieldbuses (RS-485/Modbus/CAN), Analog Signal Conditioning.
