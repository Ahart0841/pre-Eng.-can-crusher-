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
const int buzzer = 33;

bool greenled = true;
bool lastGreenButtonState = HIGH;
bool lastAmberButtonState = HIGH;
bool emergencyTriggered = false;
bool pistonResetInProgress = false;
bool pistonExtended = false; // Track piston state

unsigned long previousMillis = 0;
const long normalInterval = 500;
const long emergencyInterval = 200;

// Structure to receive data
typedef struct struct_message {
  bool pistonstate;
} struct_message;

struct_message myData;

// Debounce function to prevent false button triggers
bool debounceButton(int pin, bool lastState) {
  bool currentState = digitalRead(pin);
  if (currentState != lastState) {
    delay(50);
    currentState = digitalRead(pin);
  }
  return currentState;
}

// ESP-NOW callback function
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Data received: ");
  Serial.println(myData.pistonstate);

  if (myData.pistonstate) {
    digitalWrite(Mainpiston, HIGH);
    Serial.println("Piston is active");
    greenled = false;
    pistonExtended = true;
  } else {
    digitalWrite(Mainpiston, LOW);
    Serial.println("Piston is standby");
    greenled = true;
    pistonExtended = false;
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
  pinMode(buzzer, OUTPUT);

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

  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;

    if (emergencyTriggered || pistonResetInProgress) {
      digitalWrite(greenbuttonled, !digitalRead(greenbuttonled));
      digitalWrite(amberbuttonled, !digitalRead(amberbuttonled));
    } else {
      if (greenled) {
        digitalWrite(greenbuttonled, !digitalRead(greenbuttonled));
        digitalWrite(amberbuttonled, LOW);
      } else {
        digitalWrite(amberbuttonled, !digitalRead(amberbuttonled));
        digitalWrite(greenbuttonled, LOW);
      }
    }
  }

  noInterrupts();
  bool greenButtonState = debounceButton(greenbutton, lastGreenButtonState);
  bool amberButtonState = debounceButton(amberbutton, lastAmberButtonState);
  bool eStopState = debounceButton(ESTOP, emergencyTriggered);
  interrupts();

  // **Green Button Press - Hold for 3 Seconds (Only If Piston Is Not Extended)**
  if (greenButtonState == LOW && lastGreenButtonState == HIGH && !pistonExtended) {
    Serial.println("Green button pressed - Holding for 3 seconds...");

    unsigned long holdStartTime = millis();
    bool buttonStillHeld = true;

    // Beep while waiting ONLY if piston is not extended
    while (millis() - holdStartTime < 1500) {
      digitalWrite(buzzer, HIGH);
      delay(50);
      digitalWrite(buzzer, LOW);
      delay(50);

      if (digitalRead(greenbutton) == HIGH) {  // If button is released early, cancel operation
        Serial.println("Button released early - Canceled piston extension.");
        buttonStillHeld = false;
        break;
      }
    }

    if (buttonStillHeld) {
      Serial.println("Green button held for 3 seconds - Extending piston.");
      extendpiston();
      greenled = false;
      pistonExtended = true;  // Mark piston as extended
    }
  }
  lastGreenButtonState = greenButtonState;

  // **Amber Button Press - Retract Piston**
  if (amberButtonState == LOW && lastAmberButtonState == HIGH) {
    delay(50);
    if (digitalRead(amberbutton) == LOW) { 
      Serial.println("Amber button pressed - Retracting piston");
      retractpiston();
      greenled = true;
      pistonExtended = false;  // Mark piston as retracted
    }
  }
  lastAmberButtonState = amberButtonState;

  // **E-STOP Press - Trigger Emergency Mode**
  if (eStopState == HIGH && !emergencyTriggered) {  
    Serial.println("E-STOP pressed - Emergency air release activated permanently");
    digitalWrite(emergencyairrelse, HIGH);
    retractpiston();
    greenled = true;
    pistonExtended = false;  // Mark piston as retracted
    emergencyTriggered = true;
  }

  // **Hold BOTH Green & Amber buttons to TURN OFF emergency air release**
  if (greenButtonState == LOW && amberButtonState == LOW && emergencyTriggered) {
    Serial.println("Both buttons held - Turning off emergency air release");

    pistonResetInProgress = true;
    emergencyTriggered = false;
    digitalWrite(emergencyairrelse, LOW);

    digitalWrite(Mainpiston, HIGH);
    delay(2000);
    digitalWrite(Mainpiston, LOW);
    delay(1000);

    pistonResetInProgress = false;
    greenled = true;
    pistonExtended = false;  // Ensure piston is considered retracted after reset
  }
}

// **Extend piston function**
void extendpiston() {
  digitalWrite(Mainpiston, HIGH);
}

// **Retract piston function**
void retractpiston() {
  digitalWrite(Mainpiston, LOW);
}
