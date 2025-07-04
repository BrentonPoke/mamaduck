/**
 * @file MamaDuck.ino
 * @brief Uses the built in Mama Duck.
 */
#include <string>
#include <arduino-timer.h>
#include <MamaDuck.h>
#include <vector>
#include <Arduino.h>

#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

// create a built-in mama duck
MamaDuck duck;

DuckDisplay* display = nullptr;

// LORA RF CONFIG
#define LORA_FREQ 915.0 // Frequency Range. Set for US Region 915.0Mhz
#define LORA_TXPOWER 20 // Transmit Power
// LORA HELTEC PIN CONFIG
#define LORA_CS_PIN 18
#define LORA_DIO0_PIN 26
#define LORA_DIO1_PIN -1 // unused
#define LORA_RST_PIN 14

// create a timer with default settings
auto timer = timer_create_default();

// for sending the counter message
const int INTERVAL_MS = 60000;
int counter = 1;
bool sendData(const byte* buffer, int length);
void setup() {
    // The Device ID must be unique and 8 bytes long. Typically the ID is stored
    // in a secure nvram, or provided to the duck during provisioning/registration
    std::array<uint8_t,8> deviceId = {'R', 'E', 'L', 'A', 'Y', '0','0', '1'}; // MUST be 8 bytes and unique from other ducks


    //Set the Device ID
    duck.setDeviceId(deviceId);
    // initialize the serial component with the hardware supported baudrate
    duck.setupSerial(115200);
    // initialize the LoRa radio with specific settings. This will overwrites settings defined in the CDP config file cdpcfg.h
    duck.setupRadio(LORA_FREQ, LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN, LORA_DIO1_PIN, LORA_TXPOWER);

    display = DuckDisplay::getInstance();
    display->setupDisplay(duck.getType(), deviceId);
    // we are done
    display->showDefaultScreen();

    // Initialize the timer. The timer thread runs separately from the main loop
    // and will trigger sending a counter message.
    //timer.every(INTERVAL_MS, runSensor);
    Serial.println("[MAMA] Setup OK!");

}

void loop() {
    timer.tick();
    // Use the default run(). The Mama duck is designed to also forward data it receives
    // from other ducks, across the network. It has a basic routing mechanism built-in
    // to prevent messages from hoping endlessly.
    duck.run();
}

bool runSensor(void *) {
    bool result;
    const byte* buffer;

    String message = String("Counter:") + String(counter);
    int length = message.length();
    Serial.print("[MAMA] sensor data: ");
    Serial.println(message);
    buffer = (byte*) message.c_str();

    result = sendData(buffer, length);
    if (result) {
        Serial.println("[MAMA] runSensor ok.");
    } else {
        Serial.println("[MAMA] runSensor failed.");
    }
    return result;
}

bool sendData(const byte* buffer, int length) {
    bool sentOk = false;

    // Send Data can either take a byte buffer (unsigned char) or a vector
    int err = duck.sendData(topics::status, buffer, length);
    if (err == DUCK_ERR_NONE) {
        counter++;
        sentOk = true;
    }
    if (!sentOk) {
        Serial.println("[MAMA] Failed to send data. error = " + String(err));
    }
    return sentOk;
}
