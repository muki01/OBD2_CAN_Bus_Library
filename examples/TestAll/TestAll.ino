#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13); //RX, TX

canMessage initMSG = { 0x18DA40F1, 0, 1, 3, { 0x02, 0x10, 0x03 } };
canMessage lockDors = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x21, 0x03, 0xFF } };
canMessage unlockDors = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x20, 0x03, 0xFF } };
canMessage leftBlinker = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x09, 0x03, 0xFF } };
canMessage rightBlinker = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x0A, 0x03, 0xFF } };
canMessage fogLights = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x12, 0x03, 0xFF } };
canMessage highBeams = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x11, 0x03, 0xFF } }; //Not sure if this is correct
canMessage lowBeams = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x10, 0x03, 0xFF } };  //Not sure if this is correct
canMessage wiperOn = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x30, 0x03, 0x01 } };
canMessage wiperOff = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x30, 0x03, 0xFF } };
canMessage leftWindowsDown = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x57, 0x03, 0xFF } };
canMessage leftWindowsUp = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x06, 0x03, 0xFF } };
canMessage leftWindowsStop = { 0x18DA40F1, 0, 1, 6, { 0x05, 0x2F, 0x50, 0x57, 0x03, 0x00 } };

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus PID Reading Example");

  CanBus.setDebug(Serial);  // Enable debug messages on the Serial monitor
  //CanBus.setProtocol("ISO14230_Fast");  // Set communication protocol to ISO14230 (also known as KWP2000 Fast Init)
  //CanBus.setWriteDelay(5);              // Optional: delay between bytes when writing to OBD (in milliseconds)
  //CanBus.setDataRequestInterval(60);    // Optional: delay between data requests (in milliseconds)

  Serial.println("OBD2 Starting.");

  while (!CanBus.initOBD2()) {
    delay(500);
  }
}

void loop() {
  int rpm = CanBus.getLiveData(0x0C);  // PID 0x0C = Engine RPM
  Serial.print("Engine RPM: ");
  Serial.println(rpm);
  delay(1000);

  int coolantTemp = CanBus.getLiveData(0x05);  // PID 0x05 = Coolant Temperature
  Serial.print("Coolant Temp: ");
  Serial.print(coolantTemp);
  Serial.println(" C");
  delay(1000);

  //int speed = CanBus.getPID(0x01, 0x0D); // Alternative
  int speed = CanBus.getLiveData(0x0D);  // PID 0x0D = Vehicle Speed
  Serial.print("Vehicle Speed: ");
  Serial.print(speed);
  Serial.println(" km/h");
  delay(1000);

  int dtcCount = CanBus.readDTCs(0x03);
  Serial.print("DTC Count: ");
  Serial.println(dtcCount);

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

  CanBus.writeRawData(initMSG);
  delay(50);
  CanBus.writeRawData(lowBeams);

  Serial.println();  // Print a blank line between readings
  delay(1000);
}
