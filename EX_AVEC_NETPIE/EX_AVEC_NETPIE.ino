#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include "DHT.h"
#include <ESPert.h>

const char* ssid     = "AVEC_RMUTL";  // Change your ssid wifi
const char* password = "avecqwerty";  // Change your password wifi

#define APPID   "HelloCMMC"             // Change your appID
#define KEY     "BZYTbAa9ItnMyeW"       // Change your Key
#define SECRET  "h4aeKNOFIlSatTS8ADNk3Ft3O" // Change your SECRET
#define ALIAS   "smartuniversity"       // Change your name

#define LEDPin 16
#define DHTPIN 12
#define DHTTYPE DHT22

WiFiClient client;
AuthClient *authclient;

int timer = 0;
MicroGear microgear(client);

ESPert espert;

DHT dht(DHTPIN, DHTTYPE);

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setName("mygear");
  //  microgear.subscribe("/mygear_slave");
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  Serial.print(topic);
  Serial.print(" : ");
  char strState[msglen];
  for (int i = 0; i < msglen; i++)
  {
    strState[i] = (char)msg[i];
    Serial.print((char)msg[i]);
  }

  Serial.println();

  String stateStr = String(strState).substring(0, msglen);

  Serial.print("Topic is ");
  Serial.println(topic);

  if (stateStr == "ON")
  {
    digitalWrite(LEDPin, LOW);
  }
  else if (stateStr == "OFF")
  {
    digitalWrite(LEDPin, HIGH);
  }
}

void setup() {
  /* Event listener */
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(LEDPin, OUTPUT);
  digitalWrite(16, LOW);

  espert.init();
  espert.oled.init();
  delay(2000);

  dht.begin();

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //uncomment the line below if you want to reset token -->
  microgear.resetToken();
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

void loop() {
  if (microgear.connected())
  {
    microgear.loop();

    // อ่านค่าจากเซ็นเซอร์ DHt22
    float tempread = dht.readTemperature();
    float humidread = dht.readHumidity();
    // ประกาศตัวแปรเพื่อเก็บค่าสำหรับส่งไปยัง netpie
    char temp[10];
    char humid[10];
    // แยกจำนวนเลขทศนิยมออก เพื่อเก็บไว้ในอาเรย์
    int tempread_decimal = (tempread - (int)tempread) * 100;
    int humidread_decimal = (humidread - (int)humidread) * 100;
    // บันทึกต่าในอาเรย์ในส่วนของจำนวนจริง และทศนิม
    sprintf(temp, "%d.%d", (int)tempread, tempread_decimal);
    sprintf(humid, "%d.%d", (int)humidread, humidread_decimal);
    // ส่งค่าไปยัง netpie
    microgear.chat("smartuniversity/Temperature", temp);
    microgear.chat("smartuniversity/Humidity", humid);

    espert.oled.clear();
    espert.oled.setTextSize(1);
    espert.oled.setTextColor(ESPERT_WHITE);
    espert.oled.setCursor(0, 32);

    espert.oled.println("Hello NETPIE");
    espert.oled.print("Temperature = ");
    espert.oled.println(dht.readTemperature());
    espert.oled.print("Humidity = ");
    espert.oled.println(dht.readHumidity());
    espert.oled.update();

    Serial.println("connect...");
    Serial.print("Temperature = ");
    Serial.print(tempread);
    Serial.print("\t");
    Serial.print("Humidity = ");
    Serial.println(humidread);
  }
  else
  {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
}
