/**
   bed-firmware.ino

    Created on: 29.08.2021
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
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

#define RIGHT_PIR_0_PIN D2
#define RIGHT_PIR_1_PIN D3

#define LEFT_PIR_0_PIN D5
#define LEFT_PIR_1_PIN D6

#define BOTTOM_PIR_PIN D7

#define ULTRASOUND_TRIGGER_PIN D0
#define ULTRASOUND_ECHO_PIN D1

#define NEOPIXEL_PIN D4

#define PAYLOAD_SIZE 100
char payload[PAYLOAD_SIZE + 1];

/*
   Communication
*/
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
char mqttTopicConfig[MQTT_TOPIC_SIZE];
int incomingByte = 0;

bool ensureWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi ");
    Serial.print(wifiSsid);
    Serial.print(" ");
    WiFi.begin(wifiSsid, wifiPassword);

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
      mqtt.subscribe(mqttTopicColor);
      mqtt.subscribe(mqttTopicConfig);
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

void callback(char* topic, byte* value, unsigned int length) {
  sprintf(payload, "MQTT[%s] -> ", topic, length, value[length]);
  Serial.print(payload);
  if (length >= PAYLOAD_SIZE) {
    length = PAYLOAD_SIZE;
  }
  memcpy(payload, value, length);
  payload[length] = '\0';
  Serial.println(payload);

  if (strncmp(mqttTopicColor, topic, MQTT_TOPIC_SIZE) == 0) {
    updateColor();
  }
  else if (strncmp(mqttTopicConfig, topic, MQTT_TOPIC_SIZE) == 0) {
    executeCommand();
  }
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
    executeCommand();
  }
}

/*
   Lights
*/
#define NUMPIXELS 1
uint16_t num_pixels = NUMPIXELS;
Adafruit_NeoPixel pixels(num_pixels, NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800);

void setAllPixelsToColor(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = Adafruit_NeoPixel::Color(r, g, b);
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void updateColor() {
    uint32_t red = 2;
    uint32_t green = 2;
    uint32_t blue = 2;
    sscanf(payload, "%u,%u,%u", &red, &green, &blue);
    Serial.print("New color: ");
    Serial.print(red, DEC);
    Serial.print(", ");
    Serial.print(green, DEC);
    Serial.print( ", ");
    Serial.println(blue, DEC);
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
  digitalWrite(ULTRASOUND_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(ULTRASOUND_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASOUND_TRIGGER_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ULTRASOUND_ECHO_PIN, HIGH);

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
   Config
*/
void executeCommand() {
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
  else if (strncmp("mqtt topic config ", payload, 18) == 0) {
    strncpy(mqttTopicConfig, payload + 18, MQTT_TOPIC_SIZE);
    Serial.print("MQTT topic config -> ");
    Serial.println(mqttTopicConfig);
  }
  else if (strncmp("disconnect wifi", payload, 15) == 0) {
    WiFi.disconnect();
    Serial.println("disconnecting WiFi");
  }
  else if (strncmp("disconnect mqtt", payload, 15) == 0) {
    WiFi.disconnect();
    Serial.println("disconnecting MQTT");
  }
  else if (strncmp("dump config", payload, 11) == 0) {
    Serial.println("Config:");
    Serial.print("WIFI SSID: ");
    Serial.println(wifiSsid);
    Serial.print("WIFI Password: ");
    Serial.println(wifiPassword);
    Serial.print("MQTT Broker: ");
    Serial.println(mqttBroker);
    Serial.print("MQTT User: ");
    Serial.println(mqttUser);
    Serial.print("MQTT Password: ");
    Serial.println(mqttPassword);
    Serial.print("MQTT Client: ");
    Serial.println(mqttClient);
    Serial.print("MQTT Topic Distance: ");
    Serial.println(mqttTopicDistance);
    Serial.print("MQTT Topic Color: ");
    Serial.println(mqttTopicColor);
    Serial.print("MQTT Topic Motion: ");
    Serial.println(mqttTopicMotion);
    Serial.print("MQTT Topic Config: ");
    Serial.println(mqttTopicConfig);
    Serial.print("# Pixels: ");
    Serial.println(num_pixels, DEC);
  }
}

void writeConfig() {
  uint16_t start = 0;
  writeString2EEPROM(wifiSsid, start, WIFI_SSID_SIZE);
  start += WIFI_SSID_SIZE;
  writeString2EEPROM(wifiPassword, start, WIFI_PASSWORD_SIZE);
  start += WIFI_PASSWORD_SIZE;
  writeString2EEPROM(mqttBroker, start, MQTT_BROKER_SIZE);
  start += MQTT_BROKER_SIZE;
  writeString2EEPROM(mqttUser, start, MQTT_USER_SIZE);
  start += MQTT_USER_SIZE;
  writeString2EEPROM(mqttPassword, start, MQTT_PASSWORD_SIZE);
  start += MQTT_PASSWORD_SIZE;
  writeString2EEPROM(mqttClient, start, MQTT_CLIENT_SIZE);
  start += MQTT_CLIENT_SIZE;
  writeInteger2EEPROM(5, start);
  start += 2;
  writeString2EEPROM(mqttTopicDistance, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  writeString2EEPROM(mqttTopicColor, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  writeString2EEPROM(mqttTopicMotion, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  writeString2EEPROM(mqttTopicConfig, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
}

void loadConfig() {
  uint16_t start = 0;
  readStringFromEEPROM(wifiSsid, start, WIFI_SSID_SIZE);
  start += WIFI_SSID_SIZE;
  readStringFromEEPROM(wifiPassword, start, WIFI_PASSWORD_SIZE);
  start += WIFI_PASSWORD_SIZE;
  readStringFromEEPROM(mqttBroker, start, MQTT_BROKER_SIZE);
  start += MQTT_BROKER_SIZE;
  readStringFromEEPROM(mqttUser, start, MQTT_USER_SIZE);
  start += MQTT_USER_SIZE;
  readStringFromEEPROM(mqttPassword, start, MQTT_PASSWORD_SIZE);
  start += MQTT_PASSWORD_SIZE;
  readStringFromEEPROM(mqttClient, start, MQTT_CLIENT_SIZE);
  start += MQTT_CLIENT_SIZE;
  num_pixels = readIntegerFromEEPROM(start);
  start += 2;
  readStringFromEEPROM(mqttTopicDistance, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  readStringFromEEPROM(mqttTopicColor, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  readStringFromEEPROM(mqttTopicMotion, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
  readStringFromEEPROM(mqttTopicConfig, start, MQTT_TOPIC_SIZE);
  start += MQTT_TOPIC_SIZE;
}

void writeString2EEPROM(char* value, uint16_t startAt, uint8_t len) {
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    EEPROM.write(startAt + i, value[i]);
  }
  EEPROM.end();
}

void writeInteger2EEPROM(uint16_t value, uint16_t startAt) {
  const unsigned char high = ((value >> 7) & 0x7f) | 0x80;
  const unsigned char low  = (value & 0x7f);

  EEPROM.begin(512);
  EEPROM.write(startAt, high);
  EEPROM.write(startAt + 1, low);
  EEPROM.end();  
}

void readStringFromEEPROM(char* dest, uint16_t startAt, uint8_t len) {
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    dest[i] = EEPROM.read(startAt + i);
  }
  EEPROM.end();
}

uint16_t readIntegerFromEEPROM(uint16_t startAt) {
  EEPROM.begin(512);
  const unsigned char high = EEPROM.read(startAt);
  const unsigned char low  = EEPROM.read(startAt + 1);
  EEPROM.end();
  return (((high & 0x7f) <<  7) | low);
}

/*
   Arduino functions
*/
void setup() {
  pinMode(RIGHT_PIR_0_PIN, INPUT);
  pinMode(RIGHT_PIR_1_PIN, INPUT);
  pinMode(LEFT_PIR_0_PIN, INPUT);
  pinMode(LEFT_PIR_1_PIN, INPUT);
  pinMode(BOTTOM_PIR_PIN, INPUT);
  
  //pinMode(ULTRASOUND_TRIGGER_PIN, OUTPUT);
  //pinMode(ULTRASOUND_ECHO_PIN, INPUT);

  Serial.begin(9600);

  delay(1000);

  loadConfig();

  pixels.updateLength(num_pixels);
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
  if (ensureWiFiConnection()) {
    if (ensureMQTTConnection()) {
      byte state0 = digitalRead(RIGHT_PIR_0_PIN);
      byte state1 = digitalRead(RIGHT_PIR_1_PIN);
      byte state2 = digitalRead(LEFT_PIR_0_PIN);
      byte state3 = digitalRead(LEFT_PIR_1_PIN);
      byte state4 = digitalRead(BOTTOM_PIR_PIN);
      uint16_t state = (state4 * 10000) + (state3 * 1000) + (state2 * 100) + (state1 * 10) + state0;
      sprintf(payload, "%d%d%d%d%d", state0, state1, state2, state3, state4);
      mqtt.publish(mqttTopicMotion, payload);
      Serial.print("MQTT[");
      Serial.print(mqttTopicMotion);
      Serial.print("] <- ");
      Serial.println(payload);
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
      delay(1000);
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
