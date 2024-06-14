//Inisialisasikan library yang dibutuhkan
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "DHT.h"

//Masukan dpt pin yang dikoneksikan
#define DHT11PIN 4

//inisialisasikan dht pin dan dht11
DHT dht(DHT11PIN, DHT11);

//Masukkan property wifi
const char* namaWifi = "Jini";
const char* sandi = "@13456#";
const char* mqtt_server = "test.mosquitto.org";
const int port = 1883;

//Buat client dan buat variable temperature dan humidity
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

//Membuat variable yang menyimpan alamat topic
const char* topicTemperature = "/home/sensor/data/temperature";
const char* topicHumidity = "/home/sensor/data/humidity";
const char* topicCommand = "/home/command/mqtt";


void setup_wifi() { 
  delay(200);
  //Output yang ditampilkan saat mengokensikan ke jaringan wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(namaWifi);

  //Mulai mengoneksikan ke jaringan wifi
  WiFi.mode(WIFI_STA); 
  WiFi.begin(namaWifi, sandi); 

  //Output yang dihasilka saat menunggu koneksi
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

//Menampilkan data pada mqtt
void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   
  } else {
    digitalWrite(2, HIGH);  
  }
}

//Koneksi ke mqtt
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // inisialisasikan client id
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      //publish data ketika terkoneksi
      client.publish("/home/command/mqtt", "Hello!"); 
      //subscribe ke mqtt
      client.subscribe(topicCommand); 
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  //Menginisialisasikan output
  pinMode(2, OUTPUT); 
  //Setting alamat serial monitor    
  Serial.begin(115200);
  //Memanggil funsi setup wifi
  setup_wifi(); 
  //Mengoneksikan ke mqtt
  client.setServer(mqtt_server, port); 
  client.setCallback(callback); 
  //Memulai sensor dht11
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { 
    lastMsg = now;
    float humi = dht.readHumidity();
    float temp = dht.readTemperature();

    //Membuat format data temperatur yang akan dikirim kepada broker mqtt
    String tempString = String(temp, 2);
    //publish data ke mqtt
    client.publish(topicTemperature, tempString.c_str()); 
    
    //Membuat format data humidity yang akan dikirim kepada broker mqtt
    String humString = String(humi, 1); 
    //publish data ke mqtt
    client.publish(topicHumidity, humString.c_str()); 

    //Menampilkan hasil sensor pada serial monitor
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(humi);
  }
}