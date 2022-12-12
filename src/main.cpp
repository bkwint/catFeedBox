#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define SWITCH_PIN D5 // Allows us to toggle between on an off state for te sensors
#define PIN_EMPTY D6  // If sensing high, the last ir line break sensor is no longer interrupted
#define PIN_FULL D7   // If sensing high, the first step towards empty has been set
#define PIN_HALF D2   // If sensing hig, the cat feeder is sensing more then half empty

// Wifi credentials
const char* ssid = "";
const char* password = "";
// Mqtt broker
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);

// Reconnect to the mqtt server
// to be able to publish a message
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Make a wifi connection to allow connection to a mqtt server
void setupWifi() {
  // connect to the wifi network
  WiFi.begin(ssid, password);

  // as long as we are not connected, we wait
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Initializes the applicaion and sets all the data pins
void setup() {
  Serial.begin(9600);
  
  // lets setup the wifi connection
  setupWifi();
  client.setServer(mqtt_server, 1883);

  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, HIGH);

  pinMode(PIN_FULL, INPUT);
  pinMode(PIN_HALF, INPUT);
  pinMode(PIN_EMPTY, INPUT);  

  Serial.println("Hello world!");
}

// The loop only runs once after that the esp will go into deep sleep and if awakened the setup is run first and
// the loop runs again.
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  digitalWrite(SWITCH_PIN, LOW);
  delay(100);

  // determine the state
  int val = digitalRead(PIN_FULL) + digitalRead(PIN_HALF) + digitalRead(PIN_EMPTY);

  Serial.println(digitalRead(PIN_FULL));
  Serial.println(digitalRead(PIN_HALF));
  Serial.println(digitalRead(PIN_EMPTY));

  int state = 0;
  switch (val) {
    case 3:
      state = 100;
      break;
    case 2:
      state = 90;
      break;
    case 1:
      state = 40;
      break;
    case 0:
      state = 20;
      break;
  }

  // put your main code here, to run repeatedly:
  Serial.println("Measurement");
  Serial.print("State: ");
  Serial.println(state);

  client.publish("catfeeder/state", String(state).c_str(), true);

  digitalWrite(SWITCH_PIN, HIGH);
  delay(3000);

  ESP.deepSleep(3600e6);
}