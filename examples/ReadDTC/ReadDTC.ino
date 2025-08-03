#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  //RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Read DTCs Example");

  CanBus.setDebug(Serial);          // Enable debug messages on the Serial monitor
  CanBus.setProtocol("Automatic");  // Default protocol Automatic
  CanBus.setReadTimeout(200);       // Set read timeout to 200 ms

  Serial.println("OBD2 Starting.");
}

void loop() {
  if (CanBus.initOBD2()) {
    int storedDTCLength = CanBus.readStoredDTCs();
    //int storedDTCLength = CanBus.readDTCs(0x03);
    if (storedDTCLength > 0) {
      for (int i = 0; i < storedDTCLength; i++) {
        String dtc = CanBus.getStoredDTC(i);
        Serial.println(dtc);
      }
      Serial.println();
    } else {
      Serial.println("No Stored DTCs");
    }

    int pendingDTCLength = CanBus.readPendingDTCs();
    //int pendingDTCLength = CanBus.readDTCs(0x07);
    if (pendingDTCLength > 0) {
      for (int i = 0; i < pendingDTCLength; i++) {
        String dtc = CanBus.getPendingDTC(i);
        Serial.println(dtc);
      }
      Serial.println();
    } else {
      Serial.println("No Pending DTCs");
    }
    delay(1000);
  }
}
