# OBD2 CanBus Library

![GitHub forks](https://img.shields.io/github/forks/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub Repo stars](https://img.shields.io/github/stars/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub License](https://img.shields.io/github/license/muki01/OBD2_CAN_Bus_Library?style=flat)
![GitHub last commit](https://img.shields.io/github/last-commit/muki01/OBD2_CAN_Bus_Library)

**OBD2_CanBus** is a lightweight yet powerful ESP32 compatible library that enables direct communication with vehicles using the **Can Bus**.

This library is designed for microcontrollers such as **ESP32**, and similar platforms. It allows your device to communicate directly with a vehicle that uses the **Can Bus**.  
If you're curious about which types of data are supported, you can find a full list of features below.

You can also see my other car projects:
1. [Тhis](https://github.com/muki01/I-K_Bus) project is for BMW with I/K bus system. 
2. [Тhis](https://github.com/muki01/OBD2_CAN_Bus_Reader) project is for Cars with CAN Bus.
3. [Тhis](https://github.com/muki01/OBD2_K-line_Reader) project is for Cars with ISO9141 and ISO14230 protocols.
4. [Тhis](https://github.com/muki01/OBD2_CAN_Bus_Library) is my OBD2 CAN Bus Communication Library for Arduino IDE.
5. [Тhis](https://github.com/muki01/OBD2_KLine_Library) is my OBD2 K-Line Communication Library for Arduino IDE.
<!--6. [Тhis](https://github.com/muki01/I-K_Bus_Library) is my I/K Bus Communication Library for Arduino IDE.-->

---

## ❓ Does Your Vehicle Support Can Bus?

Before using this library, it's important to confirm whether your vehicle supports the **Can Bus** protocol.

Can Bus vehicles typically have **Pin 6 and Pin 14** on the OBD-II connector connected.
If your vehicle’s OBD-II connector has **Pins 7 connected**, it uses the **K-Line** protocol instead of Can Bus.

✅ **Pin 6 and 14 (CAN bus)**: Your vehicle uses CAN — this library will work.  
❌ **Pin 7 (K-Line)**: Your vehicle likely supports ISO 9141 or ISO 14230 (KWP2000) — consider a different library.  


### Example photos of OBD2 Connector
<p>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20KLine.jpg" width=40%>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20CanBus.jpg" width=40%>
</p>

In the first image, the OBD2 socket includes pin 7, which indicates it operates using the K-Line protocol.
In the second image, pins 6 and 14 are present, meaning it uses the CAN Bus protocol.

---

## ✨ Features

- Supports **11BIT** and **29BIT**, **250KBPS** and **500KBPS**
- **Automatic protocol detection**
- Read real-time sensor values
- Read and clear **stored and pending DTCs**
- Retrieve vehicle info (VIN, calibration IDs, etc.)
- **Mode 06** support (on-board test results)
- Debug output for easier development
- Customizable delays and request intervals
- Works with ESP32 and similar platforms

---

## 📡 Supported OBD-II Modes

| Mode | Description                                      |
|------|--------------------------------------------------|
| 01   | Read current live data (sensor values)           |
| 02   | Read freeze frame data                           |
| 03   | Read stored Diagnostic Trouble Codes (DTCs)      |
| 04   | Clear DTCs and MIL reset                         |
| 05   | Oxygen sensor test results                       |
| 06   | On-board monitoring test results                 |
| 07   | Read pending Diagnostic Trouble Codes            |
| 09   | Retrieve vehicle information (VIN, calibration)  |

---

### 📊 Typical Data Rates

Each protocol has its own timing characteristics, which affect how many responses you can expect per second when reading data from the ECU. The values below reflect **actual performance measurements** based on this library’s real-world testing.

| Protocol     | Average Responses per Second |
|--------------|-------------------------------|
| 250KBPS      |  Not Tested                   |
| 500KBPS      | ~100 responses/sec            |

> 🔎 Note: Tese values represent average conditions based on real-world testing. The actual throughput can vary depending on the ECU’s internal processing time, the specific data being requested (e.g. PID type), and system latency.

---

## 🛠️ Schematics for Communication

<img src="https://github.com/muki01/OBD2_CAN_Bus_Library/blob/main/images/TJA1050%20Schematic.png" width=70%>

---
