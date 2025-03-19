#include <esp_now.h>
#include <WiFi.h>

// Define the LED pin
const int Mainpiston = 15;

const int greenbutton = 100;
const int greenbuttonled = 113;

const int amberbutton = 111;
const int amberbuttonled = 123;

const int ESTOP = 222;

//bool amberled = false;  // Declare a boolean variable and set it to false
bool greenled = true;  // Declare a boolean variable and set it to false

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

  // **LED CONTROL LOGIC**
  if (myData.pistonstate) {
    // move piston to crush
    digitalWrite(Mainpiston, HIGH);
    Serial.println("piston is active ");

    // >>> Add more actions when LED turns ON <<<
    // Example: Activate a buzzer, turn on another LED, send feedback, etc.

  } else {
    //move piston to standby
    digitalWrite(Mainpiston, LOW);
    Serial.println("piston is standby");

    // >>> Add more actions when LED turns OFF <<<
    // Example: Stop a buzzer, turn off another LED, log data, etc.
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


if (greenled) {
  digitalWrite(greenbuttonled, HIGH);
  _500msdelay();
  digitalWrite(greenbuttonled, LOW);
} else {
  digitalWrite(amberbuttonled, HIGH);
  _500msdelay();
  digitalWrite(amberbuttonled, LOW);
}


if (digitalRead(greenbutton) == LOW); {
extendpiston();
greenled = false; 
}

if (digitalRead(amberbutton) == LOW); {
  retractpiston();
  greenled = true; 
}

}


void extendpiston() {
  digitalWrite(Mainpiston, HIGH);

}

void retractpiston() {
  digitalWrite(Mainpiston, LOW);

}

void _500msdelay() {
    unsigned long startTime = millis();
    while (millis() - startTime < 500) {
        // Do nothing, but keep looping until 500ms pass
    }
  }  
