#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#define CHANNEL 1



uint32_t lastSendTime = 0;
float newData[2];
Servo myservo;
Servo myservoy;
int i = 0;
int x;
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
//  Serial.print("I just received -> ");
 // Serial.println(*data);
  memcpy(&newData, data, sizeof(newData)); 
 

myservo.write(newData[0]);
myservoy.write(newData[1]);

}

void setup() {
  Serial.begin(115200);
  myservo.attach(13);
  myservoy.attach(12);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Meduza", "RX_1_Password", CHANNEL, 0);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
//   Serial.println("I did this to data");
 /* if (x != newData){
    x = newData;
if (x > 100) {
  int xs = x - 100;
  Serial.print("X = ");
  Serial.print(xs);
  myservo.write(xs);
  i = 1; 
} else if (x < 100) {
  int ys = 70 + x ;
  Serial.print("Y = ");
  Serial.println(ys);
  myservoy.write(ys);
  i = 0; 
}

}*/
 if((millis() - lastSendTime) >= 200) {
   Serial.print("X = ");
  Serial.println(newData[0]);
  Serial.print("Y = ");
  Serial.println(newData[1]);
  Serial.println();
  lastSendTime = millis();
  }
}
