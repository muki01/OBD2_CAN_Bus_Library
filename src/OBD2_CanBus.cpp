#include "OBD2_CanBus.h"

OBD2_CanBus::OBD2_CanBus(uint8_t rxPin, uint8_t txPin) : _rxPin(rxPin), _txPin(txPin) {
}

// ----------------------------------- Initialization functions -----------------------------------

bool OBD2_CanBus::initOBD2() {
  if (connectionStatus) return true;

  debugPrintln(F("Initializing OBD2..."));
  struct ProtocolOption {
    const char *name;
    uint8_t bit;
    twai_timing_config_t speed;
  };

  ProtocolOption options[] = {{"11b250", 11, TWAI_TIMING_CONFIG_250KBITS()},
                              {"29b250", 29, TWAI_TIMING_CONFIG_250KBITS()},
                              {"11b500", 11, TWAI_TIMING_CONFIG_500KBITS()},
                              {"29b500", 29, TWAI_TIMING_CONFIG_500KBITS()}};

  for (auto &opt : options) {
    if (selectedProtocol == opt.name || selectedProtocol == "Automatic") {
      CAN_BIT = opt.bit;
      CAN_SPEED = opt.speed;
      debugPrint(F("Trying protocol: "));
      debugPrintln(opt.name);
      if (testConnection()) {
        connectionStatus = true;
        connectedProtocol = opt.name;
        debugPrintln(F("✅ OBD2 connection established with protocol: "));
        debugPrintln(opt.name);
        debugPrintln(F(""));
        return true;
      } else {
        debugPrint(F("❌ Failed to connect with protocol: "));
        debugPrintln(opt.name);
        debugPrintln(F(""));
      }
    }
  }

  return false;
}

bool OBD2_CanBus::testConnection() {
  if (initTWAI()) {
    if (writeData(0x01, 0x00)) {
      if (readData() > 0) {
        return true;
      }
    }
    stopTWAI();
  }
  return false;
}

bool OBD2_CanBus::initTWAI() {
  debugPrintln(F("Setting up TWAI interface..."));

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)_txPin, (gpio_num_t)_rxPin, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  twai_timing_config_t t_config = CAN_SPEED;
  g_config.rx_queue_len = 60;  // Received messages queue size
  g_config.tx_queue_len = 10;  // Transmit messages queue size

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    debugPrintln(F("❌ Driver installation failed."));
    return false;
  }

  if (twai_start() != ESP_OK) {
    debugPrintln(F("❌ TWAI start failed."));
    return false;
  }

  debugPrintln(F("✅ TWAI successfully initialized."));
  return true;
}

void OBD2_CanBus::stopTWAI() {
  debugPrintln(F("Stopping TWAI..."));
  twai_stop();
  twai_driver_uninstall();
}

// ----------------------------------- Basic Read/Write functions -----------------------------------

bool OBD2_CanBus::writeRawData(canMessage msg) {
  debugPrintln(F("Sending Raw Data: "));
  twai_message_t message;

  message.identifier = msg.id;
  message.rtr = msg.rtr;
  message.extd = msg.ide;
  message.data_length_code = msg.length;

  for (int i = 0; i < msg.length; i++) {
    message.data[i] = msg.data[i];
  }

  debugPrint(F("ID: 0x"));
  debugPrintHex(message.identifier);
  debugPrint(F(" RTR: "));
  debugPrintHex(message.rtr);
  debugPrint(F(" IDE: "));
  debugPrintHex(message.extd);
  // debugPrint(F(" Length: "));
  // debugPrint(message.data_length_code);
  debugPrint(F(" Data: "));
  for (int i = 0; i < message.data_length_code; i++) {
    debugPrintHex(message.data[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));

  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    // debugPrintln(F("✅ CAN message sent successfully."));
    return true;
  } else {
    debugPrintln(F("❌ Error sending CAN message!"));
    return false;
  }
}

bool OBD2_CanBus::writeData(uint8_t mode, uint8_t pid) {
  twai_message_t message;
  uint8_t quearyLength = 0x02;  // Default query length

  if (mode == read_storedDTCs || mode == read_pendingDTCs || mode == clear_DTCs) {
    quearyLength = 0x01;
  } else if (mode == read_FreezeFrame || mode == test_OxygenSensors) {
    quearyLength = 0x03;
  } else {
    quearyLength = 0x02;
  }

  if (CAN_BIT == 29) {
    message.identifier = 0x18DB33F1;
    message.extd = 1;
  } else if (CAN_BIT == 11) {
    message.identifier = 0x7DF;
    message.extd = 0;
  } else {
    debugPrintln(F("Unsupported CAN_BIT value!"));
    return false;
  }

  message.rtr = 0;                 // Data frame
  message.data_length_code = 8;    // 8_byte data frame
  message.data[0] = quearyLength;  // Query length
  message.data[1] = mode;          // Mode
  message.data[2] = pid;           // PID
  message.data[3] = 0x00;          // Parameter
  message.data[4] = 0x00;
  message.data[5] = 0x00;
  message.data[6] = 0x00;
  message.data[7] = 0x00;

  debugPrint(F("Sending Data: ID: 0x"));
  debugPrintHex(message.identifier);
  // debugPrint(F(" RTR: "));
  // debugPrintHex(message.rtr);
  // debugPrint(F(" IDE: "));
  // debugPrintHex(message.extd);
  // debugPrint(F(" Length: "));
  // debugPrint(message.data_length_code);
  debugPrint(F(" Data: "));
  for (int i = 0; i < message.data_length_code; i++) {
    debugPrintHex(message.data[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));

  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    // debugPrintln(F("✅ CAN message sent successfully."));
    return true;
  } else {
    debugPrintln(F("❌ Error sending CAN message!"));
    return false;
  }
}

uint8_t OBD2_CanBus::readData() {
  // debugPrintln(F("Reading..."));
  twai_message_t response;
  unsigned long start_time = millis();

  while (millis() - start_time < _readTimeout) {
    if (twai_receive(&response, pdMS_TO_TICKS(_readTimeout)) == ESP_OK) {
      if (response.identifier == 0x18DAF110 || response.identifier == 0x18DAF111 || response.identifier == 0x7E8) {
        updateConnectionStatus(true);
        if (memcmp(&resultBuffer, &response, sizeof(twai_message_t)) != 0) {
          memcpy(&resultBuffer, &response, sizeof(twai_message_t));
        }

        debugPrint(F("✅ Received Data: ID: 0x"));
        debugPrintHex(response.identifier);
        // debugPrint(F(", RTR: "));
        // debugPrintHex(response.rtr);
        // debugPrint(F(", EID: "));
        // debugPrintHex(response.extd);
        // debugPrint(F(", (DLC): "));
        // debugPrint((response.data_length_code));
        debugPrint(F(", Data: "));
        for (int i = 0; i < response.data_length_code; i++) {
          debugPrintHex(response.data[i]);
          debugPrint(F(" "));
        }
        debugPrintln(F(""));
        return response.data_length_code;
      }
    } else {
      debugPrintln(F("❌ Not Received any Message!"));
    }
  }
  debugPrintln(F("❌ OBD2 Timeout!"));
  updateConnectionStatus(false);
  return 0;
}

bool OBD2_CanBus::readAndCompareData(const canMessage &target) {
  // debugPrintln(F("Reading All..."));
  twai_message_t response;
  unsigned long start_time = millis();

  while (millis() - start_time < _readTimeout) {
    if (twai_receive(&response, pdMS_TO_TICKS(_readTimeout)) == ESP_OK) {
      debugPrint(F("Received Data: ID: 0x"));
      debugPrintHex(response.identifier);
      debugPrint(F(", Data: "));
      for (int i = 0; i < response.data_length_code; i++) {
        debugPrintHex(response.data[i]);
        debugPrint(F(" "));
      }
      debugPrintln(F(""));

      if (response.identifier == target.id && response.rtr == target.rtr && response.extd == target.ide &&
          response.data_length_code == target.length) {
        bool match = true;
        for (int i = 0; i < target.length; i++) {
          if (response.data[i] != target.data[i]) {
            match = false;
            break;
          }
        }
        if (match) {
          debugPrintln(F("✅ Matching message found."));
          return true;
        }
      }

    } else {
      debugPrintln(F("❌ Not Received any Message!"));
    }
  }

  debugPrintln(F("⛔ Message did not match, timeout."));
  return false;
}

// ----------------------------------- Live Data -----------------------------------

float OBD2_CanBus::getLiveData(uint8_t pid) {
  return getPID(read_LiveData, pid);
}

float OBD2_CanBus::getFreezeFrame(uint8_t pid) {
  return getPID(read_FreezeFrame, pid);
}

float OBD2_CanBus::getPID(uint8_t mode, uint8_t pid) {
  writeData(mode, pid);
  int len = readData();

  if (len <= 0) return -1;                     // Data not received
  if (resultBuffer.data[2] != pid) return -2;  // Unexpected PID

  uint8_t A = 0, B = 0, C = 0, D = 0;

  if (mode == read_LiveData) {
    int dataBytesLen = len - 3;
    A = (dataBytesLen >= 1) ? resultBuffer.data[3] : 0;
    B = (dataBytesLen >= 2) ? resultBuffer.data[4] : 0;
    C = (dataBytesLen >= 3) ? resultBuffer.data[5] : 0;
    D = (dataBytesLen >= 4) ? resultBuffer.data[6] : 0;
  } else if (mode == read_FreezeFrame) {
    int dataBytesLen = len - 4;
    A = (dataBytesLen >= 1) ? resultBuffer.data[4] : 0;
    B = (dataBytesLen >= 2) ? resultBuffer.data[5] : 0;
    C = (dataBytesLen >= 3) ? resultBuffer.data[6] : 0;
    D = (dataBytesLen >= 4) ? resultBuffer.data[7] : 0;
  }

  switch (pid) {
    case 0x01:                                      // Monitor Status Since DTC Cleared (bit encoded)
    case 0x02:                                      // Monitor Status Since DTC Cleared (bit encoded)
    case 0x03:                                      // Fuel System Status (bit encoded)
      return A;                                     //
    case 0x04:                                      // Engine Load (%)
      return A * 100.0f / 255.0f;                   //
    case 0x05:                                      // Coolant Temperature (°C)
      return A - 40.0f;                             //
    case 0x06:                                      // Short Term Fuel Trim Bank 1 (%)
    case 0x07:                                      // Long Term Fuel Trim Bank 1 (%)
    case 0x08:                                      // Short Term Fuel Trim Bank 2 (%)
    case 0x09:                                      // Long Term Fuel Trim Bank 2 (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x0A:                                      // Fuel Pressure (kPa)
      return A * 3.0f;                              //
    case 0x0B:                                      // Intake Manifold Absolute Pressure (kPa)
      return A;                                     //
    case 0x0C:                                      // RPM
      return ((A * 256.0f) + B) / 4.0f;             //
    case 0x0D:                                      // Speed (km/h)
      return A;                                     //
    case 0x0E:                                      // Timing Advance (°)
      return A / 2.0f - 64.0f;                      //
    case 0x0F:                                      // Intake Air Temperature (°C)
      return A - 40.0f;                             //
    case 0x10:                                      // MAF Flow Rate (grams/sec)
      return ((A * 256.0f) + B) / 100.0f;           //
    case 0x11:                                      // Throttle Position (%)
      return A * 100.0f / 255.0f;                   //
    case 0x12:                                      // Commanded Secondary Air Status (bit encoded)
    case 0x13:                                      // Oxygen Sensors Present 2 Banks (bit encoded)
      return A;                                     //
    case 0x14:                                      // Oxygen Sensor 1A Voltage (V, %)
    case 0x15:                                      // Oxygen Sensor 2A Voltage (V, %)
    case 0x16:                                      // Oxygen Sensor 3A Voltage (V, %)
    case 0x17:                                      // Oxygen Sensor 4A Voltage (V, %)
    case 0x18:                                      // Oxygen Sensor 5A Voltage (V, %)
    case 0x19:                                      // Oxygen Sensor 6A Voltage (V, %)
    case 0x1A:                                      // Oxygen Sensor 7A Voltage (V, %)
    case 0x1B:                                      // Oxygen Sensor 8A Voltage (V, %)
      return A / 200.0f;                            // Voltage
    case 0x1C:                                      // OBD Standards This Vehicle Conforms To (bit encoded)
    case 0x1D:                                      // Oxygen Sensors Present 4 Banks (bit encoded)
    case 0x1E:                                      // Auxiliary Input Status (bit encoded)
      return A;                                     //
    case 0x1F:                                      // Run Time Since Engine Start (seconds)
    case 0x21:                                      // Distance Traveled With MIL On (km)
      return (A * 256.0f) + B;                      //
    case 0x22:                                      // Fuel Rail Pressure (kPa)
      return ((A * 256.0f) + B) * 0.079f;           //
    case 0x23:                                      // Fuel Rail Gauge Pressure (kPa)
      return ((A * 256.0f) + B) / 10.0f;            //
    case 0x24:                                      // Oxygen Sensor 1B (ratio, voltage)
    case 0x25:                                      // Oxygen Sensor 2B (ratio, voltage)
    case 0x26:                                      // Oxygen Sensor 3B (ratio, voltage)
    case 0x27:                                      // Oxygen Sensor 4B (ratio, voltage)
    case 0x28:                                      // Oxygen Sensor 5B (ratio, voltage)
    case 0x29:                                      // Oxygen Sensor 6B (ratio, voltage)
    case 0x2A:                                      // Oxygen Sensor 7B (ratio, voltage)
    case 0x2B:                                      // Oxygen Sensor 8B (ratio, voltage)
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x2C:                                      // Commanded EGR (%)
      return A * 100.0f / 255.0f;                   //
    case 0x2D:                                      // EGR Error (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x2E:                                      // Commanded Evaporative Purge (%)
    case 0x2F:                                      // Fuel Tank Level Input (%)
      return A * 100.0f / 255.0f;                   //
    case 0x30:                                      // Warm-ups Since Codes Cleared (count)
      return A;                                     //
    case 0x31:                                      // Distance Traveled Since Codes Cleared (km)
      return (A * 256.0f) + B;                      //
    case 0x32:                                      // Evap System Vapor Pressure (Pa)
      return ((A * 256.0f) + B) / 4.0f;             //
    case 0x33:                                      // Absolute Barometric Pressure (kPa)
      return A;                                     //
    case 0x34:                                      // Oxygen Sensor 1C (current)
    case 0x35:                                      // Oxygen Sensor 2C
    case 0x36:                                      // Oxygen Sensor 3C
    case 0x37:                                      // Oxygen Sensor 4C
    case 0x38:                                      // Oxygen Sensor 5C
    case 0x39:                                      // Oxygen Sensor 6C
    case 0x3A:                                      // Oxygen Sensor 7C
    case 0x3B:                                      // Oxygen Sensor 8C
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x3C:                                      // Catalyst Temperature Bank 1 Sensor 1 (°C)
    case 0x3D:                                      // Catalyst Temperature Bank 2 Sensor 1 (°C)
    case 0x3E:                                      // Catalyst Temperature Bank 1 Sensor 2 (°C)
    case 0x3F:                                      // Catalyst Temperature Bank 2 Sensor 2 (°C)
      return ((A * 256.0f) + B) / 10.0f - 40.0f;    //
    case 0x41:                                      // Monitor status this drive cycle (bit encoded)
      return A;                                     //
    case 0x42:                                      // Control module voltage (V)
      return ((A * 256.0f) + B) / 1000.0f;          //
    case 0x43:                                      // Absolute load value (%)
      return ((A * 256.0f) + B) * 100.0f / 255.0f;  //
    case 0x44:                                      // Fuel/Air commanded equivalence ratio (lambda)
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x45:                                      // Relative throttle position (%)
      return A * 100.0f / 255.0f;                   //
    case 0x46:                                      // Ambient air temp (°C)
      return A - 40.0f;                             //
    case 0x47:                                      // Absolute throttle position B (%)
    case 0x48:                                      // Absolute throttle position C (%)
    case 0x49:                                      // Accelerator pedal position D (%)
    case 0x4A:                                      // Accelerator pedal position E (%)
    case 0x4B:                                      // Accelerator pedal position F (%)
    case 0x4C:                                      // Commanded throttle actuator (%)
      return A * 100.0f / 255.0f;                   //
    case 0x4D:                                      // Time run with MIL on (min)
    case 0x4E:                                      // Time since trouble codes cleared (min)
      return (A * 256.0f) + B;                      //
    case 0x4F:                                      // Max values for sensors (ratio, V, mA, kPa)
    case 0x50:                                      // Maximum value for air flow rate from mass air flow sensor (g/s)
    case 0x51:                                      // Fuel Type (bit encoded)
      return A;                                     //
    case 0x52:                                      // Ethanol fuel (%)
      return A * 100.0f / 255.0f;                   //
    case 0x53:                                      // Absolute evap system pressure (kPa)
      return ((A * 256.0f) + B) / 200.0f;           //
    case 0x54:                                      // Evap system vapor pressure (Pa)
      return (A * 256.0f) + B;                      //
    case 0x55:                                      // Short term secondary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x56:                                      // Long term primary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x57:                                      // Short term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
    case 0x58:                                      // Long term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x59:                                      // Fuel rail absolute pressure (kPa)
      return ((A * 256.0f) + B) * 10.0f;            //
    case 0x5A:                                      // Relative accelerator pedal position (%)
    case 0x5B:                                      // Hybrid battery pack remaining life (%)
      return A * 100.0f / 255.0f;                   //
    case 0x5C:                                      // Engine oil temperature (°C)
      return A - 40.0f;                             //
    case 0x5D:                                      // Fuel injection timing (°)
      return ((A * 256.0f) + B) / 128.0f - 210.0f;  //
    case 0x5E:                                      // Engine fuel rate (L/h)
      return ((A * 256.0f) + B) / 20.0f;            //
    case 0x5F:                                      // Emission requirements to which vehicle is designed (bit encoded)
      return A;                                     //
    case 0x61:                                      // Driver's demand engine - percent torque (%)
    case 0x62:                                      // Actual engine - percent torque (%)
      return A - 125.0f;                            //
    case 0x63:                                      // Engine reference torque (Nm)
      return (A * 256.0f) + B;                      //
    default:                                        //
      return -4;                                    // Unknown PID
  }
}

// ----------------------------------- DTCs -----------------------------------

uint8_t OBD2_CanBus::readStoredDTCs() {
  return readDTCs(0x03);
}

uint8_t OBD2_CanBus::readPendingDTCs() {
  return readDTCs(0x07);
}

uint8_t OBD2_CanBus::readDTCs(uint8_t mode) {
  // Request:  03
  // example Response: 07 43 01 70 01 34 00 00
  // example Response: 03 43 00 00
  int dtcCount = 0;
  String *targetArray = nullptr;

  if (mode == read_storedDTCs) {
    targetArray = storedDTCBuffer;
  } else if (mode == read_pendingDTCs) {
    targetArray = pendingDTCBuffer;
  } else {
    return -1;  // Invalid mode
  }

  writeData(mode, 0x00);

  int len = readData();
  if (len >= 3) {
    for (int i = 0; i < len; i += 2) {
      uint8_t b1 = resultBuffer.data[2 + i * 2];
      uint8_t b2 = resultBuffer.data[2 + i * 2 + 1];

      if (b1 == 0 && b2 == 0) break;

      targetArray[dtcCount++] = decodeDTC(b1, b2);
    }
  }

  return dtcCount;
}

String OBD2_CanBus::getStoredDTC(uint8_t index) {
  if (index >= 0) return storedDTCBuffer[index];
  return "";
}

String OBD2_CanBus::getPendingDTC(uint8_t index) {
  if (index >= 0) return pendingDTCBuffer[index];
  return "";
}

bool OBD2_CanBus::clearDTC() {
  writeData(clear_DTCs, 0x00);
  int len = readData();
  if (len >= 3) {
    if (resultBuffer.data[1] == 0x44) {
      return true;
    }
  }
  return false;
}

// ----------------------------------- Vehicle Information -----------------------------------

String OBD2_CanBus::getVehicleInfo(uint8_t pid) {          //Not Tested
  // Request: 09 02
  // example Response: 07 49 02 01 00 00 00 31
  //                   07 49 02 02 41 31 4A 43
  //                   07 49 02 03 35 34 34 34
  //                   07 49 02 04 52 37 32 35
  //                   07 49 02 05 32 33 36 37

  uint8_t dataArray[64];
  int messageCount;
  int arrayNum = 0;

  if (pid == 0x02) {
    messageCount = 5;
  } else if (pid == 0x04 || pid == 0x06) {
    if (pid == 0x04) {
      writeData(read_VehicleInfo, read_ID_Length);
    } else if (pid == 0x06) {
      writeData(read_VehicleInfo, read_ID_Num_Length);
    } else {
      return "";
    }

    if (readData()) {
      messageCount = resultBuffer.data[3];
    } else {
      return "";
    }
  }

  writeData(read_VehicleInfo, pid);

  if (readData()) {
    for (int j = 0; j < messageCount; j++) {
      if (pid == 0x02 && j == 0) {
        dataArray[arrayNum++] = resultBuffer.data[7];
        continue;
      }
      for (int i = 1; i <= 4; i++) {
        dataArray[arrayNum++] = resultBuffer.data[i + 3 + j * 8];
      }
    }
  }

  if (pid == 0x02 || pid == 0x04) {
    return convertHexToAscii(dataArray, arrayNum);
  } else if (pid == 0x06) {
    return convertBytesToHexString(dataArray, arrayNum);
  }
  return "";
}

// ----------------------------------- Supported PIDs -----------------------------------

uint8_t OBD2_CanBus::readSupportedLiveData() {
  return readSupportedData(read_LiveData);
}

uint8_t OBD2_CanBus::readSupportedFreezeFrame() {
  return readSupportedData(read_FreezeFrame);
}

uint8_t OBD2_CanBus::readSupportedOxygenSensors() {
  return readSupportedData(test_OxygenSensors);
}

uint8_t OBD2_CanBus::readSupportedOtherComponents() {
  return readSupportedData(test_OtherComponents);
}

uint8_t OBD2_CanBus::readSupportedOnBoardComponents() {
  return readSupportedData(control_OnBoardComponents);
}

uint8_t OBD2_CanBus::readSupportedVehicleInfo() {
  return readSupportedData(read_VehicleInfo);
}

uint8_t OBD2_CanBus::readSupportedData(uint8_t mode) {
  int supportedCount = 0;
  int pidIndex = 0;
  int startByte = 0;
  int arraySize = 32;  // Size of supported data arrays
  uint8_t *targetArray = nullptr;

  if (mode == read_LiveData) {  // Mode 01
    startByte = 3;
    targetArray = supportedLiveData;
  } else if (mode == read_FreezeFrame) {  // Mode 02
    startByte = 4;
    targetArray = supportedFreezeFrame;
  } else if (mode == test_OxygenSensors) {  // Mode 05
    startByte = 4;
    targetArray = supportedOxygenSensor;
  } else if (mode == test_OtherComponents) {  // Mode 06
    startByte = 3;
    targetArray = supportedOtherComponents;
  } else if (mode == control_OnBoardComponents) {  // Mode 08
    startByte = 3;
    targetArray = supportedControlComponents;
  } else if (mode == read_VehicleInfo) {  // Mode 09
    startByte = 3;
    targetArray = supportedVehicleInfo;
  } else {
    return -1;  // Invalid mode
  }

  uint8_t pidCmds[] = {SUPPORTED_PIDS_1_20, SUPPORTED_PIDS_21_40, SUPPORTED_PIDS_41_60, SUPPORTED_PIDS_61_80, SUPPORTED_PIDS_81_100};

  for (int n = 0; n < 5; n++) {
    // Group 0 is always processed, others must be checked
    if (n != 0 && !isInArray(targetArray, 32, pidCmds[n])) break;

    writeData(mode, pidCmds[n]);
    if (readData() && resultBuffer.data[1] == 0x40 + mode) {
      for (int i = 0; i < 4; i++) {
        uint8_t value = resultBuffer.data[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) targetArray[supportedCount++] = pidIndex + 1;
          pidIndex++;
        }
      }
    }
  }

  return supportedCount;
}

uint8_t OBD2_CanBus::getSupportedData(uint8_t mode, uint8_t index) {
  if (mode == 0x01) {
    if (index >= 0) return supportedLiveData[index];
  } else if (mode == 0x02) {
    if (index >= 0) return supportedFreezeFrame[index];
  } else if (mode == 0x06) {
    if (index >= 0) return supportedOtherComponents[index];
  } else if (mode == 0x09) {
    if (index >= 0) return supportedVehicleInfo[index];
  }
  return 0;
}

// ----------------------------------- Helper Functions -----------------------------------

void OBD2_CanBus::updateConnectionStatus(bool messageReceived) {
  if (messageReceived) {
    unreceivedDataCount = 0;
    // if (!connectionStatus) {
    //   connectionStatus = true;
    //   debugPrintln(F("✅ Connection established."));
    // }
  } else {
    if (!connectionStatus) return;  // No need to update if not connected

    unreceivedDataCount++;
    debugPrint(F("⚠️ Not received data: "));
    debugPrintln(String(unreceivedDataCount).c_str());
    if (unreceivedDataCount > 2 && connectionStatus) {
      stopTWAI();
      connectionStatus = false;
      unreceivedDataCount = 0;
      debugPrintln(F("⛔ Connection lost."));
    }
  }
}

void OBD2_CanBus::setReadTimeout(uint16_t timeoutMs) {
  _readTimeout = timeoutMs;
}

void OBD2_CanBus::setProtocol(const String &protocolName) {
  selectedProtocol = protocolName;
  connectionStatus = false;  // Reset connection status
  connectedProtocol = "";    // Reset connected protocol
  stopTWAI();
  debugPrintln(("Protocol set to: " + selectedProtocol).c_str());
}

String OBD2_CanBus::decodeDTC(uint8_t input_byte1, uint8_t input_byte2) {
  String ErrorCode = "";
  const char type_lookup[4] = {'P', 'C', 'B', 'U'};
  const char digit_lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  ErrorCode += type_lookup[(input_byte1 >> 6) & 0x03];
  ErrorCode += digit_lookup[(input_byte1 >> 4) & 0x03];
  ErrorCode += digit_lookup[input_byte1 & 0x0F];
  ErrorCode += digit_lookup[(input_byte2 >> 4) & 0x0F];
  ErrorCode += digit_lookup[input_byte2 & 0x0F];

  return ErrorCode;
}

bool OBD2_CanBus::isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value) {
  for (int i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
}

String OBD2_CanBus::convertHexToAscii(const uint8_t *dataArray, int length) {
  String asciiString = "";
  for (int i = 0; i < length; i++) {
    uint8_t b = dataArray[i];
    if (b >= 0x20 && b <= 0x7E) {  // Printable ASCII range
      asciiString += (char)b;
    }
  }
  return asciiString;
}

String OBD2_CanBus::convertBytesToHexString(const uint8_t *dataArray, int length) {
  String hexString = "";
  for (int i = 0; i < length; i++) {
    if (dataArray[i] < 0x10) hexString += "0";  // Pad leading zero
    hexString += String(dataArray[i], HEX);
  }
  hexString.toUpperCase();
  return hexString;
}

// ----------------------------------- Debug Functions -----------------------------------

void OBD2_CanBus::setDebug(Stream &serial) {
  _debugSerial = &serial;
}

void OBD2_CanBus::debugPrint(const char *msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void OBD2_CanBus::debugPrint(const __FlashStringHelper *msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void OBD2_CanBus::debugPrintln(const char *msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void OBD2_CanBus::debugPrintln(const __FlashStringHelper *msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void OBD2_CanBus::debugPrintHex(uint32_t val) {
  if (_debugSerial) _debugSerial->printf("%02lX", val);
}

void OBD2_CanBus::debugPrintHexln(uint32_t val) {
  if (_debugSerial) {
    debugPrintHex(val);
    _debugSerial->println();
  }
}