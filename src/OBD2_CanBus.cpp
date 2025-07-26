#include "OBD2_CanBus.h"

OBD2_CanBus::OBD2_CanBus(uint8_t rxPin, uint8_t txPin) : _rxPin(rxPin), _txPin(txPin) {
}

bool OBD2_CanBus::initOBD2() {
  if (connectionStatus) return true;

  debugPrintln(F("Initializing OBD2..."));
  struct ProtocolOption {
    const char *name;
    int bit;
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
        return true;
      } else {
        debugPrint(F("❌ Failed to connect with protocol: "));
        debugPrintln(opt.name);
      }
    }
  }

  return false;
}

bool OBD2_CanBus::testConnection() {
  debugPrintln(F("Testing OBD2 connection..."));
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

  twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)_txPin, (gpio_num_t)_rxPin, TWAI_MODE_NORMAL);
  general.rx_queue_len = 60;  // Received messages queue size
  general.tx_queue_len = 10;  // Transmit messages queue size
  twai_timing_config_t timing = CAN_SPEED;
  twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&general, &timing, &filter) != ESP_OK) {
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
  delay(100);
}

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
    debugPrintln(F("✅ CAN message sent successfully."));
    return true;
  } else {
    debugPrintln(F("❌ Error sending CAN message!"));
    return false;
  }
}

bool OBD2_CanBus::writeData(byte mode, byte pid) {
  debugPrintln(F("Sending Data: "));
  twai_message_t message;
  byte quearyLength = 0x02;  // Default query length

  if (mode == read_storedDTCs || mode == read_pendingDTCs || mode == clear_DTCs) {
    quearyLength = 0x01;
  } else if (mode == read_FreezeFrame) {
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
  message.data_length_code = 8;    // 8-byte data frame
  message.data[0] = quearyLength;  // Query length
  message.data[1] = mode;          // Mode
  message.data[2] = pid;           // PID
  message.data[3] = 0x00;          // Parameter
  message.data[4] = 0x00;
  message.data[5] = 0x00;
  message.data[6] = 0x00;
  message.data[7] = 0x00;

  debugPrint(F("ID: 0x"));
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

int OBD2_CanBus::readData() {
  // debugPrintln(F("Reading..."));
  twai_message_t response;
  unsigned long start_time = millis();

  while (millis() - start_time < 500) {
    if (twai_receive(&response, pdMS_TO_TICKS(500)) == ESP_OK) {
      if (response.identifier == 0x18DAF110 || response.identifier == 0x18DAF111 || response.identifier == 0x7E8) {
        errors = 0;
        if (memcmp(&resultBuffer, &response, sizeof(twai_message_t)) != 0) {
          memcpy(&resultBuffer, &response, sizeof(twai_message_t));
        }

        debugPrint(F("Received Data: "));
        debugPrint(F("ID: 0x"));
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
      debugPrintln(F("Not Received any Message!"));
    }
  }
  debugPrintln(F("OBD2 Timeout!"));
  errors++;
  if (errors > 2) {
    errors = 0;
    if (connectionStatus) {
      connectionStatus = false;
    }
  }
  return 0;
}

float OBD2_CanBus::getPID(byte mode, byte pid) {
  writeData(mode, pid);

  int len = readData();
  if (len <= 0) {
    return -1;  // Data not received
  }

  if (resultBuffer.data[2] != pid) {
    return -2;  // Unexpected PID
  }

  byte A = 0, B = 0, C = 0, D = 0;

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
    case 0x01:                                        // Monitor Status Since DTC Cleared (bit encoded)
    case 0x02:                                        // Monitor Status Since DTC Cleared (bit encoded)
    case 0x03:                                        // Fuel System Status (bit encoded)
      return A;                                       //
    case 0x04:                                        // Engine Load (%)
      return A * 100 / 255;                           //
    case 0x05:                                        // Coolant Temperature (°C)
      return A - 40;                                  //
    case 0x06:                                        // Short Term Fuel Trim Bank 1 (%)
    case 0x07:                                        // Long Term Fuel Trim Bank 1 (%)
    case 0x08:                                        // Short Term Fuel Trim Bank 2 (%)
    case 0x09:                                        // Long Term Fuel Trim Bank 2 (%)
      return (int)((int8_t)A * 100 / 128);            //
    case 0x0A:                                        // Fuel Pressure (kPa)
      return A * 3;                                   //
    case 0x0B:                                        // Intake Manifold Absolute Pressure (kPa)
      return A;                                       //
    case 0x0C:                                        // RPM
      return ((A * 256) + B) / 4;                     //
    case 0x0D:                                        // Speed (km/h)
      return A;                                       //
    case 0x0E:                                        // Timing Advance (°)
      return (int8_t)A / 2;                           //
    case 0x0F:                                        // Intake Air Temperature (°C)
      return A - 40;                                  //
    case 0x10:                                        // MAF Flow Rate (grams/sec)
      return ((A * 256) + B) / 100;                   //
    case 0x11:                                        // Throttle Position (%)
      return A * 100 / 255;                           //
    case 0x12:                                        // Commanded Secondary Air Status (bit encoded)
    case 0x13:                                        // Oxygen Sensors Present 2 Banks (bit encoded)
      return A;                                       //
    case 0x14:                                        // Oxygen Sensor 1A Voltage (V)
    case 0x15:                                        // Oxygen Sensor 2A Voltage (V)
    case 0x16:                                        // Oxygen Sensor 3A Voltage (V)
    case 0x17:                                        // Oxygen Sensor 4A Voltage (V)
    case 0x18:                                        // Oxygen Sensor 5A Voltage (V)
    case 0x19:                                        // Oxygen Sensor 6A Voltage (V)
    case 0x1A:                                        // Oxygen Sensor 7A Voltage (V)
    case 0x1B:                                        // Oxygen Sensor 8A Voltage (V)
      return A / 200;                                 // Volt to mV (V*1000), float hesap gerekirse fonksiyon
    case 0x1C:                                        // OBD Standards This Vehicle Conforms To (bit encoded)
    case 0x1D:                                        // Oxygen Sensors Present 4 Banks (bit encoded)
    case 0x1E:                                        // Auxiliary Input Status (bit encoded)
      return A;                                       //
    case 0x1F:                                        // Run Time Since Engine Start (seconds)
    case 0x21:                                        // Distance Traveled With MIL On (km)
      return (A * 256) + B;                           //
    case 0x22:                                        // Fuel Rail Pressure (kPa)
    case 0x23:                                        // Fuel Rail Gauge Pressure (kPa)
      return ((A * 256) + B) / 10;                    //
    case 0x24:                                        // Oxygen Sensor 1B (ratio voltage)
    case 0x25:                                        // Oxygen Sensor 2B
    case 0x26:                                        // Oxygen Sensor 3B
    case 0x27:                                        // Oxygen Sensor 4B
    case 0x28:                                        // Oxygen Sensor 5B
    case 0x29:                                        // Oxygen Sensor 6B
    case 0x2A:                                        // Oxygen Sensor 7B
    case 0x2B:                                        // Oxygen Sensor 8B
      return ((A * 256) + B) * 0.0000305 * 1000;      // ratio * 1000 (mV), float önerilir
    case 0x2C:                                        // Commanded EGR (%)
      return A * 100 / 255;                           //
    case 0x2D:                                        // EGR Error (%)
      return (int8_t)A * 100 / 128;                   //
    case 0x2E:                                        // Commanded Evaporative Purge (%)
    case 0x2F:                                        // Fuel Tank Level Input (%)
      return A * 100 / 255;                           //
    case 0x30:                                        // Warm-ups Since Codes Cleared (count)
      return A;                                       //
    case 0x31:                                        // Distance Traveled Since Codes Cleared (km)
      return (A * 256) + B;                           //
    case 0x32: {                                      // Evap System Vapor Pressure (Pa)
      int16_t signedValue = (int16_t)((A << 8) | B);  //
      return signedValue / 4;                         //
    }
    case 0x33:                            // Absolute Barometric Pressure (kPa)
      return A;                           //
    case 0x34:                            // Oxygen Sensor 1C (current)
    case 0x35:                            // Oxygen Sensor 2C
    case 0x36:                            // Oxygen Sensor 3C
    case 0x37:                            // Oxygen Sensor 4C
    case 0x38:                            // Oxygen Sensor 5C
    case 0x39:                            // Oxygen Sensor 6C
    case 0x3A:                            // Oxygen Sensor 7C
    case 0x3B:                            // Oxygen Sensor 8C
      return ((A * 256) + B) * 0.000488;  //
    case 0x3C:                            // Catalyst Temperature Bank 1 Sensor 1 (°C)
    case 0x3D:                            // Catalyst Temperature Bank 2 Sensor 1 (°C)
    case 0x3E:                            // Catalyst Temperature Bank 1 Sensor 2 (°C)
    case 0x3F:                            // Catalyst Temperature Bank 2 Sensor 2 (°C)
      return (A * 256) + B - 40;          //
    default:                              //
      return -4;                          // Unknown PID
  }
}

float OBD2_CanBus::getLiveData(byte pid) {
  return getPID(read_LiveData, pid);
}

float OBD2_CanBus::getFreezeFrame(byte pid) {
  return getPID(read_FreezeFrame, pid);
}

int OBD2_CanBus::readDTCs(byte mode) {
  // Request: C2 33 F1 03 F3
  // example Response: 87 F1 11 43 01 70 01 34 00 00 72
  // example Response: 87 F1 11 43 00 00 72
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
      byte b1 = resultBuffer.data[3 + i * 2];
      byte b2 = resultBuffer.data[3 + i * 2 + 1];

      if (b1 == 0 && b2 == 0) break;

      targetArray[dtcCount++] = decodeDTC(b1, b2);
    }
  }

  return dtcCount;
}

int OBD2_CanBus::readStoredDTCs() {
  return readDTCs(0x03);
}

int OBD2_CanBus::readPendingDTCs() {
  return readDTCs(0x07);
}

String OBD2_CanBus::getStoredDTC(int index) {
  if (index >= 0) return storedDTCBuffer[index];
  return "";
}

String OBD2_CanBus::getPendingDTC(int index) {
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

// String OBD2_CanBus::getVehicleInfo(byte pid) {
//   // Request: C2 33 F1 09 02 F1
//   // example Response: 87 F1 11 49 02 01 00 00 00 31 06
//   //                   87 F1 11 49 02 02 41 31 4A 43 D5
//   //                   87 F1 11 49 02 03 35 34 34 34 A8
//   //                   87 F1 11 49 02 04 52 37 32 35 C8
//   //                   87 F1 11 49 02 05 32 33 36 37 E6

//   byte dataArray[64];
//   int messageCount;
//   int arrayNum = 0;

//   if (pid == 0x02) {
//     messageCount = 5;
//   } else if (pid == 0x04 || pid == 0x06) {
//     if (pid == 0x04) {
//       writeData(read_VehicleInfo, read_ID_Length);
//     } else if (pid == 0x06) {
//       writeData(read_VehicleInfo, read_ID_Num_Length);
//     } else {
//       return "";
//     }

//     if (readData()) {
//       messageCount = resultBuffer[5];
//     } else {
//       return "";
//     }
//   }

//   writeData(read_VehicleInfo, pid);

//   if (readData()) {
//     for (int j = 0; j < messageCount; j++) {
//       if (pid == 0x02 && j == 0) {
//         dataArray[arrayNum++] = resultBuffer[9];
//         continue;
//       }
//       for (int i = 1; i <= 4; i++) {
//         dataArray[arrayNum++] = resultBuffer[i + 5 + j * 11];
//       }
//     }
//   }

//   if (pid == 0x02 || pid == 0x04) {
//     return convertHexToAscii(dataArray, arrayNum);
//   } else if (pid == 0x06) {
//     return convertBytesToHexString(dataArray, arrayNum);
//   }
//   return "";
// }

int OBD2_CanBus::readSupportedData(byte mode) {
  int supportedCount = 0;
  int pidIndex = 0;
  int startByte = 0;
  int arraySize = 32;  // Size of supported data arrays
  byte *targetArray = nullptr;

  if (mode == read_LiveData) {
    startByte = 3;
    targetArray = supportedLiveData;
  } else if (mode == read_FreezeFrame) {
    startByte = 4;
    targetArray = supportedFreezeFrame;
  } else if (mode == read_VehicleInfo) {
    startByte = 4;
    targetArray = supportedVehicleInfo;
  } else if (mode == component_Monitoring) {
    startByte = 4;
    targetArray = supportedComponentMonitoring;
  } else {
    return -1;  // Invalid mode
  }

  writeData(mode, SUPPORTED_PIDS_1_20);
  if (readData()) {
    for (int i = 0; i < 4; i++) {
      byte value = resultBuffer.data[i + startByte];
      for (int bit = 7; bit >= 0; bit--) {
        if ((value >> bit) & 1) {
          targetArray[supportedCount++] = pidIndex + 1;
        }
        pidIndex++;
      }
    }
  }

  if (isInArray(targetArray, arraySize, 0x20)) {
    writeData(mode, SUPPORTED_PIDS_21_40);
    if (readData()) {
      for (int i = 0; i < 4; i++) {
        byte value = resultBuffer.data[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) {
            targetArray[supportedCount++] = pidIndex + 1;
          }
          pidIndex++;
        }
      }
    }
  }

  if (isInArray(targetArray, arraySize, 0x40)) {
    writeData(mode, SUPPORTED_PIDS_41_60);
    if (readData()) {
      for (int i = 0; i < 4; i++) {
        byte value = resultBuffer.data[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) {
            targetArray[supportedCount++] = pidIndex + 1;
          }
          pidIndex++;
        }
      }
    }
  }

  return supportedCount;
}

int OBD2_CanBus::readSupportedLiveData() {
  return readSupportedData(read_LiveData);
}

int OBD2_CanBus::readSupportedFreezeFrame() {
  return readSupportedData(read_FreezeFrame);
}

int OBD2_CanBus::readSupportedComponentMonitoring() {
  return readSupportedData(component_Monitoring);
}

int OBD2_CanBus::readSupportedVehicleInfo() {
  return readSupportedData(read_VehicleInfo);
}

byte OBD2_CanBus::getSupportedData(byte mode, int index) {
  if (mode == 0x01) {
    if (index >= 0) return supportedLiveData[index];
  } else if (mode == 0x02) {
    if (index >= 0) return supportedFreezeFrame[index];
  } else if (mode == 0x06) {
    if (index >= 0) return supportedComponentMonitoring[index];
  } else if (mode == 0x09) {
    if (index >= 0) return supportedVehicleInfo[index];
  }
  return 0;
}

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

void OBD2_CanBus::debugPrintHex(byte val) {
  if (_debugSerial) {
    if (val < 0x10) _debugSerial->print("0");
    _debugSerial->print(val, HEX);
  }
}

void OBD2_CanBus::debugPrintHexln(byte val) {
  if (_debugSerial) {
    debugPrintHex(val);
    _debugSerial->println();
  }
}

// void OBD2_CanBus::setWriteDelay(uint16_t delay) {
//   _writeDelay = delay;
// }

// void OBD2_CanBus::setDataRequestInterval(uint16_t interval) {
//   _dataRequestInterval = interval;
// }

void OBD2_CanBus::setProtocol(const String &protocolName) {
  selectedProtocol = protocolName;
  debugPrintln(("Protocol set to: " + selectedProtocol).c_str());
}

String OBD2_CanBus::decodeDTC(byte input_byte1, byte input_byte2) {
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

bool OBD2_CanBus::isInArray(const byte *dataArray, int length, byte value) {
  for (int i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
}

// String OBD2_CanBus::convertHexToAscii(const byte *dataArray, int length) {
//   String asciiString = "";
//   for (int i = 0; i < length; i++) {
//     byte b = dataArray[i];
//     if (b >= 0x20 && b <= 0x7E) {  // Printable ASCII range
//       asciiString += (char)b;
//     }
//   }
//   return asciiString;
// }

// String OBD2_CanBus::convertBytesToHexString(const byte *dataArray, int length) {
//   String hexString = "";
//   for (int i = 0; i < length; i++) {
//     if (dataArray[i] < 0x10) hexString += "0";  // Pad leading zero
//     hexString += String(dataArray[i], HEX);
//   }
//   hexString.toUpperCase();
//   return hexString;
// }
