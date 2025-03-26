#include <esp_now.h>
#include <WiFi.h>

// Define pins
const int Mainpiston = 15;
const int greenbutton = 18;
const int greenbuttonled = 4;
const int amberbutton = 21;
const int amberbuttonled = 2;
const int ESTOP = 22;
const int emergencyairrelse = 26;

bool greenled = true;
bool lastGreenButtonState = HIGH;
bool lastAmberButtonState = HIGH;
bool emergencyTriggered = false;  // Flag to track if air release has been triggered
bool pistonResetInProgress = false; // Flag to track if piston reset is in progress

unsigned long previousMillis = 0;
const long normalInterval = 500;  // 500ms interval for normal blinking
const long emergencyInterval = 200;  // 200ms interval for emergency blinking

// Structure to receive data
typedef struct struct_message {
  bool pistonstate;
} struct_message;

struct_message myData;

// Callback function for receiving data
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Data received: ");
  Serial.println(myData.pistonstate);

  if (myData.pistonstate) {
    digitalWrite(Mainpiston, HIGH);
    Serial.println("Piston is active");
    greenled = false;
  } else {
    digitalWrite(Mainpiston, LOW);
    Serial.println("Piston is standby");
    greenled = true;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(Mainpiston, OUTPUT);
  pinMode(greenbuttonled, OUTPUT);
  pinMode(amberbuttonled, OUTPUT);
  pinMode(emergencyairrelse, OUTPUT);
  pinMode(greenbutton, INPUT_PULLUP);
  pinMode(amberbutton, INPUT_PULLUP);
  pinMode(ESTOP, INPUT_PULLUP);

  // Ensure emergency air release is OFF at startup
  digitalWrite(emergencyairrelse, LOW);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long blinkInterval = emergencyTriggered ? emergencyInterval : normalInterval;  

  // **LED Blinking Logic**
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;

    if (emergencyTriggered || pistonResetInProgress) {
      // Blink both LEDs rapidly during emergency mode or piston reset
      digitalWrite(greenbuttonled, !digitalRead(greenbuttonled));
      digitalWrite(amberbuttonled, !digitalRead(amberbuttonled));
    } else {
      // Normal operation blinking logic
      if (greenled) {
        digitalWrite(greenbuttonled, !digitalRead(greenbuttonled));
        digitalWrite(amberbuttonled, LOW);
      } else {
        digitalWrite(amberbuttonled, !digitalRead(amberbuttonled));
        digitalWrite(greenbuttonled, LOW);
      }
    }
  }

  // **Green Button Press - Extend Piston**
  bool greenButtonState = digitalRead(greenbutton);
  if (greenButtonState == LOW && lastGreenButtonState == HIGH) {
    Serial.println("Green button pressed - Extending piston");
    extendpiston();
    greenled = false;
  }
  lastGreenButtonState = greenButtonState;

  // **Amber Button Press - Retract Piston**
  bool amberButtonState = digitalRead(amberbutton);
  if (amberButtonState == LOW && lastAmberButtonState == HIGH) {
    Serial.println("Amber button pressed - Retracting piston");
    retractpiston();
    greenled = true;
  }
  lastAmberButtonState = amberButtonState;

  // **E-STOP Press - Turn ON emergency air release and keep it ON permanently**
  bool eStopState = digitalRead(ESTOP);
  if (eStopState == HIGH && !emergencyTriggered) {  // Only trigger once
    Serial.println("E-STOP pressed - Emergency air release activated permanently");
    digitalWrite(emergencyairrelse, HIGH);
    retractpiston();
    greenled = true;
    emergencyTriggered = true;  // Set flag so it stays ON
  }

  // **Hold BOTH Green & Amber buttons to TURN OFF emergency air release**
  if (digitalRead(greenbutton) == LOW && digitalRead(amberbutton) == LOW && emergencyTriggered) {
    Serial.println("Both buttons held - Turning off emergency air release");

    // Start piston reset
    pistonResetInProgress = true;
    emergencyTriggered = false;
    digitalWrite(emergencyairrelse, LOW); // Reset emergency air release

    // Reset piston with blinking lights
    digitalWrite(Mainpiston, HIGH);
    delay(2000);  // Simulate piston reset time
    digitalWrite(Mainpiston, LOW);
    delay(1000);  // Wait before finishing the reset

    pistonResetInProgress = false;  // End reset
    greenled = true;  // Turn off the blinking lights when reset is done
  }
}

void extendpiston() {
  digitalWrite(Mainpiston, HIGH);
}

void retractpiston() {
  digitalWrite(Mainpiston, LOW);
}
