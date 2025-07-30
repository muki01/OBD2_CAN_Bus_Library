#include "OBD2_CanBus.h"  // Include the library for OBD2 CanBus communication

OBD2_CanBus CanBus(12, 13);  //RX, TX

void setup() {
  Serial.begin(1000000);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 CanBus Get Live Data Example");

  CanBus.setDebug(Serial);          // Enable debug messages on the Serial monitor
  CanBus.setProtocol("Automatic");  // Default protocol Automatic
  //CanBus.setWriteDelay(5);
  //CanBus.setDataRequestInterval(60);

  Serial.println("OBD2 Starting.");
}

void loop() {
  if (CanBus.initOBD2()) {
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

    Serial.println();  // Print a blank line between readings
  }
}
