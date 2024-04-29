void setup();
void loop();
void uint8ToMacString(const BleAddress* macArray, char* macString);
int isMacAddressEqual(const BleAddress *macArray, const char* macString);
int8_t isMacAddressValid(const BleAddress *address);
char* bufferToString(const uint8_t* buffer, size_t length);
void bufferToHexString(const uint8_t *buffer, size_t length, char *hexString, size_t hexStringSize);
void scanResultCallback(const BleScanResult *scanResult, void *context);
