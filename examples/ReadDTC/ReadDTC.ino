#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  //RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Read DTCs Example");

  CanBus.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  CanBus.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: 11b250. 11b500, 29b250, 29b500)
  CanBus.setReadTimeout(200);       // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
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
