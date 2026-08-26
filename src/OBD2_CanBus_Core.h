/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * CAN core: the ESP32 TWAI driver, the pins, the bit rate and moving frames -
 * plus the debug output.
 *
 * This class knows no protocols and no diagnostic services. It installs the
 * driver, puts a frame on the bus exactly as given and reads whatever comes
 * back. Deciding WHICH frame to build and WHICH answer belongs to us is the
 * protocol layer's job (see "CanBus_Protocol.h").
 *
 * So everything that describes a REQUEST (the query length, the mode, the PID,
 * the functional address) or the CONNECTION (11 / 29 bit, 250 / 500 kbit/s
 * selection, retries) lives in CanBus_Protocol, not here. What stays here is
 * what the microcontroller itself needs: the pins, the driver, the read
 * timeout and the debug output.
 *
 * This header depends on nothing but Arduino.h and the ESP32 TWAI driver.
 *
 * Developed by: Muksin Muksin (MukiTech)
 * GitHub: https://github.com/muki01/OBD2_CAN_Bus_Library
 * Email: muksin.muksin04@gmail.com
 *
 * LICENSE: DUAL-LICENSED
 * 1. PERSONAL/RESEARCH: Free for non-commercial use.
 * 2. COMMERCIAL: Mandatory paid license required for any for-profit usage.
 * Copyright (c) 2025 MukiTech. All rights reserved.
 */

#ifndef OBD2_CANBUS_CORE_H
#define OBD2_CANBUS_CORE_H

#include <Arduino.h>
#include <driver/twai.h>

// A raw CAN frame, in the shape a sketch writes it. The library converts it to
// the driver's own type when sending.
typedef struct {
  uint32_t id;       // Identifier (11 or 29 bit)
  uint32_t rtr;      // Remote transmission request (0 = data frame)
  uint32_t ide;      // Extended identifier flag (0 = 11 bit, 1 = 29 bit)
  uint8_t length;    // Number of data bytes (0..8)
  uint8_t data[8];   // Payload
} canMessage;

class CanBus_Core {
 public:
  CanBus_Core();
  CanBus_Core(uint8_t rxPin, uint8_t txPin);

  void setPins(uint8_t rx, uint8_t tx);
  void setBitrate(uint16_t kbitPerSecond);  // 250 or 500
  void setDebug(Stream& serial);

  // Installs and starts the TWAI driver with the current pins and bit rate.
  bool begin();
  void end();
  bool isDriverRunning();

  // Puts a frame on the bus exactly as given - nothing is added.
  // Building the frame is the protocol layer's job.
  bool writeRawData(canMessage msg);

  // Reads the next frame from the bus and returns its data length (0 on
  // timeout). Every frame is accepted - deciding whether it is addressed to us
  // is the protocol layer's job.
  uint8_t readMessage();

  void setReadTimeout(uint16_t timeoutMs);

  // Data Access Functions
  twai_message_t* getResultBuffer();
  uint8_t getResultLength();

 protected:
  uint8_t _rxPin = 4;
  uint8_t _txPin = 5;
  uint16_t _bitrate = 500;  // kbit/s
  uint16_t _readTimeout = 200;
  bool _isDriverRunning = false;

  Stream* _debugSerial = nullptr;
  twai_message_t resultBuffer = {};

  twai_timing_config_t timingConfig();

  void debugPrint(const char* msg);
  void debugPrint(const __FlashStringHelper* msg);
  void debugPrint(uint32_t val);
  void debugPrintln(const char* msg);
  void debugPrintln(const __FlashStringHelper* msg);
  void debugPrintln(uint32_t val);
  void debugPrintHex(uint32_t val);    // Hexadecimal output
  void debugPrintHexln(uint32_t val);  // Hexadecimal + newline
  void debugPrintFrame(const twai_message_t& message);
};

#endif  // OBD2_CANBUS_CORE_H
