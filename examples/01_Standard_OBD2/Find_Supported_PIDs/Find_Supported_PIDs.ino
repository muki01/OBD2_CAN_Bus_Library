/*
 * OBD2 CAN Bus - Find Supported PIDs
 *
 * Queries and lists all PIDs supported by the vehicle ECU across the
 * standard OBD2 service modes. Useful for discovering what data the ECU can
 * provide before implementing a diagnostic application.
 *
 * Scans : Mode 01 (Live Data)      | Mode 02 (Freeze Frame)
 *         Mode 05 (Oxygen Sensors) | Mode 06 (Other Components)
 *         Mode 08 (On-Board Ctrl)  | Mode 09 (Vehicle Info)
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

// Prints the supported PID list for a given mode.
void printSupportedPIDs(const char* label, uint8_t mode, uint8_t count) {
  Serial.print(F("[MODE 0")); Serial.print(mode, HEX); Serial.print(F("] "));
  Serial.print(label); Serial.print(F(": "));

  if (count == 0) {
    Serial.println(F("Not supported"));
    return;
  }
  for (uint8_t i = 0; i < count; i++) {
    uint8_t pid = CanBus.getSupportedData(mode, i);
    if (pid < 0x10) Serial.print('0');
    Serial.print(pid, HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  CanBus.setPins(CAN_RX_PIN, CAN_TX_PIN);

  // -- Optional Settings --
  CanBus.setDebug(OBD_DEBUG_SERIAL);   // Enable debug output to Serial
  CanBus.setProtocol(CAN_Automatic);   // CAN_Automatic, CAN_11bit_500k, CAN_29bit_500k,
                                       // CAN_11bit_250k, CAN_29bit_250k, CAN_Custom
  CanBus.setReadTimeout(200);          // Max time (ms) to wait for a response

  Serial.println(F("=== OBD2 CAN Bus | Supported PID Scanner ==="));
  Serial.println(F("Scanning Modes: 01, 02, 05, 06, 08, 09"));
  Serial.println(F("============================================"));
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

  printSupportedPIDs("Live Data",        0x01, CanBus.readSupportedLiveData());          delay(500);
  printSupportedPIDs("Freeze Frame",     0x02, CanBus.readSupportedFreezeFrame());       delay(500);
  printSupportedPIDs("Oxygen Sensors",   0x05, CanBus.readSupportedOxygenSensors());     delay(500);
  printSupportedPIDs("Other Components", 0x06, CanBus.readSupportedOtherComponents());   delay(500);
  printSupportedPIDs("On-Board Ctrl",    0x08, CanBus.readSupportedOnBoardComponents()); delay(500);
  printSupportedPIDs("Vehicle Info",     0x09, CanBus.readSupportedVehicleInfo());

  Serial.println(F("----------------------------------------"));
  delay(10000);
}
