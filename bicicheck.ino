#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
// Update these with values suitable for your network.
const char *ssid = "HUAWEI-1035";
const char *password = "93542670";
const char *mqtt_server = "143.198.182.161";
char mqtt_payload[50] = "";

SoftwareSerial SerialGPS(4, 5);
TinyGPSPlus gps;

String inStringESCMessage = "";

WiFiClient espClient;
PubSubClient client(espClient);

void peep(int times)
{
  for (int i = 1; i <= times; i++)
  {
    digitalWrite(D3, HIGH);
    delay(350);
    digitalWrite(D3, LOW);
  }
}

void setup_wifi()
{
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(ssid);
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
  peep(2);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message from: ");
  Serial.print(topic);
  for (int i = 0; i < length; i++)
  {
    inStringESCMessage += (char)payload[i];
  }
  Serial.print(" | ");
  Serial.print("Value: ");
  inStringESCMessage = "";
  peep(1);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-BICIMAURI-Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    // if you MQTT broker has clientID,username and password
    // please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      peep(2);
      // once connected to MQTT broker, subscribe command if any
      client.subscribe("/BICI/MAURI");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      peep(6);
      delay(2000);
    }
  }
}

void setup()
{
  pinMode(D3, OUTPUT);
  peep(1);
  Serial.begin(9600);
  SerialGPS.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read()))
    {
      if (gps.location.isValid())
      {
        String data = String(gps.location.lat(), 6)+","+String(gps.location.lng(), 6)+","+gps.satellites.value()+","+gps.altitude.meters()+","+gps.course.deg()+","+gps.speed.kmph();
        // data = latitude,longitude,satelites,altitude,course,speed(kmp)
        client.publish("/BICI/MAURI/DATA",(char*) data.c_str(), true);
        //snprintf(mqtt_payload, 50, "%d", String(gps.satellites.value()));
      }
      delay(3000);
    }
}
