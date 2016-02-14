# ESP8266 MQTT Power Button control

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

##HARDWARE NOTES:
* The power button pin can be connected to a relay or an optoisolator or similar device to actuate the device's power button.  A pull-down resistor will probably be needed.
* The power monitor pin can be connected to one leg of an LED or maybe VCC of the controlled device to detect the power state.  Keep in mind input voltage limits.
* Various versions of the ESP8266 have requirements regarding GPIO pin state during power on.  Be careful about which pins you choose in this sketch.

##REQUIREMENTS:
PubSubClient: https://github.com/knolleary/pubsubclient

##ACKNOWLEDGEMENTS
Large portions of the code here are based on the "Basic ESP8266 MQTT example" from PubSubClient by Nick O'Leary available at https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
