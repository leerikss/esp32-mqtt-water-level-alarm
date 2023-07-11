#include <Arduino.h>
#include <cmath>

#include <WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#include <Wire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> 

#include <secrets.h>

// in & out pins
#define WATER_SENSOR_OUT_PIN  A0
#define WATER_SENSOR_IN_PIN   A5

#define SLEEP_IN_SECONDS      300 // 5 min sleep time in seconds after message has been delivered
#define BATTERY_CHANGE_RATE   5   // 100%, 95%, 90%, ...

// Create a secrets.h file with these values defined (look at the secrets.h.example file)
char      wifi_ssid[] =             WIFI_SSID;
char      wifi_password[] =         WIFI_PASSWORD;
char      mqtt_broker_ip[] =        MQTT_BROKER_IP;
uint16_t  mqtt_broker_port =        MQTT_BROKER_PORT;
char      mqtt_broker_username[] =  MQTT_BROKER_USERNAME;
char      mqtt_broker_password[] =  MQTT_BROKER_PASSWORD;
char      mqtt_topic[] =            MQTT_TOPIC;
char      mqtt_client_id[] =        MQTT_CLIENT_ID;

SFE_MAX1704X lipo(MAX1704X_MAX17048); 
AsyncMqttClient mqtt_client;
Ticker wifi_reconnect_timer;
Ticker mqtt_reconnect_timer;
char json_buffer[64];

RTC_DATA_ATTR signed int  prev_liquid_detected = -1;
RTC_DATA_ATTR signed int  prev_battery_percent1 = -1;
RTC_DATA_ATTR signed int  prev_battery_percent2 = -1;

int read_battery_percent(int nearest_n) {
  while(!Wire.available() && !Wire.begin()) {
    delay(10);
  }
  while (!lipo.begin()) {
    delay(10);
  }
  lipo.quickStart(); // Should we run this every time?
  return static_cast<int>(round(lipo.getSOC() / nearest_n) * nearest_n);
}

void go_to_sleep() {
  Serial.printf("Going to deep sleep for %d seconds..\n", SLEEP_IN_SECONDS);
  esp_sleep_enable_timer_wakeup(SLEEP_IN_SECONDS * 1000000);
  esp_deep_sleep_start(); 
}

void connect_to_mqtt() {
  Serial.println("Connecting to the mqtt broker...");
  mqtt_client.connect();
}

void mqtt_event_connected(bool session_present) {
  Serial.printf("Connected to mqtt broker, publishing %s\n", json_buffer);
  mqtt_client.publish(MQTT_TOPIC, 1, true, json_buffer); // Using QoS 1 to retrieve acknowledgement..
}

void mqtt_event_disconnected(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from the mqtt broker.");
  if (WiFi.isConnected()) {
    mqtt_reconnect_timer.once(2, connect_to_mqtt);
  }  
}

void mqtt_event_published(uint16_t packet_id) {
  go_to_sleep();
}

void setup_mqtt() {
  mqtt_client.onConnect(mqtt_event_connected);
  mqtt_client.onDisconnect(mqtt_event_disconnected);
  mqtt_client.onPublish(mqtt_event_published);
  mqtt_client.setServer(mqtt_broker_ip, mqtt_broker_port);
  mqtt_client.setCredentials(mqtt_broker_username, mqtt_broker_password);
}

void connect_to_wifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin (wifi_ssid, wifi_password);
}

void wifi_event_connected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Connected to Wi-Fi");
  connect_to_mqtt();
}

void wifi_event_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Disconnected from Wi-Fi.");
  mqtt_reconnect_timer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifi_reconnect_timer.once(2, connect_to_wifi);
}

void setup_wifi() {
  WiFi.onEvent(wifi_event_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(wifi_event_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

const void set_message(boolean liquid_detected, int battery_percent) {
  snprintf(json_buffer, 64, "{\"full\": %s, \"battery\": %i }", 
    (liquid_detected) ? "true" : "false", battery_percent);
  json_buffer[sizeof(json_buffer) - 1] = '\0';
}

void publish_message(boolean liquid_detected, int battery_percent) {
  set_message(liquid_detected, battery_percent);
  setup_wifi();
  setup_mqtt();
  connect_to_wifi(); // Starts async message submission process
}

void setup () {

  setCpuFrequencyMhz(80); // Save power
  Serial.begin (115200);

  pinMode(WATER_SENSOR_OUT_PIN, OUTPUT);
  pinMode(WATER_SENSOR_IN_PIN, INPUT_PULLUP);

  digitalWrite(WATER_SENSOR_OUT_PIN, HIGH);
  delay(500); // Allow the sensor to stabilize
  boolean liquid_detected = (digitalRead(WATER_SENSOR_IN_PIN) == 0);
  digitalWrite(WATER_SENSOR_OUT_PIN, LOW);

  int battery_percent = read_battery_percent(BATTERY_CHANGE_RATE);

  // Preserve battery by not doing networking if values are unchanged
  if(liquid_detected == prev_liquid_detected && 
    (battery_percent == prev_battery_percent1 || 
    battery_percent == prev_battery_percent2)) {
    go_to_sleep();
  }

  // Persist values - we persist and check against two last battery values, as the battery value
  // fluctuates between two values when close to limit values, no need to waste battery by reporting that
  prev_liquid_detected = liquid_detected;
  prev_battery_percent2 = (prev_battery_percent1 == -1) ? battery_percent : prev_battery_percent1;
  prev_battery_percent1 = battery_percent;

  publish_message(liquid_detected, battery_percent);
}

void loop () {
}