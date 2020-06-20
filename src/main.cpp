#include <Arduino.h>
#include <OneButton.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WiFiUdp.h"
#include "ArduinoOTA.h"
#include <bits/stdc++.h>

#include "main.hpp"

OneButton button(0, true);
WiFiClient espClient;
PubSubClient client(espClient);


String topics[] = {"water/info", "water/state", "water/period/off", "water/period/on"};

unsigned long period_timer;

bool is_watering = true;

uint period_time = 3*24*60*60;
uint watering_time = 60;



char* convertIntToChar(int value) {
  String str_value = (String)value;
  char char_value[str_value.length()];
  str_value.toCharArray(char_value, str_value.length());

  return char_value;
}



void callBack(char* topic, byte* payload, unsigned int length) { // Функция в которой обрабатываются все присланные команды
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");


  String strTopic = String(topic); // Строка равная топику
  String strPayload = ""; // Создаем пустую строку, которая будет хранить весь payload


  for (uint i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    strPayload = strPayload + ((char)payload[i]); // присваеваем значение по столбикам, чтобы получить всю строку
  }
  Serial.println();


  if (strTopic == topics[0]){ // Полив по команде
    is_watering = strPayload == "on" ? true : false;
  }

  if (strTopic == topics[2]) { // Время покая
    period_time = strPayload.toInt();
  }
  
  if (strTopic == topics[3]){ // Как долго происходит полив
    watering_time = strPayload.toInt();
  }

  if (strTopic == topics[0]){
    if (strPayload == "update"){
      client.publish(topics[1].c_str(), is_watering ? "on" : "off");
      client.publish(topics[2].c_str(), convertIntToChar(period_time));
      client.publish(topics[3].c_str(), convertIntToChar(watering_time));
    }
  }
}



void connect() {
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(name)) {
      for (int i = 0; i < 4; i++) {
        client.subscribe(topics[i].c_str());
      }
      client.setCallback(callBack);
      Serial.println("connected");
      Serial.println('\n');
    }
    else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// void connect() { // Если мы потеряли подключение то включаем перезагрузку
//   // Loop until we're reconnected
//   while(!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     // Create a random client ID
//     String clientId = "ESP8266Client-";
//     clientId += String(random(0xffff), HEX);
//     // Attempt to connect
//     if (client.connect(clientId.c_str())) {
//       Serial.println("connected");
//       // Once connected, publish an announcement...
//       client.publish("outTopic", "hello world");
//       // ... and resubscribe
//       client.subscribe("inTopic");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }



void initWifi(){ // Настройка WIFI
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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
// void initWifi(){ // Настройка WIFI
//   delay(10);
//   // We start by connecting to a WiFi network
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print('.');
//   }

//   randomSeed(micros());

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
// }




void initWifiUpd() {
  ArduinoOTA.setHostname("FLOWER-WATER"); // Задаем имя сетевого порта
  //ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
}



void setup() {
  Serial.begin(9600);

  //pinMode(POMP_PIN, OUTPUT);
  //pinMode(LAMP_PIN, OUTPUT);
  // digitalWrite(POMP_PIN, LOW);
  
  initWifi();
  delay(100);
  connect();
  
  initWifiUpd();
}



void loop() {
  if (!client.connected()) // Если потеряли подключение
    connect(); // Переподключаемся

  button.tick();

  client.loop();

  ArduinoOTA.handle(); // Всегда готовы к прошивке


  // for (int i = 0; i < sizeof(topics); i++) {
  //   Serial.println(convertStrToChar(topics[i]));
  // }

  if (is_watering){ // Если флаг активирован, то включаем помпу и следим за таймером отключения
    static unsigned long timer;
    if (millis() - timer > watering_time * 1000){
      timer = millis();
      is_watering = false;
    }
    digitalWrite(POMP_PIN, HIGH);
    digitalWrite(LAMP_PIN, HIGH);
  }
  else {
    digitalWrite(POMP_PIN, LOW);
    digitalWrite(LAMP_PIN, LOW);
  }


  // Включение полива по большому таймеру
  if (!is_watering) { 
    if (millis() - period_timer > period_time) {
      period_timer = millis();
      is_watering= true;
    }
  }
}