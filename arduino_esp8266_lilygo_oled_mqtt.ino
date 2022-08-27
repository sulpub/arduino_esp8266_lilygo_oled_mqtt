/*
  Code for testing MQTT link with ESP8266

  Source information : 
   - https://github.com/LilyGO/TTGO-ESP8266-0.91-SSD1306?utm_source=pocket_mylist
   - https://fr.aliexpress.com/item/32824012998.html
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "OLED.h"

// ------------------------------------------------------------
// PARAMETRES
// ------------------------------------------------------------

const char* ssid        = "ssid";
const char* password    = "mot passe wifi";
const char* mqtt_server = "ip du broker mosquitto";
String clientLoginMqtt  = "login mosquitto - vide si non utilisé";
String clientPassMqtt   = "mot passe mosquitto - vide si non utilisé";
// ------------------------------------------------------------

//NOTES
//ok sur broker raspberry probleme sur mosquito windows
//ouvert port 1883 sur le firewall de windows

//WIFI_Kit_8's OLED connection:
//SDA -- D4
//SCL -- D5
//RST -- D2
OLED display(2, 14);  //OLED display(D4, D5);

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
//PubSubClient client(espClient);
PubSubClient client(mqtt_server, 1883, callback, espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Carte esp8266");
      // ... and resubscribe
      client.subscribe("relais");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);   // turn D2 low to reset OLED
  delay(50);
  digitalWrite(4, HIGH);    // while OLED is running, must set D2 in high

  // Initialize display
  display.begin();

  // Test message
  display.print("Start...");
  delay(3 * 1000);
  // Test display clear
  display.clear();
  delay(3 * 1000);

  // Test display OFF
  display.off();
  //  display.print("...", 3, 8);
  //  delay(3 * 1000);
  //
  //  // Test display ON
  //  display.on();
  //  delay(3 * 1000);

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Carte esp8266 trame #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);

    display.on();
    display.print("cmpt", value);
  }
}
