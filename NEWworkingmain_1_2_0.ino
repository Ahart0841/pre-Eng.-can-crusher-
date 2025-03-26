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

unsigned long previousMillis = 0;
const long interval = 500;  // 500ms interval for blinking

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

  // **Non-blocking LED blinking**
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (greenled) {
      digitalWrite(greenbuttonled, !digitalRead(greenbuttonled));  // Toggle green LED
      digitalWrite(amberbuttonled, LOW);
    } else {
      digitalWrite(amberbuttonled, !digitalRead(amberbuttonled));  // Toggle amber LED
      digitalWrite(greenbuttonled, LOW);
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
  }
}

void extendpiston() {
  digitalWrite(Mainpiston, HIGH);
}

void retractpiston() {
  digitalWrite(Mainpiston, LOW);
}
