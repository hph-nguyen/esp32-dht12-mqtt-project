#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT12.h"          // Library for DHT sensors
 
#define wifi_ssid "ssid"         
#define wifi_password "pass"     
 
#define mqtt_server "ip"  // IP or name from MQTT-Server
 
#define temperature_topic "esp32/dht/temperature"    // Topic for temperature
#define humidity_topic "esp32/dht/humidity"          // Topic for humidity
#define debug_topic "debug"                          // Topic for debugging
 
/* definitions for deepsleep */
#define uS_TO_S_FACTOR 1000000          // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP 300               // Time ESP32 will go to sleep for every 5 minutes (in seconds)

bool debug = true;             //Display log message if True
 
// Create objects
DHT12 dht;                    // I2C Grove as default setting
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
  Serial.begin(115200);     
  setup_wifi();                           //Connect to Wifi network
   
  client.setServer(mqtt_server, 1883);    // Configure MQTT connection, change port if needed.
 
  if (!client.connected()) {
    reconnect();
  }
  dht.begin();
  
  // Read temperature in Celcius
    float t = dht.readTemperature();
  // Read humidity
    float h = dht.readHumidity();
  
 
  // Nothing to send. Warn on MQTT debug_topic and then go to sleep for longer period.
    if ( isnan(t) || isnan(h)) {
      Serial.println("[ERROR] Please check the DHT sensor !");
      client.publish(debug_topic, "[ERROR] Please check the DHT sensor !", true);       // Publish humidity on broker
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);                    //go to sleep
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
    // Publish values to MQTT topics
    client.publish(temperature_topic, String(t).c_str(), true);   // Publish temperature on broker/temp1
    if ( debug ) {    
      Serial.println("Temperature sent to MQTT.");
    }
    delay(100); //some delay is needed for the mqtt server to accept the message
    client.publish(humidity_topic, String(h).c_str(), true);      // Publish humidity on broker/humid1
    if ( debug ) {
    Serial.println("Humidity sent to MQTT.");
    }   
 
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //go to sleep
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now.");
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
