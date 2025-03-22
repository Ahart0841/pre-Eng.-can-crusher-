#include <esp_now.h>
#include <WiFi.h>
#include <Bounce2.h>  // Debouncing for buttons
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define button pins
#define BUTTON_ON_PIN 12  // Button to turn ON
#define BUTTON_OFF_PIN 27 // Button to turn OFF

// MAC address of the slave ESP32 (replace with actual MAC)
uint8_t slaveAddress[] = {0x78, 0x21, 0x84, 0x8E, 0x38, 0x94};

// Structure for sending data
typedef struct struct_message {
    bool ledState;
} struct_message;

struct_message myData;

// Create Bounce objects for debouncing
Bounce debouncerOn = Bounce();
Bounce debouncerOff = Bounce();

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    delay(100);  // Allow time for display to initialize
    display.clearDisplay();
    display.display();  // Update with cleared screen

    // Initialize buttons with pull-up resistors
    pinMode(BUTTON_ON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_OFF_PIN, INPUT_PULLUP);

    // Attach buttons to debouncer objects
    debouncerOn.attach(BUTTON_ON_PIN);
    debouncerOn.interval(25);
    debouncerOff.attach(BUTTON_OFF_PIN);
    debouncerOff.interval(25);

    // Initialize WiFi in Station mode
    WiFi.mode(WIFI_STA);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register ESP-NOW peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, slaveAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
    // Update debounce states
    debouncerOn.update();
    debouncerOff.update();

    // Handle ON button press
    if (debouncerOn.fell()) {
        myData.ledState = true;
        esp_now_send(slaveAddress, (uint8_t*)&myData, sizeof(myData));
        Serial.println("Button pressed - LED ON command sent");

        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 20);
        display.println(F("EXTENDING"));
        display.display();
        delay(300);  // Prevent flickering
    }

    // Handle OFF button press
    if (debouncerOff.fell()) {
        myData.ledState = false;
        esp_now_send(slaveAddress, (uint8_t*)&myData, sizeof(myData));
        Serial.println("Button pressed - LED OFF command sent");

        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 20);
        display.println(F("RETRACTING"));
        display.display();
        delay(300);  // Prevent flickering
    }
}
