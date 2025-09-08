#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  // RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Get Vehicle Info Example");

  CanBus.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  CanBus.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: 11b250. 11b500, 29b250, 29b500)
  CanBus.setReadTimeout(200);       // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
  if (CanBus.initOBD2()) {
    String VIN = CanBus.getVehicleInfo(0x02);
    Serial.print("VIN: "), Serial.println(VIN);

    String CalibrationID = CanBus.getVehicleInfo(0x04);
    Serial.print("CalibrationID: "), Serial.println(CalibrationID);

    String CalibrationID_Num = CanBus.getVehicleInfo(0x06);
    Serial.print("CalibrationID_Num: "), Serial.println(CalibrationID_Num);

    Serial.println();
    delay(1000);
  }
}
