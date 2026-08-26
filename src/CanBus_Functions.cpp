/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * Shared helper functions implementation.
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

#include "CanBus_Functions.h"

// ----------------------------------- Compare -----------------------------------

bool compareData(const canMessage& message1, const canMessage& message2) {
  if (message1.id != message2.id) return false;
  if (message1.length != message2.length) return false;
  return (memcmp(message1.data, message2.data, message1.length) == 0);
}

bool compareData(const canMessage& message, const twai_message_t& received) {
  if (message.id != received.identifier) return false;
  if (message.length != received.data_length_code) return false;
  return (memcmp(message.data, received.data, message.length) == 0);
}

// ----------------------------------- Conversion -----------------------------------

String decodeDTC(uint8_t b1, uint8_t b2) {
  String ErrorCode = "";
  static const char typeLookup[4] = {'P', 'C', 'B', 'U'};
  static const char digitLookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  ErrorCode += typeLookup[(b1 >> 6) & 0x03];
  ErrorCode += digitLookup[(b1 >> 4) & 0x03];
  ErrorCode += digitLookup[b1 & 0x0F];
  ErrorCode += digitLookup[(b2 >> 4) & 0x0F];
  ErrorCode += digitLookup[b2 & 0x0F];

  return ErrorCode;
}

bool isInArray(const uint8_t* dataArray, uint8_t length, uint8_t value) {
  for (uint8_t i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
}

String convertBytesToHexString(const uint8_t* dataArray, uint8_t length) {
  String hexString = "";
  for (uint8_t i = 0; i < length; i++) {
    if (dataArray[i] < 0x10) hexString += "0";  // Pad leading zero
    hexString += String(dataArray[i], HEX);
  }
  hexString.toUpperCase();
  return hexString;
}

String convertHexToAscii(const uint8_t* dataArray, uint8_t length) {
  String asciiString = "";
  for (uint8_t i = 0; i < length; i++) {
    uint8_t b = dataArray[i];
    if (b >= 0x20 && b <= 0x7E) {  // Printable ASCII range
      asciiString += (char)b;
    }
  }
  return asciiString;
}
