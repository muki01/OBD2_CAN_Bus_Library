/*
 * OBD2_CanBus Library - MukiTech
 * ------------------------------
 * A professional Arduino library for vehicle diagnostics over the CAN bus.
 *
 * SUPPORTED PROTOCOLS
 *   ISO 15765-4 defines four combinations, and all four are supported:
 *     CAN 11 bit @ 500 kbit/s    request $7DF      response $7E8..$7EF
 *     CAN 29 bit @ 500 kbit/s    request $18DB33F1 response $18DAF1xx
 *     CAN 11 bit @ 250 kbit/s
 *     CAN 29 bit @ 250 kbit/s
 *   Custom      No preset at all - the sketch defines the identifiers itself
 *   Automatic   Tries the four standard combinations until one answers
 *
 * The bus runs on the ESP32 TWAI controller, so this library is ESP32 only.
 * A CAN transceiver (TJA1050, SN65HVD230 or similar) is required between the
 * microcontroller and the vehicle.
 *
 * FILE STRUCTURE
 * --------------
 *   OBD2_CanBus_Core.h/.cpp    CanBus_Core - Microcontroller side only: the TWAI driver,
 *                              the pins, the bit rate, sending a ready made frame /
 *                              receiving frames, the read timeout and the debug output.
 *                              This header depends on nothing but Arduino.h and twai.h.
 *   CanBus_Protocol.h/.cpp     CanBus_Protocol - Protocol layer. The top of the header holds
 *                              the shared type vocabulary every layer speaks (OBD2CanProtocol,
 *                              OBD2CanIdLength, OBD2CanBitrate). The top of the .cpp holds the
 *                              default settings of EVERY protocol (identifier length, bit rate,
 *                              request and response identifiers) as one table per protocol.
 *                              The rest is protocol selection, request building, response
 *                              filtering and the connection logic.
 *   CanBus_Functions.h/.cpp    Shared helpers as plain free functions, no object and no state:
 *                              frame comparison (compareData) and conversion / decoding
 *                              (decodeDTC, isInArray, convertBytesToHexString,
 *                              convertHexToAscii). Any layer may call them.
 *   ecus/OBD2_Standard.h/.cpp  OBD2_Standard - Standard OBD2 diagnostics: live data, freeze
 *                              frame, DTCs, vehicle info, supported PIDs. Defined by SAE J1979,
 *                              so it works on any car - but it lives under "ecus/" like every
 *                              other diagnostic vocabulary and is only compiled when a sketch
 *                              includes it.
 *   ecus/<Ecu>.h/.cpp          One self contained file pair per ECU. Holds its own connection
 *                              settings, its own service requests and its own response layout
 *                              (which byte means what). Manufacturer services are NOT shared
 *                              between ECUs on purpose - addresses, sub functions and
 *                              identifiers differ from car to car.
 *
 * The core (this file and the three next to it) is always compiled. Everything under "ecus/"
 * is opt in: including this header alone gives you the connection, not a diagnostic
 * vocabulary. Pick the one you need:
 *
 *   #include "OBD2_CanBus.h"
 *   #include "ecus/OBD2_Standard.h"   // standard OBD2, any car -> OBD2_CanBus
 *
 * Unused ECU tables never reach the flash this way.
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

#ifndef OBD2_CANBUS_H
#define OBD2_CANBUS_H

// The order matters: Core is the leaf, Functions builds on it, and Protocol
// defines the shared enums on top of both.
#include "OBD2_CanBus_Core.h"
#include "CanBus_Functions.h"
#include "CanBus_Protocol.h"

#endif  // OBD2_CANBUS_H
