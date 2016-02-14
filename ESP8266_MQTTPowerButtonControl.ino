/*
  This sketch is intended to toggle a button in reponse to incoming MQTT messages.

  Many devices have a power button that can be momentarily pressed to toggle power
  on and off.  Further, the device will often have something like a power LED that
  can be monitored to determine the device's power state.

  This sketch will monitor the configured MQTT topic for "on" and "off" events.  It
  will also monitor a selected pin to determine if the local device is "on" or "off".
  If the local state is "off" and an "on" message is received, it will momentarily
  toggle the output pin to turn the device on.  Likewise, if the monitored device
  is "on" and an incoming message for "off" is received, the power button pin will be
  toggled to turn the device off.

  The sketch will also monitor the power LED and publish an outgoing MQTT message when
  the power state changes.  This will capture local user interaction and update MQTT
  with the new power state.

  Finally, the sketch will periodically announce its local state every minute.

  HARDWARE NOTES:
    The power button pin can be connected to a relay or an optoisolator or similar device
    to actuate the device's power button.  A pull-down resistor will probably be needed.
    The power monitor pin can be connected to one leg of an LED or maybe VCC of the
    controlled device to detect the power state.  Keep in mind input voltage limits.

  REQUIREMENTS: PubSubClient
  Large portions of the code here are based on the "Basic ESP8266 MQTT example" from
  PubSubClient by Nick O'Leary available at http://pubsubclient.knolleary.net/
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "linksys";  // Your WiFi network name
const char* password = "password";  // Your WiFi network password
const char* mqtt_server = "192.168.0.100";  // Your MQTT server IP address

// This is the topic to be subscribed and published to
const char* mqtt_topic = "smartthings/Virtual Test Switch/switch";
const char* mqtt_onMessage = "on";
const char* mqtt_offMessage = "off";

const int powerButton = 14; // D5 on NodeMCU
const int powerMonitor = 12; // D6 on NodeMCU
const int powerButtonHoldTime = 50; // How long should the button be held for toggle
const int powerButtonWaitTime = 50; // How long to wait for the system to power up after toggling the power button

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
int powerSetState = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected succesfully and assigned IP: ");
  Serial.print(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String inMessage;
  for (int i = 0; i < length; i++) {
    inMessage += (char)payload[i];
  }
  Serial.print("Incoming message ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(inMessage);

  if (inMessage == mqtt_onMessage) {
    Serial.println("Received MQTT power on message");
    digitalWrite(BUILTIN_LED, LOW); // LOW == On
    int powerMonitorState = digitalRead(powerMonitor);
    powerSetState = 1;
    if (powerMonitorState == 0) {
      Serial.println("Toggling power on");
      digitalWrite(powerButton, HIGH);
      delay(powerButtonHoldTime);
      digitalWrite(powerButton, LOW);
      delay(powerButtonWaitTime);
    }
  }
  else if (inMessage == mqtt_offMessage) {
    Serial.println("Received MQTT power off message");
    digitalWrite(BUILTIN_LED, HIGH);  // HIGH == Off
    powerSetState = 0;
    int powerMonitorState = digitalRead(powerMonitor);
    if (powerMonitorState == 1) {
      Serial.println("Toggling power off");
      digitalWrite(powerButton, HIGH);
      delay(powerButtonHoldTime);
      digitalWrite(powerButton, LOW);
      delay(powerButtonWaitTime);
    }
  }
  else {
    Serial.print("No match for message: ");
    Serial.println(inMessage);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic, "ESP8266Client connected");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(powerButton, OUTPUT);
  pinMode(powerMonitor, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);

  // Record the current state as the desired set state, possibly to be
  // overwritten when we start MQTT
  powerSetState = digitalRead(powerMonitor);

  Serial.begin(115200);
  Serial.println("Program start");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // If our local state has changed against the set state, publish a message to MQTT
  int powerMonitorState = digitalRead(powerMonitor);
  if ((powerMonitorState == 0) && (powerSetState == 1)) {
    Serial.println("Caught local power off, publishing to MQTT");
    client.publish(mqtt_topic, mqtt_offMessage);
    powerSetState = 0;
  }
  else if ((powerMonitorState == 1) && (powerSetState == 0)) {
    Serial.println("Caught local power on, publishing to MQTT");
    client.publish(mqtt_topic, mqtt_onMessage);
    powerSetState = 1;
  }

  // Publish current state to topic every minute
  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    if (powerMonitorState) {
      Serial.print("Periodic publishing 'on' to topic: ");
      Serial.println(mqtt_topic);
      client.publish(mqtt_topic, "on");
    }
    else {
      Serial.print("Periodic publishing 'off' to topic: ");
      Serial.println(mqtt_topic);
      client.publish(mqtt_topic, "off");
    }
  }
}
