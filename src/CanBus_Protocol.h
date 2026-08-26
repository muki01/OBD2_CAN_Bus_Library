/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * Protocol layer: the shared type vocabulary, the protocol configuration
 * structure, protocol selection, request building and the connection logic.
 *
 * Everything that describes the shape of a request (functional address, query
 * length, mode, PID) and everything the connection needs (11 / 29 bit
 * identifiers, 250 / 500 kbit/s, retry counting) is owned by this class - the
 * core below only moves frames.
 *
 * The shared enums (OBD2CanProtocol, OBD2CanIdLength, OBD2CanBitrate) live at
 * the top of this file. They describe WHAT the bus looks like, so they belong
 * to the protocol layer - but the core, the ECU files and the user's sketch all
 * speak this vocabulary, which is why they sit above the class instead of
 * inside it.
 *
 * The default values of every protocol are collected at the top of
 * CanBus_Protocol.cpp, one OBD2CanProtocolConfig table per protocol.
 * getProtocolConfig() below returns the requested one.
 *
 * ISO 15765-4 defines exactly four combinations - 11 or 29 bit identifiers at
 * 250 or 500 kbit/s - and those are the four this layer tries in Automatic
 * mode.
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

#ifndef CANBUS_PROTOCOL_H
#define CANBUS_PROTOCOL_H

#include <Arduino.h>

#include "OBD2_CanBus_Core.h"
#include "CanBus_Functions.h"

// ==================== Shared Types ====================

// Supported protocols.
//
// Every value here is one of the four combinations defined by ISO 15765-4:
// the identifier length and the bit rate together. They are listed in the order
// Automatic tries them - 500 kbit/s first, because it is by far the most common
// on modern cars.
enum OBD2CanProtocol {
  CAN_Automatic = 0,     // Tries the four standard combinations until one answers
  CAN_11bit_500k = 1,    // ISO 15765-4: 11 bit identifiers, 500 kbit/s (most common)
  CAN_29bit_500k = 2,    // ISO 15765-4: 29 bit identifiers, 500 kbit/s
  CAN_11bit_250k = 3,    // ISO 15765-4: 11 bit identifiers, 250 kbit/s
  CAN_29bit_250k = 4,    // ISO 15765-4: 29 bit identifiers, 250 kbit/s
  CAN_Custom = 5,        // No preset - set the identifiers and bit rate yourself
  CAN_None = 6           // No protocol selected
};

// Identifier length. 11 bit uses the $7DF / $7E8 pair, 29 bit uses
// $18DB33F1 / $18DAF1xx.
enum OBD2CanIdLength {
  CanId_11bit = 11,
  CanId_29bit = 29
};

// Bus speed in kbit/s.
enum OBD2CanBitrate {
  CanBitrate_250k = 250,
  CanBitrate_500k = 500
};

// ==================== Protocol Configuration ====================

// All default settings of one protocol
struct OBD2CanProtocolConfig {
  const char* name;              // Protocol name
  OBD2CanIdLength idLength;      // 11 or 29 bit identifiers
  OBD2CanBitrate bitrate;        // Bus speed
  uint32_t requestId;            // Functional request identifier
  uint32_t responseId;           // Expected response identifier
  uint32_t responseMask;         // Which bits of the response id have to match
};

// Returns the default settings of the requested protocol (nullptr if unknown)
const OBD2CanProtocolConfig* getProtocolConfig(OBD2CanProtocol protocol);

class CanBus_Protocol : public CanBus_Core {
 public:
  using CanBus_Core::CanBus_Core;

  void setProtocol(OBD2CanProtocol protocol);
  void setProtocol(uint8_t protocolId);
  OBD2CanProtocol getConnectedProtocol();
  const char* getProtocolName(OBD2CanProtocol protocol);

  // ---- Request layout (for setting things by hand instead of using a preset) ----
  void setIdLength(OBD2CanIdLength idLength);
  void setRequestId(uint32_t id);
  void setResponseId(uint32_t id, uint32_t mask = 0xFFFFFFFF);

  // Opens the bus and confirms an ECU answers. In Automatic mode the four
  // standard combinations are tried in turn.
  bool connect();

  // ---- Reading and connection status ----
  // The core reads any frame that arrives. Deciding whether it is addressed to
  // us and whether the connection is still alive happens here; when a subclass
  // calls readData() both happen automatically.
  uint8_t readData();

  bool isConnected();
  void updateConnectionStatus(bool messageReceived);
  void setConnectionStatus(bool status);
  void setMaxRetryCount(uint8_t count);  // 0 = disabled (the connection never drops)

  // Builds an OBD2 query frame (query length, mode, PID) and sends it.
  bool writeData(uint8_t mode, uint8_t pid);

 protected:
  bool _tryProtocol(OBD2CanProtocol protocol);
  void applyProtocolPresets(OBD2CanProtocol protocol);

  // ---- Request layout (writeData uses these) ----
  OBD2CanIdLength _idLength = CanId_11bit;
  uint32_t _requestId = 0x7DF;
  uint32_t _responseId = 0x7E8;
  uint32_t _responseMask = 0xFFFFFFFF;

  // ---- Connection status ----
  bool connectionStatus = false;
  uint8_t unreceivedDataCount = 0;
  uint8_t _maxRetryCount = 3;

  // ---- Protocol status ----
  OBD2CanProtocol selectedProtocol = CAN_Automatic;  // What the user selected
  OBD2CanProtocol connectedProtocol = CAN_None;      // What actually answered
  OBD2CanProtocol currentProtocol = CAN_None;        // The one currently in use
};

#endif  // CANBUS_PROTOCOL_H
