#include <Arduino.h>

#include <WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#include <Wire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> 

#include <secrets.h>

// in & out pins
#define WATER_SENSOR_OUT_PIN  A0
#define WATER_SENSOR_IN_PIN   A5

// Sleep time in seconds after message has been delivered
#define SLEEP_IN_SECONDS      600

// Create a secrets.h file with these values defined (look at the secrets.h.example file)
char      wifi_ssid[] = WIFI_SSID;
char      wifi_password[] = WIFI_PASSWORD;
char      mqtt_broker_ip[] = MQTT_BROKER_IP;
uint16_t  mqtt_broker_port = MQTT_BROKER_PORT;
char      mqtt_broker_username[] = MQTT_BROKER_USERNAME;
char      mqtt_broker_password[] = MQTT_BROKER_PASSWORD;
char      mqtt_topic[] = MQTT_TOPIC;
char      mqtt_client_id[] = MQTT_CLIENT_ID;

SFE_MAX1704X lipo(MAX1704X_MAX17048); 
AsyncMqttClient mqttClient;
Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;
char json_buffer[64];

double read_battery_level() {
  while(!Wire.available() && !Wire.begin()) {
    delay(10);
  }
  while (!lipo.begin()) {
    delay(10);
  }
  // lipo.quickStart(); // Should we run this?
  return lipo.getSOC();
}

void connect_to_mqtt() {
  Serial.println("Connecting to the mqtt broker...");
  mqttClient.connect();
}

void mqtt_event_disconnected(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from the mqtt broker.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connect_to_mqtt);
  }  
}

void mqtt_event_connected(bool sessionPresent) {
  Serial.printf("Connected to mqtt broker, publishing %s\n", json_buffer);
  mqttClient.publish(MQTT_TOPIC, 1, true, json_buffer); // Using QoS 1 to retrieve acknowledgement..
}

void mqtt_event_published(uint16_t packetId) {
  Serial.printf("Publish acknowledged. Going to deep sleep for %d seconds..\n", SLEEP_IN_SECONDS);
  esp_sleep_enable_timer_wakeup(SLEEP_IN_SECONDS * 1000000);
  esp_deep_sleep_start(); 
}

void setup_mqtt() {
  mqttClient.onConnect(mqtt_event_connected);
  mqttClient.onDisconnect(mqtt_event_disconnected);
  mqttClient.onPublish(mqtt_event_published);
  mqttClient.setServer(mqtt_broker_ip, mqtt_broker_port);
  mqttClient.setCredentials(mqtt_broker_username, mqtt_broker_password);
}

void wifi_event_connected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Connected to Wi-Fi");
  connect_to_mqtt();
}

void connect_to_wifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin (wifi_ssid, wifi_password);
}

void wifi_event_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connect_to_wifi);
}

void setup_wifi() {
  WiFi.onEvent(wifi_event_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(wifi_event_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

const void set_json_message(boolean liquidDetected, double batteryPercent) {
  snprintf(json_buffer, 64, "{\"full\": %s, \"battery\": %.1f }", 
    (liquidDetected) ? "true" : "false", batteryPercent);
  json_buffer[sizeof(json_buffer) - 1] = '\0';
}

void setup () {

  Serial.begin (115200);

  pinMode(WATER_SENSOR_OUT_PIN, OUTPUT);
  pinMode(WATER_SENSOR_IN_PIN, INPUT_PULLUP);

  // Power on sensor
  digitalWrite(WATER_SENSOR_OUT_PIN, HIGH);
  delay(500); // Allow the sensor to stabilize

  // Read sensor input
  boolean is_full = (digitalRead(WATER_SENSOR_IN_PIN) == 0);

  // Power off sensor
  digitalWrite(WATER_SENSOR_OUT_PIN, LOW);

  // Generate json message
  double battery_level = read_battery_level();
  set_json_message(is_full, battery_level);
  Serial.printf("JSON to be sent: %s \n", json_buffer);

  // Start connections and publish json async
  setup_wifi();
  setup_mqtt();
  connect_to_wifi();

}

void loop () {
}