// Include Particle Device OS APIs
#include "Particle.h"
#include "capstone.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

int board_led = D7; // Instead of writing D7 over and over again, we'll write led2

// # define BLE_MAX_ADV_DATA_LEN 29

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

#define VALID_MAC_COUNT 5

const char* const valid_macs[VALID_MAC_COUNT] = {
    "76:2D:12:2A:0B:93",
    "00:57:A0:A8:D8:1A",
    "63:DC:2C:0C:44:F5",
    "41:84:C5:0F:F8:94",
    "6B:EE:E5:64:50:ED",
};

int8_t last_uploaded_index[VALID_MAC_COUNT];


void setup() {
  Log.info("Booting up...");
  pinMode(board_led, OUTPUT);
  
  for (size_t i = 0 ; i < VALID_MAC_COUNT; i++) {
      last_uploaded_index[i] = -1;
  }
}

void loop() {
  // Flash LED  
  delay(2000);
  digitalWrite(board_led, HIGH);


  delay(20);
  digitalWrite(board_led, LOW);
  BLE.setScanTimeout(500);
  BLE.scan(scanResultCallback, NULL);
}

void uint8ToMacString(const BleAddress* macArray, char* macString) {
    snprintf(macString, 18,"%02X:%02X:%02X:%02X:%02X:%02X",
            (*macArray)[5], (*macArray)[4], (*macArray)[3],
            (*macArray)[2], (*macArray)[1], (*macArray)[0]);
}

// Function to check if MAC addresses are equal
int isMacAddressEqual(const BleAddress *macArray, const char* macString) {
    char convertedMac[18];  // To store the converted MAC address string
    uint8ToMacString(macArray, convertedMac);
    
    // Compare the generated string with the input string
    return strcmp(convertedMac, macString) == 0;
}

int8_t isMacAddressValid(const BleAddress *address) {
    for (size_t i = 0 ; i < VALID_MAC_COUNT ; i ++) {
        if (isMacAddressEqual(address, valid_macs[i])) {
            return i;
        }
    }
    return -1;
}

char* bufferToString(const uint8_t* buffer, size_t length) {
    // Allocate memory for the string (including null terminator)
    char* result = (char*)malloc(length + 1);

    if (result == NULL) {
        // Memory allocation failed
        Log.error("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Copy each byte from the buffer to the string
    for (size_t i = 0; i < length; ++i) {
        result[i] = (char)buffer[i];
    }

    // Null-terminate the string
    result[length] = '\0';

    return result;
}

void bufferToHexString(const uint8_t *buffer, size_t length, char *hexString, size_t hexStringSize) {
    for (size_t i = 0; i < length; i++) {
        snprintf(hexString + 2 * i, hexStringSize - 2 * i, "%02x", buffer[i]);
    }
}

void scanResultCallback(const BleScanResult *scanResult, void *context) {
    BleAddress address = scanResult->address();
    int8_t address_position = !isMacAddressValid(&address);
    address_position = 0;
    if (address_position == -1) {
        return;
    }

    // uint8_t buf[BLE_MAX_ADV_DATA_LEN]; 
    // int len = scanResult->advertisingData().get(buf, BLE_MAX_ADV_DATA_LEN);
    
    uint8_t buf[BLE_MAX_ADV_DATA_LEN] = { 0x00, 0x1E, 0x00, 0x1F, 0x1F };
    int len = 5;
    
    uint8_t current_index = buf[2];
    
    if (last_uploaded_index[address_position] == current_index) {
        return;
    }
    
    bool upload_all = last_uploaded_index[address_position] != current_index + 1;
    last_uploaded_index[address_position] = current_index;
    
    size_t hexStringSize = 2 * len + 1; // +1 for null terminator
    char hexString[hexStringSize];
    
    bufferToHexString(buf, len, hexString, hexStringSize);
    
    Log.info(
        "MAC: %02X:%02X:%02X:%02X:%02X:%02X | RSSI: %dBm | Data (%d): %s",
        scanResult->address()[5], scanResult->address()[4], scanResult->address()[3],
        scanResult->address()[2], scanResult->address()[1], scanResult->address()[0], 
        scanResult->rssi(), len, hexString
    );
    
    time32_t timestamp = Time.now();
}