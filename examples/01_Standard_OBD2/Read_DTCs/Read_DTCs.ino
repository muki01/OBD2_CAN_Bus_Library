/*
 * OBD2 CAN Bus - Read DTCs
 *
 * Reads stored (Mode 03) and pending (Mode 07) Diagnostic Trouble Codes
 * from the vehicle ECU and prints them as readable codes (P0123 style).
 *
 * Protocol  : Standard OBD2 over CAN (ISO 15765-4)
 * Bus       : 11 or 29 bit identifiers, 250 or 500 kbit/s (auto-detected)
 * Board     : ESP32 only - the bus runs on the built-in TWAI controller
 * Tested On : Generic OBD2-compliant vehicles
 *
 * Reference : For a full list of OBD2 PIDs, visit:
 *             https://en.wikipedia.org/wiki/OBD-II_PIDs
 *
 * Wiring: a CAN transceiver (TJA1050, SN65HVD230 or similar) between the OBD2
 * connector (pin 6 = CAN-H, pin 14 = CAN-L) and the pins configured below.
 */

#include "OBD2_CanBus.h"         // core: driver + protocol layer
#include "ecus/OBD2_Standard.h"  // standard OBD2 diagnostics -> OBD2_CanBus

OBD2_CanBus CanBus;

#define CAN_RX_PIN        12
#define CAN_TX_PIN        13
#define OBD_DEBUG_SERIAL  Serial

void setup() {
  Serial.begin(115200);
  while (!Serial);

  CanBus.setPins(CAN_RX_PIN, CAN_TX_PIN);

  // -- Optional Settings --
  CanBus.setDebug(OBD_DEBUG_SERIAL);   // Enable debug output to Serial
  CanBus.setProtocol(CAN_Automatic);   // CAN_Automatic, CAN_11bit_500k, CAN_29bit_500k,
                                       // CAN_11bit_250k, CAN_29bit_250k, CAN_Custom
  CanBus.setReadTimeout(200);          // Max time (ms) to wait for a response

  Serial.println(F("=== OBD2 CAN Bus | Read DTCs ==="));
  Serial.println(F("================================"));
}

void loop() {
  if (!CanBus.isConnected()) {
    Serial.println(F("Connecting to ECU..."));
    if (!CanBus.connect()) {
      Serial.println(F("Connection failed. Retrying..."));
      delay(2000);
      return;
    }
    Serial.print(F("Connection established: "));
    Serial.println(CanBus.getProtocolName(CanBus.getConnectedProtocol()));
    Serial.println(F("----------------------------------------"));
  }

  // -- Stored DTCs (Mode 03) --
  uint8_t storedCount = CanBus.readStoredDTCs();
  Serial.print(F("Stored DTCs: "));
  if (storedCount > 0) {
    Serial.println(storedCount);
    for (uint8_t i = 0; i < storedCount; i++) {
      Serial.print(F("  > ")); Serial.println(CanBus.getStoredDTC(i));
    }
  } else {
    Serial.println(F("None"));
  }

  // -- Pending DTCs (Mode 07) --
  uint8_t pendingCount = CanBus.readPendingDTCs();
  Serial.print(F("Pending DTCs: "));
  if (pendingCount > 0) {
    Serial.println(pendingCount);
    for (uint8_t i = 0; i < pendingCount; i++) {
      Serial.print(F("  > ")); Serial.println(CanBus.getPendingDTC(i));
    }
  } else {
    Serial.println(F("None"));
  }
  Serial.println(F("----------------------------------------"));

  delay(5000);
}
