#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  // RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Get Vehicle Info Example");

  CanBus.setDebug(Serial);          // Enable debug messages on the Serial monitor
  CanBus.setProtocol("Automatic");  // Default protocol Automatic
  //CanBus.setWriteDelay(5);
  //CanBus.setDataRequestInterval(60);

  Serial.println("OBD2 Starting.");
}

void loop() {
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
