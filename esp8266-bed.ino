/**
   bed.ino

    Created on: 29.08.2021
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

#define PAYLOAD_SIZE 100
char payload[PAYLOAD_SIZE + 1];

/*
   Communication
*/
//ESP8266WiFiMulti WiFiMulti;
WiFiClient c;
PubSubClient mqtt(c);
#define WIFI_SSID_SIZE 16
char wifiSsid[WIFI_SSID_SIZE];
#define WIFI_PASSWORD_SIZE 21
char wifiPassword[WIFI_PASSWORD_SIZE];
#define MQTT_BROKER_SIZE 32
char mqttBroker[MQTT_BROKER_SIZE];
#define MQTT_USER_SIZE 32
char mqttUser[MQTT_USER_SIZE];
#define MQTT_PASSWORD_SIZE 32
char mqttPassword[MQTT_PASSWORD_SIZE];
#define MQTT_CLIENT_SIZE 16
char mqttClient[MQTT_CLIENT_SIZE];
#define MQTT_TOPIC_SIZE 32
char mqttTopicDistance[MQTT_TOPIC_SIZE];
char mqttTopicMotion[MQTT_TOPIC_SIZE];
char mqttTopicColor[MQTT_TOPIC_SIZE];
int incomingByte = 0;

bool ensureWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
//    WiFiMulti.addAP(wifiSsid, wifiPassword);
    Serial.print("Connecting to WiFi ");
    Serial.print(wifiSsid);
    Serial.print(" ");
    WiFi.begin(wifiSsid, wifiPassword);

//    while (WiFiMulti.run() != WL_CONNECTED) {
    for (uint8_t i = 0; i < 20; i++) {
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi SSID:");
      Serial.println(WiFi.SSID());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    return false;
  }
  else {
    return true;
  }
}

bool ensureMQTTConnection() {
  if (!mqtt.connected()) {
    Serial.print("Connecting to MQTT ");
    Serial.print(mqttBroker);
    Serial.print(" ");
    mqtt.setServer(mqttBroker, 1883);
    mqtt.setCallback(callback);

    if(mqtt.connect(mqttClient, mqttUser, mqttPassword)) {
      mqtt.subscribe(mqttTopicColor); // 27 + 1
    }
    for (uint8_t i = 0; i < 10; i++) {
      if (mqtt.connected()) {
        break;
      }
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
  if (!mqtt.connected()) {
    Serial.println(mqtt.state());
    return false;
  }
  return true;
}

void checkIncomingSerial() {
  incomingByte = 0;
  uint8_t i = 0;
  while (Serial.available() > 0 && incomingByte != 10 && i <= PAYLOAD_SIZE) {
    incomingByte = Serial.read();
    if (incomingByte == 10 || i == PAYLOAD_SIZE) {
      payload[i] = 0;
      break;
    }
    payload[i] = incomingByte;
    i += 1;
  }
  if (i > 0) {
    Serial.print("Read from serial: ");
    Serial.println(payload);
    if (strncmp("writeConfig", payload, PAYLOAD_SIZE) == 0) {
      Serial.print("writeConfig");
      writeConfig();
      Serial.println(" [done]");
    }
    else if (strncmp("ssid ", payload, 5) == 0) {
      strncpy(wifiSsid, payload + 5, WIFI_SSID_SIZE);
      Serial.print("SSID -> ");
      Serial.println(wifiSsid);
    }
    else if (strncmp("wifiPassword ", payload, 13) == 0) {
      strncpy(wifiPassword, payload + 13, WIFI_PASSWORD_SIZE);
      Serial.print("WiFi password -> ");
      Serial.println(wifiPassword);
    }
    else if (strncmp("mqtt broker ", payload, 12) == 0) {
      strncpy(mqttBroker, payload + 12, MQTT_BROKER_SIZE);
      Serial.print("MQTT broker -> ");
      Serial.println(mqttBroker);
    }
    else if (strncmp("mqtt user ", payload, 10) == 0) {
      strncpy(mqttUser, payload + 10, MQTT_USER_SIZE);
      Serial.print("MQTT user -> ");
      Serial.println(mqttUser);
    }
    else if (strncmp("mqtt password ", payload, 14) == 0) {
      strncpy(mqttPassword, payload + 14, MQTT_PASSWORD_SIZE);
      Serial.print("MQTT password -> ");
      Serial.println(mqttPassword);
    }
    else if (strncmp("mqtt client ", payload, 12) == 0) {
      strncpy(mqttClient, payload + 12, MQTT_CLIENT_SIZE);
      Serial.print("MQTT client -> ");
      Serial.println(mqttClient);
    }
    else if (strncmp("mqtt topic distance ", payload, 20) == 0) {
      strncpy(mqttTopicDistance, payload + 20, MQTT_TOPIC_SIZE);
      Serial.print("MQTT topic distance -> ");
      Serial.println(mqttTopicDistance);
    }
    else if (strncmp("mqtt topic color ", payload, 17) == 0) {
      strncpy(mqttTopicColor, payload + 17, MQTT_TOPIC_SIZE);
      Serial.print("MQTT topic color -> ");
      Serial.println(mqttTopicColor);
    }
    else if (strncmp("mqtt topic motion ", payload, 18) == 0) {
      strncpy(mqttTopicMotion, payload + 18, MQTT_TOPIC_SIZE);
      Serial.print("MQTT topic motion -> ");
      Serial.println(mqttTopicMotion);
    }
    else if (strncmp("disconnect wifi", payload, 15) == 0) {
      WiFi.disconnect();
      Serial.println("disconnecting WiFi");
    }
    else if (strncmp("disconnect mqtt", payload, 15) == 0) {
      WiFi.disconnect();
      Serial.println("disconnecting MQTT");
    }
  }
}

void writeConfig() {
  uint16_t start = 0;
  writeEEPROM(wifiSsid, start, WIFI_SSID_SIZE);
  start += WIFI_SSID_SIZE;
  writeEEPROM(wifiPassword, start, WIFI_PASSWORD_SIZE);
  start += WIFI_PASSWORD_SIZE;
  writeEEPROM(mqttBroker, start, MQTT_BROKER_SIZE);
  start += MQTT_BROKER_SIZE;
  writeEEPROM(mqttUser, start, MQTT_USER_SIZE);
  start += MQTT_USER_SIZE;
  writeEEPROM(mqttPassword, start, MQTT_PASSWORD_SIZE);
  start += MQTT_PASSWORD_SIZE;
  writeEEPROM(mqttClient, start, MQTT_CLIENT_SIZE);
  start += MQTT_CLIENT_SIZE;
  writeEEPROM(mqttTopicDistance, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  writeEEPROM(mqttTopicColor, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  writeEEPROM(mqttTopicMotion, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;

  Serial.println(wifiSsid);
  Serial.println(wifiPassword);
  Serial.println(mqttBroker);
  Serial.println(mqttUser);
  Serial.println(mqttPassword);
  Serial.println(mqttClient);
  Serial.println(mqttTopicDistance);
  Serial.println(mqttTopicColor);
  Serial.println(mqttTopicMotion);
}

void loadConfig() {
  uint16_t start = 0;
  readEEPROM(wifiSsid, start, WIFI_SSID_SIZE);
  start += WIFI_SSID_SIZE;
  readEEPROM(wifiPassword, start, WIFI_PASSWORD_SIZE);
  start += WIFI_PASSWORD_SIZE;
  readEEPROM(mqttBroker, start, MQTT_BROKER_SIZE);
  start += MQTT_BROKER_SIZE;
  readEEPROM(mqttUser, start, MQTT_USER_SIZE);
  start += MQTT_USER_SIZE;
  readEEPROM(mqttPassword, start, MQTT_PASSWORD_SIZE);
  start += MQTT_PASSWORD_SIZE;
  readEEPROM(mqttClient, start, MQTT_CLIENT_SIZE);
  start += MQTT_CLIENT_SIZE;
  readEEPROM(mqttTopicDistance, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  readEEPROM(mqttTopicColor, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  readEEPROM(mqttTopicMotion, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;

  Serial.println(wifiSsid);
  Serial.println(wifiPassword);
  Serial.println(mqttBroker);
  Serial.println(mqttUser);
  Serial.println(mqttPassword);
  Serial.println(mqttClient);
  Serial.println(mqttTopicDistance);
  Serial.println(mqttTopicColor);
  Serial.println(mqttTopicMotion);
}

void writeEEPROM(char* value, int startAt, uint8_t len) {
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    EEPROM.write(startAt + i, value[i]);
  }
  EEPROM.end();
}

void readEEPROM(char* dest, int startAt, uint8_t len) {
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    dest[i] = EEPROM.read(startAt + i);
  }
  EEPROM.end();
}

/*
   Lights
*/
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, D8, NEO_GRB + NEO_KHZ800);

void setAllPixelsToColor(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = Adafruit_NeoPixel::Color(r, g, b);
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void callback(char* topic, byte* value, unsigned int length) {
  sprintf(payload, "MQTT[%s] -> ", topic, length, value[length]);
  Serial.print(payload);
  if (length >= PAYLOAD_SIZE) {
    length = PAYLOAD_SIZE;
  }
  memcpy(payload, value, length);
  payload[length] = '\0';
  Serial.println(payload);

  uint32_t red = 2;
  uint32_t green = 2;
  uint32_t blue = 2;
  sscanf(payload, "%u,%u,%u", &red, &green, &blue);
  setAllPixelsToColor(red, green, blue);
}

/*
   Distance measuring
*/
#define SOUND_VELOCITY 0.3432
long duration;
float distanceMm;

void measureDistance() {
  // Clears the trigPin
  digitalWrite(D7, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(D7, HIGH);
  delayMicroseconds(10);
  digitalWrite(D7, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(D6, HIGH);

  // Calculate the distance
  distanceMm = duration * SOUND_VELOCITY / 2;

  sprintf(payload, "%.1f", distanceMm);
  mqtt.publish(mqttTopicDistance, payload);
  Serial.print("MQTT[");
  Serial.print(mqttTopicDistance);
  Serial.print("] <- ");
  Serial.println(payload);
}

/*
   Arduino functions
*/
void setup() {
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, OUTPUT);
  //  pinMode(D8, OUTPUT);

  Serial.begin(9600);

  delay(1000);

  loadConfig();

  pixels.begin();

  setAllPixelsToColor(0, 0, 0);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  ensureWiFiConnection();

  ensureMQTTConnection();
}

void loop() {

  checkIncomingSerial();

  // wait for WiFi connection
  //if ((WiFiMulti.run() == WL_CONNECTED)) {
  if (ensureWiFiConnection()) {
    if (ensureMQTTConnection()) {
      //    byte state = digitalRead(D0);
      //    if(state == 1)Serial.print("D0: yes / ");
      //    else if(state == 0)Serial.print("D0: no / ");
      //
      //    state = digitalRead(D1);
      //    if(state == 1)Serial.print("D1: yes / ");
      //    else if(state == 0)Serial.print("D1: no / ");
      //
      //    state = digitalRead(D2);
      //    if(state == 1)Serial.print("D2: yes / ");
      //    else if(state == 0)Serial.print("D2: no / ");
      //
      //    state = digitalRead(D3);
      //    if(state == 1)Serial.print("D3: yes / ");
      //    else if(state == 0)Serial.print("D3: no / ");
      //
      //    state = digitalRead(D4);
      //    if(state == 1)Serial.print("D4: yes / ");
      //    else if(state == 0)Serial.print("D4: no / ");
      //
      //    state = digitalRead(D5);
      //    if(state == 1)Serial.println("D5: yes");
      //    else if(state == 0)Serial.println("D5: no");
  
      measureDistance();
  
      mqtt.loop();
      delay(2500);
    }
    else {
    Serial.println("Not connected to MQTT. Waiting 10 seconds");
    delay(10000);
    }
  }
  else {
    Serial.println("Not connected to WIFI. Waiting 10 seconds");
    delay(10000);
  }
}
