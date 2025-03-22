//REMOVE LINE 34&38 IF PROBLEM
// check 34&38 for corrext logic
#include <esp_now.h>
#include <WiFi.h>

// Define pins
const int Mainpiston = 15;
const int greenbutton = 18;
const int greenbuttonled = 4;
const int amberbutton = 21;
const int amberbuttonled = 2;
const int ESTOP = 22;

bool greenled = true;  // Initial state
bool lastGreenButtonState = HIGH;
bool lastAmberButtonState = HIGH;
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
    greenled = false; //REMOVE IT PROBLEM
  } else {
    digitalWrite(Mainpiston, LOW);
    Serial.println("Piston is standby");
    greenled = TRUE; //REMOVE LINE 34&38 IF PROBLEM
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(Mainpiston, OUTPUT);
  pinMode(greenbuttonled, OUTPUT);
  pinMode(amberbuttonled, OUTPUT);
  pinMode(greenbutton, INPUT_PULLUP);
  pinMode(amberbutton, INPUT_PULLUP);
  pinMode(ESTOP, INPUT_PULLUP);

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
  if (greenButtonState == LOW && lastGreenButtonState == HIGH) {  // Detect button press (not hold)
    Serial.println("Green button pressed - Extending piston");
    extendpiston();
    greenled = false;  // Switch to amber LED blinking
  }
  lastGreenButtonState = greenButtonState;

  // **Amber Button Press - Retract Piston**
  bool amberButtonState = digitalRead(amberbutton);
  if (amberButtonState == LOW && lastAmberButtonState == HIGH) {  // Detect button press (not hold)
    Serial.println("Amber button pressed - Retracting piston");
    retractpiston();
    greenled = true;  // Switch to green LED blinking
  }
  lastAmberButtonState = amberButtonState;
}

void extendpiston() {
  digitalWrite(Mainpiston, HIGH);
}

void retractpiston() {
  digitalWrite(Mainpiston, LOW);
}


//v1.0.0 main
