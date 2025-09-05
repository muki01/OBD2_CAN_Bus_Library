#ifndef OBD2_CANBUS_H
#define OBD2_CANBUS_H

#include <Arduino.h>
#include <driver/twai.h>

// #include "VehicleData.h"

// ==== OBD2 Mods ====
const uint8_t read_LiveData = 0x01;              // Show current live data
const uint8_t read_FreezeFrame = 0x02;           // Show freeze frame data
const uint8_t read_storedDTCs = 0x03;            // Show stored Diagnostic Trouble Codes (DTCs)
const uint8_t clear_DTCs = 0x04;                 // Clear Diagnostic Trouble Codes and stored values
const uint8_t test_OxygenSensors = 0x05;         // Test results, oxygen sensor monitoring (non-CAN only)
const uint8_t test_OtherComponents = 0x06;       // Test results, other component/system monitoring (for CAN)
const uint8_t read_pendingDTCs = 0x07;           // Show pending Diagnostic Trouble Codes
const uint8_t control_OnBoardComponents = 0x08;  // Control operation of on-board component/system
const uint8_t read_VehicleInfo = 0x09;           // Request vehicle information
const uint8_t read_PermanentDTCs = 0x0A;         // Show permanent Diagnostic Trouble Codes

const uint8_t SUPPORTED_PIDS_1_20 = 0x00;
const uint8_t SUPPORTED_PIDS_21_40 = 0x20;
const uint8_t SUPPORTED_PIDS_41_60 = 0x40;
const uint8_t SUPPORTED_PIDS_61_80 = 0x60;
const uint8_t SUPPORTED_PIDS_81_100 = 0x80;

const uint8_t supported_VehicleInfo = 0x00;  // Read Supported Vehicle Info
const uint8_t read_VIN_Count = 0x01;         // Read VIN Count
const uint8_t read_VIN = 0x02;               // Read VIN
const uint8_t read_ID_Length = 0x03;         // Read Calibration ID Length
const uint8_t read_ID = 0x04;                // Read Calibration ID
const uint8_t read_ID_Num_Length = 0x05;     // Read Calibration ID Number Length
const uint8_t read_ID_Num = 0x06;            // Read Calibration ID Number

typedef struct {
  uint32_t id;
  uint32_t rtr;
  uint32_t ide;
  uint8_t length;
  uint8_t data[8];
} canMessage;

class OBD2_CanBus {
 public:
  // VehicleData car;
  OBD2_CanBus(uint8_t rxPin, uint8_t txPin);

  void setDebug(Stream &serial);
  bool initOBD2();
  bool testConnection();
  bool initTWAI();
  void stopTWAI();
  bool writeData(uint8_t mode, uint8_t pid);
  bool writeRawData(canMessage msg);
  uint8_t readData();
  bool readAndCompareData(const canMessage &target);

  float getPID(uint8_t mode, uint8_t pid);
  float getLiveData(uint8_t pid);
  float getFreezeFrame(uint8_t pid);

  uint8_t readDTCs(uint8_t mode);
  uint8_t readStoredDTCs();
  uint8_t readPendingDTCs();
  String getStoredDTC(uint8_t index);
  String getPendingDTC(uint8_t index);

  bool clearDTC();

  // String getVehicleInfo(uint8_t pid);

  uint8_t readSupportedLiveData();
  uint8_t readSupportedFreezeFrame();
  uint8_t readSupportedComponentMonitoring();
  uint8_t readSupportedVehicleInfo();
  uint8_t readSupportedData(uint8_t mode);
  uint8_t getSupportedData(uint8_t mode, uint8_t index);

  void setProtocol(const String &protocolName);

  void updateConnectionStatus(bool messageReceived);
  void setReadTimeout(uint16_t timeoutMs);

 private:
  uint8_t _rxPin;
  uint8_t _txPin;
  Stream *_debugSerial = nullptr;  // Debug serial port

  twai_message_t resultBuffer;
  uint8_t unreceivedDataCount = 0;
  bool connectionStatus = false;

  String selectedProtocol = "Automatic";
  String connectedProtocol = "";
  uint8_t CAN_BIT = 11;  // 11-bit or 29-bit
  twai_timing_config_t CAN_SPEED = TWAI_TIMING_CONFIG_250KBITS();
  uint16_t _readTimeout = 200;
  String storedDTCBuffer[32];
  String pendingDTCBuffer[32];

  uint8_t supportedLiveData[32];
  uint8_t supportedFreezeFrame[32];
  uint8_t supportedOxygenSensor[32];
  uint8_t supportedOtherComponents[32];
  uint8_t supportedControlComponents[32];
  uint8_t supportedVehicleInfo[32];

  String decodeDTC(uint8_t input_byte1, uint8_t input_byte2);
  bool isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value);
  // String convertBytesToHexString(const uint8_t *dataArray, int length);
  // String convertHexToAscii(const uint8_t *dataArray, int length);
  void debugPrint(const char *msg);
  void debugPrint(const __FlashStringHelper *msg);
  void debugPrintln(const char *msg);
  void debugPrintln(const __FlashStringHelper *msg);
  void debugPrintHex(uint32_t val);    // Hexadecimal output
  void debugPrintHexln(uint32_t val);  // Hexadecimal + newline
};

#endif  // OBD2_CANBUS_H