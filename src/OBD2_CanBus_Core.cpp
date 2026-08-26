/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * CAN core implementation: TWAI driver / pin handling, raw send - receive and
 * the debug output. Request layout (mode, PID, functional address) and the
 * connection logic are not here - they belong to CanBus_Protocol.cpp.
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

#include "OBD2_CanBus_Core.h"

CanBus_Core::CanBus_Core() {
}

CanBus_Core::CanBus_Core(uint8_t rxPin, uint8_t txPin) : _rxPin(rxPin), _txPin(txPin) {
}

void CanBus_Core::setPins(uint8_t rx, uint8_t tx) {
  _rxPin = rx;
  _txPin = tx;
  debugPrint(F("✅ Pins updated to: RX=")); debugPrint(rx); debugPrint(F(", TX=")); debugPrintln(tx);
}

void CanBus_Core::setBitrate(uint16_t kbitPerSecond) {
  _bitrate = kbitPerSecond;
  debugPrint(F("✅ Bitrate updated to: ")); debugPrint(kbitPerSecond); debugPrintln(F(" kbit/s"));
}

// The driver wants a timing struct, not a number. Only the two rates defined by
// ISO 15765-4 are produced here; anything else falls back to 500 kbit/s.
twai_timing_config_t CanBus_Core::timingConfig() {
  if (_bitrate == 250) {
    twai_timing_config_t t = TWAI_TIMING_CONFIG_250KBITS();
    return t;
  }
  twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();
  return t;
}

// ----------------------------------- Driver -----------------------------------

bool CanBus_Core::begin() {
  if (_isDriverRunning) end();

  debugPrintln(F("🔄 Setting up TWAI interface..."));

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)_txPin, (gpio_num_t)_rxPin, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  twai_timing_config_t t_config = timingConfig();
  g_config.rx_queue_len = 60;  // Received messages queue size
  g_config.tx_queue_len = 10;  // Transmit messages queue size

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    debugPrintln(F("❌ Driver installation failed."));
    return false;
  }

  if (twai_start() != ESP_OK) {
    debugPrintln(F("❌ TWAI start failed."));
    twai_driver_uninstall();
    return false;
  }

  _isDriverRunning = true;
  debugPrintln(F("✅ TWAI successfully initialized."));
  return true;
}

void CanBus_Core::end() {
  if (!_isDriverRunning) return;

  debugPrintln(F("🔄 Stopping TWAI..."));
  twai_stop();
  twai_driver_uninstall();
  _isDriverRunning = false;
}

bool CanBus_Core::isDriverRunning() {
  return _isDriverRunning;
}

// ----------------------------------- Basic Read/Write -----------------------------------

// Puts the frame on the bus exactly as given. How that frame was built is the
// protocol layer's job - this only sends.
bool CanBus_Core::writeRawData(canMessage msg) {
  twai_message_t message = {};

  message.identifier = msg.id;
  message.rtr = msg.rtr;
  message.extd = msg.ide;
  message.data_length_code = msg.length;
  memcpy(message.data, msg.data, msg.length);

  debugPrint(F("\n➡️ Sending Data: "));
  debugPrintFrame(message);

  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    return true;
  }

  debugPrintln(F("❌ Error sending CAN message!"));
  return false;
}

// Reads the next frame and returns its data length. Every frame is accepted -
// filtering the ones addressed to us is the protocol layer's job.
uint8_t CanBus_Core::readMessage() {
  twai_message_t response;
  unsigned long startTime = millis();

  while (millis() - startTime < _readTimeout) {
    if (twai_receive(&response, pdMS_TO_TICKS(_readTimeout)) == ESP_OK) {
      memcpy(&resultBuffer, &response, sizeof(twai_message_t));

      debugPrint(F("✅ Received Data: "));
      debugPrintFrame(response);
      return response.data_length_code;
    }
  }

  return 0;
}

void CanBus_Core::setReadTimeout(uint16_t timeoutMs) {
  _readTimeout = timeoutMs;
  debugPrint(F("✅ Read timeout set to: ")); debugPrint(timeoutMs); debugPrintln(F(" ms"));
}

twai_message_t* CanBus_Core::getResultBuffer() {
  return &resultBuffer;
}

uint8_t CanBus_Core::getResultLength() {
  return resultBuffer.data_length_code;
}

// ----------------------------------- Debug -----------------------------------

void CanBus_Core::setDebug(Stream& serial) {
  _debugSerial = &serial;
}

void CanBus_Core::debugPrint(const char* msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void CanBus_Core::debugPrint(const __FlashStringHelper* msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void CanBus_Core::debugPrint(uint32_t val) {
  if (_debugSerial) _debugSerial->print(val);
}

void CanBus_Core::debugPrintln(const char* msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void CanBus_Core::debugPrintln(const __FlashStringHelper* msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void CanBus_Core::debugPrintln(uint32_t val) {
  if (_debugSerial) _debugSerial->println(val);
}

void CanBus_Core::debugPrintHex(uint32_t val) {
  if (_debugSerial) _debugSerial->printf("%02lX", val);
}

void CanBus_Core::debugPrintHexln(uint32_t val) {
  if (_debugSerial) {
    debugPrintHex(val);
    _debugSerial->println();
  }
}

// "ID: 7E8, Data: 03 41 0C 1A F8 00 00 00"
void CanBus_Core::debugPrintFrame(const twai_message_t& message) {
  if (_debugSerial == nullptr) return;

  debugPrint(F("ID: "));
  debugPrintHex(message.identifier);
  debugPrint(F(", Data: "));
  for (uint8_t i = 0; i < message.data_length_code; i++) {
    debugPrintHex(message.data[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));
}
