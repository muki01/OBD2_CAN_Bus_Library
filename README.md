<div align="center">

# 🚗 OBD2 CAN Bus Library — ESP32 Arduino Library

**A lightweight yet powerful ESP32 Arduino library for OBD-II diagnostics over the CAN bus (ISO 15765) — automatic protocol detection, live sensor data, DTC read/clear, VIN & vehicle info, and Mode 06 on-board test results.**

![GitHub forks](https://img.shields.io/github/forks/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub Repo stars](https://img.shields.io/github/stars/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub License](https://img.shields.io/github/license/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub last commit](https://img.shields.io/github/last-commit/muki01/OBD2_CAN_Bus_Library)
![ESP32](https://img.shields.io/badge/ESP32-000000?logo=espressif&logoColor=red)
![Arduino](https://img.shields.io/badge/Arduino-00979D?logo=arduino&logoColor=white)
![Protocol](https://img.shields.io/badge/Protocol-CAN%20Bus%20(ISO%2015765)-blue)

</div>

---

## 📌 Overview

**OBD2_CanBus** is a lightweight yet powerful **ESP32-compatible Arduino library** that enables direct **OBD-II communication with vehicles over the CAN bus**. It lets your microcontroller talk straight to the car's ECU to read real-time sensor values, diagnose and clear trouble codes, and pull vehicle information — no ELM327 required. Designed for **ESP32** and similar platforms, it supports both 11-bit and 29-bit identifiers with automatic protocol detection.

## ❓ Does Your Vehicle Support CAN Bus?

Before using this library, confirm your car speaks CAN by checking the OBD-II connector pins:

- ✅ **Pins 6 & 14 connected → CAN bus.** This library will work.
- ❌ **Pin 7 connected → K-Line** (ISO 9141 / ISO 14230 / KWP2000). Use my [OBD2 K-Line Library](https://github.com/muki01/OBD2_KLine_Library) instead.

**Example OBD-II connectors** (left: K-Line with pin 7 · right: CAN with pins 6 & 14):

<p>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20KLine.jpg" width="40%">
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20CanBus.jpg" width="40%">
</p>

## ✨ Features

- 🔀 **Automatic protocol detection** — 11-bit & 29-bit, 250 kbps & 500 kbps.
- 📊 **Live sensor data** — read real-time PIDs (RPM, speed, temperatures, and more).
- ⚠️ **DTC handling** — read stored & pending trouble codes, and clear them (MIL reset).
- 🚙 **Vehicle info** — retrieve VIN, calibration IDs and more.
- 🧪 **Mode 06 support** — on-board monitoring test results.
- 🐞 **Debug output** — for easy development and troubleshooting.
- ⏱️ **Customizable timing** — adjustable delays and request intervals.

## 📡 Supported OBD-II Modes

| Mode | Description |
| ---- | ----------- |
| 01 | Read current live data (sensor values) |
| 02 | Read freeze-frame data |
| 03 | Read stored Diagnostic Trouble Codes (DTCs) |
| 04 | Clear DTCs and reset the MIL |
| 05 | Oxygen sensor test results |
| 06 | On-board monitoring test results |
| 07 | Read pending Diagnostic Trouble Codes |
| 09 | Retrieve vehicle information (VIN, calibration IDs) |

## 📊 Typical Data Rates

Real-world throughput measured with this library:

| Protocol | Average responses per second |
| -------- | ---------------------------- |
| 250 kbps | Not tested |
| 500 kbps | **Over 100 responses/sec** |

> 🔎 Actual throughput varies with the ECU's internal processing time, the requested PID type, and overall system latency.

## 🛠️ Schematic

CAN transceiver wiring (TJA1050):

<img src="https://github.com/muki01/OBD2_CAN_Bus_Library/blob/main/images/TJA1050%20Schematic.png" width="70%">

## 🔗 Related Projects

- [OBD2 CAN Bus Reader](https://github.com/muki01/OBD2_CAN_Bus_Reader) — ready-to-use CAN reader firmware
- [OBD2 K-Line Library](https://github.com/muki01/OBD2_KLine_Library) — for ISO 9141 / ISO 14230 vehicles
- [OBD2 Diagnostic UI](https://github.com/muki01/OBD2-Diagnostic-UI) — web dashboard front-end
- [BMW I/K Bus](https://github.com/muki01/I-K_Bus) · [VAG KW1281](https://github.com/muki01/VAG_KW1281)

## ☕ Support My Work

If you enjoy my projects and want to support me, you can do so through the links below:

[![Buy Me A Coffee](https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-FFDD00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://www.buymeacoffee.com/muki01)
[![PayPal](https://img.shields.io/badge/-PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate/?hosted_button_id=SAAH5GHAH6T72)
[![GitHub Sponsors](https://img.shields.io/badge/-Sponsor%20Me%20on%20GitHub-181717?style=for-the-badge&logo=github)](https://github.com/sponsors/muki01)

---

## 📬 Contact

For information, job offers, collaboration, sponsorship, or purchasing my devices, you can contact me via email.

📧 Email: muksin.muksin04@gmail.com

---

<div align="center">

Created by [**Muki**](https://github.com/muki01) · If you find this useful, consider giving it a ⭐

</div>
