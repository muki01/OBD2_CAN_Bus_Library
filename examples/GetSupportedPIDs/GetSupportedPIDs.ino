#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  // RX, TX

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Get Supported PIDs Example");

  CanBus.setDebug(Serial);       // Enable debug messages on the Serial monitor
  CanBus.setProtocol("29b500");  // Set communication protocol to 29 BIT 500 KBPS
  // CanBus.setWriteDelay(5);
  // CanBus.setDataRequestInterval(60);

  Serial.println("OBD2 Starting.");
}

void loop() {
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
