#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT12.h"   // Library for DHT12

#define wifi_ssid "Vodafone-6E44"
#define wifi_password "69qHrk2nNTaENdEG"
// IP or name from MQTT-Server
#define mqtt_server "192.168.0.226"
// Topic for temperature,humidity,debugging
#define temperature_topic "esp32/dht/temperature"
#define humidity_topic "esp32/dht/humidity"
#define debug_topic "debug"

/* definitions for deepsleep */
// Conversion factor for micro seconds to seconds
#define uS_TO_S_FACTOR 1000000
// Time ESP32 will go to sleep for every 5 minutes (in seconds)
#define TIME_TO_SLEEP 300

//Display log message if True
bool debug = true;

// Create objects
DHT12 dht;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  //Connect to Wifi network
  setup_wifi();
  // Configure MQTT connection, change port if needed.
  client.setServer(mqtt_server, 1883);

  if (!client.connected()) {
    reconnect();
  }
  dht.begin();
  // Read temperature in Celcius (default)
  float t = dht.readTemperature();
  // Read humidity (%)
  float h = dht.readHumidity();

  // Nothing to send. Warn on MQTT debug_topic then sleep again
  if ( isnan(t) || isnan(h)) {
    Serial.println("[ERROR] Please check the DHT sensor !");
    // Publish error on broker
    client.publish(debug_topic, "[ERROR] Please check the DHT sensor !", true);
    //go to sleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep now due to ERROR");
    esp_deep_sleep_start();
    return;
  }

  if ( debug ) {
    Serial.print("Temperature : ");
    Serial.print(t);
    Serial.print(" | Humidity : ");
    Serial.println(h);
  }
  /* Publish values to MQTT topics */
  // Publish temperature on broker
  client.publish(temperature_topic, String(t).c_str(), true);
  if ( debug ) {
    Serial.println("Temperature sent to MQTT.");
  }
  // delay for the mqtt server to accept the message
  delay(100); 
  // Publish humidity on broker
  client.publish(humidity_topic, String(h).c_str(), true);
  if ( debug ) {
    Serial.println("Humidity sent to MQTT.");
  //delay for the mqtt server to accept the message before going to sleep
  delay(100); 
  }
  //go to sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}

//Setup connection to wifi
void setup_wifi() {
  delay(20);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi is OK ");
  Serial.print("=> ESP32 new IP address is: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

//Reconnect to wifi if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    String clientId = "ESP32Client";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("OK");
    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(5000);
    }
  }
}
void loop() {}
