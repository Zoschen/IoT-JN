
#include <Arduino.h>
#include <SDS011.h>
#include <Adafruit_BMP085.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

float p10, p25;
int error;

SDS011 my_sds;

Adafruit_BMP085 bmp;
float Temp;
float pressure;
float Altitude;
float PaSl;
float rAltitude;

// Update these with values suitable for your network.
const char *ssid = "Nour";
const char *password = "Nour4321";
const char *mqtt_server = "192.168.137.18 "; // von der virtuellen Maschine unter enp0s9

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Nour", "hello there (reconnected)");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // config for the Dust Sensor pins
  my_sds.begin(D4, D3); // RX, TX

  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Wire.begin();
}

void loop()
{

  error = my_sds.read(&p25, &p10);
 /* if (!error)
  {
    Serial.println("P2.5: " + String(p25));
    Serial.println("P10:  " + String(p10));
  }
  else
  {
    Serial.println("no PM-Sensor data is available!");
  }
  // delay(100);

  Serial.println("");
  */
  if (bmp.begin())
  {
    Temp = bmp.readTemperature();

    pressure = bmp.readPressure();

    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Altitude = bmp.readAltitude();

    PaSl = bmp.readSealevelPressure();

    // you can get a more precise measurement of altitude
    // if you know the current sea level pressure which will
    // vary with weather and such. If it is 1015 millibars
    // that is equal to 101500 Pascals.
    rAltitude = bmp.readAltitude();
  }

  Serial.println("");
  delay(500);

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000)
  {
    lastMsg = now;
    ++value;
    if (!error)
    {
      snprintf(msg, MSG_BUFFER_SIZE, "%f", p25);
      Serial.print("Publish p2.5: ");
      Serial.println(msg);
      client.publish("outTopic/Feinstaubsensor/p25", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "%f", p10);
      Serial.print("Publish p10: ");
      Serial.println(msg);
      client.publish("outTopic/Feinstaubsensor/p10", msg);
    }
    else
    {
      snprintf(msg, MSG_BUFFER_SIZE, "No PM-Sensor data available #%ld", 0);
      Serial.print("Publish p2.5: ");
      Serial.println(msg);
      client.publish("outTopic/Feinstaubsensor/p25", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "No PM-Sensor data available #%ld", 0);
      Serial.print("Publish p10: ");
      Serial.println(msg);
      client.publish("outTopic/Feinstaubsensor/p10", msg);
    }

    if (!bmp.begin())
    {
      snprintf(msg, MSG_BUFFER_SIZE, "No BMP-Sensor data available #%ld", 0);
      Serial.print("Publish Temperature: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Temperature", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "No BMP-Sensor data available #%ld", 0);
      Serial.print("Publish pressure: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/pressure", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "No BMP-Sensor data available #%ld", 0);
      Serial.print("Publish Altitude: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Altitude", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "No BMP-Sensor data available #%ld", 0);
      Serial.print("Publish Pressure at Sea level: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Pressure_at_Sea_level", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "No BMP-Sensor data available #%ld", 0);
      Serial.print("Publish real Altitude: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/real_Altitude", msg);
    }
    else
    {
      snprintf(msg, MSG_BUFFER_SIZE, "%f", Temp);
      Serial.print("Publish Temperature: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Temperature", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "%f", pressure);
      Serial.print("Publish pressure: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/pressure", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "%f", Altitude);
      Serial.print("Publish Altitude: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Altitude", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "%f", PaSl);
      Serial.print("Publish Pressure at Sea level: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/Pressure_at_Sea_level", msg);

      snprintf(msg, MSG_BUFFER_SIZE, "%f", rAltitude);
      Serial.print("Publish real Altitude: ");
      Serial.println(msg);
      client.publish("outTopic/BMP-Sensor/real_Altitude", msg);
    }
  }

  /*  client.subscribe(led_button);
    if (led_button == "1")
    {
    }
    if (led_button == "0")
    {
    }*/
}