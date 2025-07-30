#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  // RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Clear DTCs Example");

  CanBus.setDebug(Serial);          // Enable debug messages on the Serial monitor
  CanBus.setProtocol("Automatic");  // Default protocol Automatic
  //CanBus.setWriteDelay(5);
  //CanBus.setDataRequestInterval(60);

  Serial.println("OBD2 Starting.");
}

void loop() {
  if (CanBus.initOBD2()) {
    CanBus.clearDTC();
    delay(1000);
  }
}
