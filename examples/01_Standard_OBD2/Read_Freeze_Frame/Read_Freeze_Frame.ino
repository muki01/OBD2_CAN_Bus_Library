/*
 * OBD2 CAN Bus - Read Freeze Frame
 *
 * Reads the freeze frame (Mode 02) - the sensor snapshot the ECU stored the
 * moment a fault appeared. If no fault is stored, the ECU answers nothing.
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

  Serial.println(F("=== OBD2 CAN Bus | Freeze Frame ==="));
  Serial.println(F("==================================="));
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

  float rpm         = CanBus.getFreezeFrame(0x0C);  // PID 0x0C - Engine RPM
  Serial.print(F("RPM         : ")); Serial.print(rpm);         Serial.println(F(" rpm"));

  float coolantTemp = CanBus.getFreezeFrame(0x05);  // PID 0x05 - Coolant Temperature
  Serial.print(F("Coolant Temp: ")); Serial.print(coolantTemp); Serial.println(F(" C"));

  float speed       = CanBus.getFreezeFrame(0x0D);  // PID 0x0D - Vehicle Speed
  Serial.print(F("Speed       : ")); Serial.print(speed);       Serial.println(F(" km/h"));
  Serial.println(F("----------------------------------------"));

  delay(1000);
}
