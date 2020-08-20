//master/sender ESPNow compatible with ESP32 and ESP8266
#ifdef ESP32
#include <esp_now.h>
#include <WiFi.h>
#elif ESP8266
#include <espnow.h>
#include <ESP8266WiFi.h>
#endif

#define CHANNEL 0

uint8_t remoteMac[] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC}; // Replace with the AP MAC address of the slave/receiver

#ifndef ESP_NOW_SEND_SUCCESS 
#define ESP_NOW_SEND_SUCCESS 0
#endif

uint8_t data = 1;
bool retry = true;

// callback when data is sent
#ifdef ESP32
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#elif ESP8266
void OnDataSent(uint8_t* mac_addr, uint8_t status) {
#endif
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendData() {
  data++;
  int result = esp_now_send(remoteMac, &data, sizeof(data));
  Serial.print("Send Command: ");
  if (result ==0) {
    Serial.println("Success " + String(result));
  } else {
    Serial.println("Failed " + String(result));
  }
  delay(100);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
#ifdef ESP32
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, remoteMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  int addStatus = esp_now_add_peer(&peerInfo);
#elif ESP8266
  int addStatus = esp_now_add_peer((uint8_t*)remoteMac, ESP_NOW_ROLE_CONTROLLER, CHANNEL, NULL, 0);
#endif
  if (addStatus == 0) {
    // Pair success
    Serial.println("Pair success");
  } else {
    Serial.println("Pair failed");
  }
}

void loop() {
  while (retry) {
    sendData();
    delay(100);
  }
  retry = true;
  delay(10000);
}
