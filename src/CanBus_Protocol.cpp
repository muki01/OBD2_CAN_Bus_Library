/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * Protocol layer implementation: protocol tables, protocol selection, request
 * building and the connection logic.
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

#include "CanBus_Protocol.h"

// ==================================================================================
//                              PROTOCOL DEFAULTS
// ==================================================================================
// All default settings of every protocol are here. To add a new protocol:
// 1) Add a new <PROTOCOL>_CONFIG table below
// 2) Add the new protocol to the OBD2CanProtocol enum (CanBus_Protocol.h)
// 3) Add a single line to the getProtocolConfig() switch at the bottom
// ==================================================================================

// ----------------------------- 11 bit identifiers -----------------------------
// The functional request goes to $7DF. ECUs answer on $7E8..$7EF, so the mask
// only checks the upper bits - $7E8 is the engine, $7E9 the gearbox, and so on.
static const OBD2CanProtocolConfig CAN_11BIT_500K_CONFIG = {
    "CAN 11bit 500k",  // name
    CanId_11bit,       // idLength
    CanBitrate_500k,   // bitrate
    0x7DF,             // requestId  (functional)
    0x7E8,             // responseId
    0x7F8              // responseMask (accepts 7E8..7EF)
};

static const OBD2CanProtocolConfig CAN_11BIT_250K_CONFIG = {
    "CAN 11bit 250k",
    CanId_11bit,
    CanBitrate_250k,
    0x7DF,
    0x7E8,
    0x7F8};

// ----------------------------- 29 bit identifiers -----------------------------
// The functional request goes to $18DB33F1. ECUs answer on $18DAF1xx, where the
// low byte is the source ECU, so the mask leaves that byte free.
static const OBD2CanProtocolConfig CAN_29BIT_500K_CONFIG = {
    "CAN 29bit 500k",
    CanId_29bit,
    CanBitrate_500k,
    0x18DB33F1,        // requestId  (functional)
    0x18DAF100,        // responseId
    0x1FFFFF00         // responseMask (accepts 18DAF1xx)
};

static const OBD2CanProtocolConfig CAN_29BIT_250K_CONFIG = {
    "CAN 29bit 250k",
    CanId_29bit,
    CanBitrate_250k,
    0x18DB33F1,
    0x18DAF100,
    0x1FFFFF00};

const OBD2CanProtocolConfig* getProtocolConfig(OBD2CanProtocol protocol) {
  switch (protocol) {
    case CAN_11bit_500k: return &CAN_11BIT_500K_CONFIG;
    case CAN_29bit_500k: return &CAN_29BIT_500K_CONFIG;
    case CAN_11bit_250k: return &CAN_11BIT_250K_CONFIG;
    case CAN_29bit_250k: return &CAN_29BIT_250K_CONFIG;
    default: return nullptr;  // Automatic, Custom, None
  }
}

// ----------------------------------- Auto Detect Order -----------------------------------
// Automatic mode walks these in order. 500 kbit/s comes first because it is by
// far the most common on modern cars, and 11 bit before 29 bit for the same
// reason - the first entry connects on most vehicles.
static const OBD2CanProtocol AUTO_DETECT_ORDER[] = {
    CAN_11bit_500k,
    CAN_29bit_500k,
    CAN_11bit_250k,
    CAN_29bit_250k};

static const uint8_t AUTO_DETECT_COUNT =
    sizeof(AUTO_DETECT_ORDER) / sizeof(AUTO_DETECT_ORDER[0]);

// ----------------------------------- Protocol Selection -----------------------------------

const char* CanBus_Protocol::getProtocolName(OBD2CanProtocol protocol) {
  const OBD2CanProtocolConfig* config = getProtocolConfig(protocol);
  if (config != nullptr) return config->name;

  switch (protocol) {
    case CAN_Custom: return "Custom";
    case CAN_None: return "None";
    default: return "Automatic";
  }
}

void CanBus_Protocol::setProtocol(OBD2CanProtocol protocol) {
  selectedProtocol = protocol;
  connectionStatus = false;
  connectedProtocol = CAN_None;
  end();  // the driver has to be reinstalled with the new bit rate

  debugPrint(F("✅ Protocol set to: "));
  debugPrintln(getProtocolName(protocol));

  applyProtocolPresets(protocol);
}

void CanBus_Protocol::setProtocol(uint8_t protocolId) {
  setProtocol((OBD2CanProtocol)protocolId);
}

OBD2CanProtocol CanBus_Protocol::getConnectedProtocol() {
  return connectedProtocol;
}

// All default settings come from the protocol tables at the top of this file.
void CanBus_Protocol::applyProtocolPresets(OBD2CanProtocol protocol) {
  const OBD2CanProtocolConfig* config = getProtocolConfig(protocol);
  if (config == nullptr) return;  // Automatic / Custom / None

  currentProtocol = protocol;
  setIdLength(config->idLength);
  setBitrate(config->bitrate);
  setRequestId(config->requestId);
  setResponseId(config->responseId, config->responseMask);
}

// ----------------------------------- Request Layout -----------------------------------
// When no preset is applied (the Custom protocol) the user sets these by hand.

void CanBus_Protocol::setIdLength(OBD2CanIdLength idLength) {
  _idLength = idLength;
}

void CanBus_Protocol::setRequestId(uint32_t id) {
  _requestId = id;
}

void CanBus_Protocol::setResponseId(uint32_t id, uint32_t mask) {
  _responseId = id;
  _responseMask = mask;
}

// ----------------------------------- Connecting -----------------------------------

bool CanBus_Protocol::connect() {
  if (connectionStatus) return true;

  debugPrintln(F("🔍 Starting Connection Sequence..."));

  // If a protocol was chosen by hand nothing is tried: its settings were
  // already loaded by setProtocol(), so it is used directly.
  if (selectedProtocol != CAN_Automatic) return _tryProtocol(selectedProtocol);

  debugPrintln(F("🛠️ Automatic mode: Testing standard protocols..."));

  for (uint8_t i = 0; i < AUTO_DETECT_COUNT; i++) {
    if (_tryProtocol(AUTO_DETECT_ORDER[i])) {
      debugPrint(F("🎉 SUCCESS! Auto-detected: "));
      debugPrintln(getProtocolName(connectedProtocol));
      return true;
    }
  }

  debugPrintln(F("❌ No Protocol Matched. Initialization Failed."));
  return false;
}

// One attempt: install the driver with this protocol's settings and ask for
// Mode 01 PID 00 (supported PIDs) - the one request every OBD2 ECU answers.
bool CanBus_Protocol::_tryProtocol(OBD2CanProtocol protocol) {
  if (protocol == CAN_None) return false;

  applyProtocolPresets(protocol);

  debugPrint(F("\n🔄 Trying protocol: "));
  debugPrintln(getProtocolName(protocol));

  if (!begin()) return false;

  if (writeData(0x01, 0x00) && readData() > 0) {
    connectedProtocol = protocol;
    connectionStatus = true;
    unreceivedDataCount = 0;
    debugPrintln(F("✅ Connected."));
    return true;
  }

  debugPrintln(F("❌ No answer."));
  end();
  return false;
}

// ----------------------------------- Write -----------------------------------

// Builds the OBD2 query according to the active protocol settings.
//
// The frame is always 8 bytes: [length] [mode] [pid] then padding. The length
// byte counts only the meaningful bytes after it, and that count depends on the
// mode - the DTC modes carry no PID at all.
bool CanBus_Protocol::writeData(uint8_t mode, uint8_t pid) {
  canMessage message = {};

  uint8_t queryLength;
  if (mode == 0x03 || mode == 0x04 || mode == 0x07 || mode == 0x0A) {
    queryLength = 0x01;  // mode only, no PID
  } else if (mode == 0x02 || mode == 0x05) {
    queryLength = 0x03;  // mode + PID + frame number
  } else {
    queryLength = 0x02;  // mode + PID
  }

  message.id = _requestId;
  message.ide = (_idLength == CanId_29bit) ? 1 : 0;
  message.rtr = 0;
  message.length = 8;
  message.data[0] = queryLength;
  message.data[1] = mode;
  message.data[2] = pid;
  // data[3..7] stay zero

  return writeRawData(message);
}

// ----------------------------------- Read / Connection State -----------------------------------

// The core hands over any frame that arrives. Whether it is addressed to us and
// whether the connection is still alive are decided here.
uint8_t CanBus_Protocol::readData() {
  unsigned long startTime = millis();

  // Frames from other modules share the bus, so keep reading until one of them
  // matches our response identifier or the read timeout runs out.
  while (millis() - startTime < _readTimeout) {
    uint8_t length = CanBus_Core::readMessage();
    if (length == 0) break;  // nothing arrived at all

    if ((resultBuffer.identifier & _responseMask) == (_responseId & _responseMask)) {
      updateConnectionStatus(true);
      return length;
    }
  }

  debugPrintln(F("❌ OBD2 Timeout!"));
  updateConnectionStatus(false);
  return 0;
}

bool CanBus_Protocol::isConnected() {
  return connectionStatus;
}

void CanBus_Protocol::setConnectionStatus(bool status) {
  connectionStatus = status;
}

void CanBus_Protocol::setMaxRetryCount(uint8_t count) {
  _maxRetryCount = count;
}

void CanBus_Protocol::updateConnectionStatus(bool messageReceived) {
  if (messageReceived) {
    unreceivedDataCount = 0;  // A response arrived, reset the counter
    return;
  }

  if (!connectionStatus) return;  // If we are not connected there is nothing to count

  // Drop detection only happens while maxRetryCount > 0
  if (_maxRetryCount == 0) return;

  unreceivedDataCount++;
  debugPrint(F("⚠️ No answer: "));
  debugPrintln(unreceivedDataCount);

  if (unreceivedDataCount >= _maxRetryCount) {
    end();
    connectionStatus = false;
    connectedProtocol = CAN_None;
    unreceivedDataCount = 0;
    debugPrintln(F("⛔ Critical: Connection lost after multiple retries."));
  }
}
