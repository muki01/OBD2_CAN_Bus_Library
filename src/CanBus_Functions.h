/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * Shared helper functions.
 *
 * These are pure functions: give them bytes, they give you a bool or a String
 * back. They touch no CAN driver, no protocol state and no object - so they are
 * free functions here instead of class members. Any layer (core, protocol, ECU
 * files, the user's sketch) may call them.
 *
 * What lives here:
 *   - Frame comparison
 *   - Byte array conversion / decoding (hex string, ASCII, DTC code)
 *
 * The POLICY (which frame is compared against what, which response counts as
 * ours) is NOT here: that is per-connection state and lives in CanBus_Core /
 * CanBus_Protocol.
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

#ifndef CANBUS_FUNCTIONS_H
#define CANBUS_FUNCTIONS_H

#include <Arduino.h>

#include "OBD2_CanBus_Core.h"  // canMessage

// ----------------------------------- Compare -----------------------------------

// Compares two CAN frames: identifier, data length and payload must all match.
// Where either frame came from is of no concern here.
bool compareData(const canMessage& message1, const canMessage& message2);

// Compares a frame against a raw driver frame (e.g. the last response).
bool compareData(const canMessage& message, const twai_message_t& received);

// ----------------------------------- Conversion -----------------------------------

// Turns two DTC bytes into a readable error code such as "P0123"
String decodeDTC(uint8_t b1, uint8_t b2);

// Is the value present in the array
bool isInArray(const uint8_t* dataArray, uint8_t length, uint8_t value);

// Converts a byte array to an upper case hex string (e.g. {0x0A, 0xF3} -> "0AF3")
String convertBytesToHexString(const uint8_t* dataArray, uint8_t length);

// Converts the printable ASCII characters of a byte array to a string (VIN, Calibration ID)
String convertHexToAscii(const uint8_t* dataArray, uint8_t length);

#endif  // CANBUS_FUNCTIONS_H
