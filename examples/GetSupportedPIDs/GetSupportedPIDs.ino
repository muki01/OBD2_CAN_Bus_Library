#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  // RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Get Supported PIDs Example");

  CanBus.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  CanBus.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: 11b250. 11b500, 29b250, 29b500)
  CanBus.setReadTimeout(200);       // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
  if (CanBus.initOBD2()) {
    int liveDataLength = CanBus.readSupportedLiveData();
    if (liveDataLength > 0) {
      Serial.print("LiveData: ");
      for (int i = 0; i < liveDataLength; i++) {
        byte supported = CanBus.getSupportedData(0x01, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int freezeFrameLength = CanBus.readSupportedFreezeFrame();
    if (freezeFrameLength > 0) {
      Serial.print("FreezeFrame: ");
      for (int i = 0; i < freezeFrameLength; i++) {
        byte supported = CanBus.getSupportedData(0x02, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int componentMonitoringLength = CanBus.readSupportedComponentMonitoring();
    if (componentMonitoringLength > 0) {
      Serial.print("Component Monitoring: ");
      for (int i = 0; i < componentMonitoringLength; i++) {
        byte supported = CanBus.getSupportedData(0x06, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int vehicleInfoLength = CanBus.readSupportedVehicleInfo();
    if (vehicleInfoLength > 0) {
      Serial.print("VehicleInfo: ");
      for (int i = 0; i < vehicleInfoLength; i++) {
        byte supported = CanBus.getSupportedData(0x09, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);
    // KLine.readSupportedData(0x01);
    // delay(1000);
  }
}
