#include <ESP8266WiFi.h>
#include <MQTT.h>

#include "secrets.h"

const char* wifiSsid = WIFISSID;
const char* wifiPass = WIFIPASS;

const char* mqttIP   = MQTTIP;
const char* mqttUser = MQTTUSER;
const char* mqttPass = MQTTPASS;

const char* deviceID = "BME680";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect()
{
  Serial.print("checking wifi connection...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(deviceID, mqttUser, mqttPass))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("stat/light_dinnertable/RESULT");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup()
{
  delay(5000);
  Serial.begin(115200);
  WiFi.begin(wifiSsid, wifiPass);
  client.begin(mqttIP, net);
  client.onMessage(messageReceived);

  connect();
}

void loop()
{
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    client.publish("sensors/BME680/string", "fooo");
    client.publish("sensors/BME680/millis", String(millis()));

  }
}
