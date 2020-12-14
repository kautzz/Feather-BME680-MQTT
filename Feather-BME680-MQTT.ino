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

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1015.00)
Adafruit_BME680 bme;

int vBatt = 0;

unsigned long lastUpdate = 0;

void connect()
{
  Serial.print("* Checking WIFI connection ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(0, LOW);
    // TODO handle wifi disconnect
    delay(1000);
  }

  Serial.println("");
  Serial.print("* Connecting to MQTT as: " + String(deviceID) + " ");
  int i = 0;
  while (!client.connect(deviceID, mqttUser, mqttPass))
  {
    Serial.print(".");
    digitalWrite(0, LOW);
    delay(1000);
    i++;
    if(i == 3)
    {
      Serial.println("* NO MQTT MODE");
      break;
    }
  }

  Serial.println("");
  if (!bme.begin()) {
    Serial.println("* Could not find BME680 sensor, check wiring!");
    digitalWrite(0, LOW);

  } else {
    Serial.println("* BME680 SENSOR CONNECTED");
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_16X);
    bme.setHumidityOversampling(BME680_OS_16X);
    bme.setPressureOversampling(BME680_OS_16X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 300); // 320*C for 300 ms
  }

  client.subscribe("stat/light_dinnertable/RESULT");
  digitalWrite(0, HIGH);

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
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);

  digitalWrite(2,HIGH);

  delay(5000);

  Serial.begin(115200);
  Serial.println("* Connecting to WIFI: " + String(wifiSsid));
  WiFi.begin(wifiSsid, wifiPass);
  client.begin(mqttIP, net);
  client.onMessage(messageReceived);

  connect();
}

void loop()
{
  client.loop();
  if (!client.connected()) { connect(); }

  if (millis() - lastUpdate > 5000)
  {
    Serial.println("% ---- ");


    vBatt = analogRead(A0);
    Serial.print("> Sending: battery voltage = ");
    Serial.print(vBatt*5.5/1000);
    Serial.println(" V");
    client.publish("SENSORS/BME680/vBatt", String(vBatt*5.5/1000));

    vBatt = map(vBatt, 580, 774, 0, 100);
    Serial.print("> Sending: battery percentage = ");
    Serial.print(vBatt);
    Serial.println(" %");
    client.publish("SENSORS/BME680/pBatt", String(vBatt));

    if (! bme.performReading())
    {
      Serial.println("* Failed to perform reading :(");
      connect();
    } else {

      digitalWrite(2, LOW);

      Serial.print("> Sending: temperature = ");
      Serial.print(bme.temperature);
      Serial.println(" *C");
      client.publish("SENSORS/BME680/temperature", String(bme.temperature));

      Serial.print("> Sending: pressure = ");
      Serial.print(bme.pressure / 100.0);
      Serial.println(" hPa");
      client.publish("SENSORS/BME680/pressure", String(bme.pressure / 100.0));

      Serial.print("> Sending: humidity = ");
      Serial.print(bme.humidity);
      Serial.println(" %");
      client.publish("SENSORS/BME680/humidity", String(bme.humidity));

      Serial.print("> Sending: gas_resistance = ");
      Serial.print(bme.gas_resistance / 1000.0);
      Serial.println(" KOhms");
      client.publish("SENSORS/BME680/gas_resistance", String(bme.gas_resistance / 1000.0));

      Serial.print("> Sending: approx. altitude = ");
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      Serial.println(" m");
      client.publish("SENSORS/BME680/altitude", String(bme.readAltitude(SEALEVELPRESSURE_HPA)));


      digitalWrite(2, HIGH);
    }
    lastUpdate = millis();
  }
}
