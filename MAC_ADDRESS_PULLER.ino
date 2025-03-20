#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    
    // Set up ESP32 as an Access Point
    WiFi.softAP("ESP32_AP");

    // Get and print the MAC address
    Serial.print("ESP32 AP MAC Address: ");
    Serial.println(WiFi.softAPmacAddress());
}

void loop() {
    // Nothing needed here
}
