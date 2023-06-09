#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include <Wire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> 

#include <secrets.h>

// Create a secrets.h file with these values defined (look at the secrets.h.example file)
char      wifi_ssid[] = WIFI_SSID;
char      wifi_password[] = WIFI_PASSWORD;
char      mqtt_broker_ip[] = MQTT_BROKER_IP;
uint16_t  mqtt_broker_port = MQTT_BROKER_PORT;
char      mqtt_broker_username[] = MQTT_BROKER_USERNAME;
char      mqtt_broker_password[] = MQTT_BROKER_PASSWORD;
char      mqtt_topic[] = MQTT_TOPIC;
char      mqtt_client_id[] = MQTT_CLIENT_ID;

// in & out pins
#define WATER_SENSOR_OUT_PIN  A0
#define WATER_SENSOR_IN_PIN   A5

#define SHORT_SLEEP_SECS  60   // 1 minute sleep if sending message totally fails
#define LONG_SLEEP_SECS   300  // 5 minutes normally (though messages aren still not guaranteed reach server..)

SFE_MAX1704X lipo(MAX1704X_MAX17048); 
char json_buffer[64];
WiFiClient wifi_client;

void wifi_event_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  WiFi.begin(wifi_ssid, wifi_password);
}

void connect_wifi() {
	WiFi.mode (WIFI_MODE_STA);
  WiFi.onEvent(wifi_event_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin (wifi_ssid, wifi_password);
  delay(2000); // Wait a bit for wifi to come up
}

double get_battery_level() {
  while(!Wire.available() && !Wire.begin()) {
    delay(100);
  }
  while (!lipo.begin()) {
    delay(100);
  }
  lipo.quickStart(); // Should we run this all the time?
	// double voltage = maxlipo.getVoltage();
	return lipo.getSOC();
}

boolean send_mqtt_message(const char* json_message) {

  Serial.printf("Connecting to MQTT Server");  
  PubSubClient mqttClient(mqtt_broker_ip, mqtt_broker_port, wifi_client);
  int try_nr = 0, max_tries = 10;
  while(!mqttClient.connect(mqtt_client_id, mqtt_broker_username, mqtt_broker_password)) {
    if(++try_nr > max_tries) {
      Serial.printf("Sending MQTT Message failed after %d tries, giving up :(\n", max_tries);
      return false;
    }
    Serial.printf("Unable to connect to the MQTT Server at attempt nr %d, retrying..\n", try_nr);
    delay(1000);
  }

  Serial.printf("Publishing MQTT message");
  mqttClient.publish(mqtt_topic, json_message);
  mqttClient.flush();

  Serial.printf("Disconnecting from the MQTT Server");
  mqttClient.disconnect();
  delay(1000); // Need delay for disconnect to be sent
  return true;
}

const char * get_json_message(boolean liquidDetected, double batteryPercent) {
  snprintf(json_buffer, 64, "{\"full\": %s, \"battery\": %.1f }", 
    (liquidDetected) ? "true" : "false", 
    batteryPercent);
  json_buffer[sizeof(json_buffer) - 1] = '\0';
  return json_buffer;
}

void setup () {

	Serial.begin (115200);

  pinMode(WATER_SENSOR_OUT_PIN, OUTPUT);
  pinMode(WATER_SENSOR_IN_PIN, INPUT_PULLUP);

  // Power on sensor
  digitalWrite(WATER_SENSOR_OUT_PIN, HIGH);
  delay(1000);

  // Read sensor input
  boolean is_full = (digitalRead(WATER_SENSOR_IN_PIN) == 0);

  // Power off sensor
  digitalWrite(WATER_SENSOR_OUT_PIN, LOW);

  // Gather message data
  double battery_level = get_battery_level();
  const char* json_message =  get_json_message(is_full, battery_level);
  Serial.printf("JSON to be sent: %s \n", json_message);

  // Send message
  connect_wifi();
  boolean success = send_mqtt_message(json_message);

  // Sleep
  Serial.printf("Message sent success = %d. Going to sleep.\n", success);
  uint64_t sleep_secs = (!success || is_full) ? SHORT_SLEEP_SECS : LONG_SLEEP_SECS;
  esp_sleep_enable_timer_wakeup(sleep_secs * (uint64_t)1000000);
  esp_deep_sleep_start(); 
}

void loop () {
  // DO nothing
}