#include <Arduino.h>
#include <GyverButton.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WiFiUdp.h"
#include "ArduinoOTA.h"

#include "main.hpp"

GButton button(0);
WiFiClient espClient;
PubSubClient client(espClient);


String topics[] = {"water/info", "water/state", "water/period/off", "water/period/on"};

unsigned long timer;

bool is_watering = true;
bool is_auto = false;

int period_time = 3*24*60*60;
int watering_time = 60;



void callBack(char* topic, byte* payload, unsigned int length) { // Функция в которой обрабатываются все присланные команды
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");


  String strTopic = String(topic); // Строка равная топику
  String strPayload = ""; // Создаем пустую строку, которая будет хранить весь payload


  for (int i = 0; i < length; i++) {
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
      delay(100);
      client.publish(topics[2].c_str(), String(period_time).c_str());
      delay(100);
      client.publish(topics[3].c_str(), String(watering_time).c_str());
    }
  }
}



void connect() {
  client.setServer(mqtt_server, 1883);
  delay(1000);
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
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}



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


void initWifiUpd() {
  ArduinoOTA.setHostname("FLOWER-WATER"); // Задаем имя сетевого порта
  //ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
}



void setup() {
  Serial.begin(9600);

  pinMode(POMP_PIN, OUTPUT);
  
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

  if (button.isHolded()) {
    is_auto = is_auto ? false : true;
  }

  if (is_auto) {
    if (is_watering){ // Если флаг активирован, то включаем помпу и следим за таймером отключения
      if (millis() - timer > watering_time * 1000){
        timer = millis();
        is_watering = false;
      }
      digitalWrite(POMP_PIN, HIGH);
      digitalWrite(LAMP_PIN, HIGH);
    }
    else {
      // Включение полива по большому таймеру
      if (millis() - timer > period_time) {
        timer = millis();
        is_watering= true;
      }
      digitalWrite(POMP_PIN, LOW);
      digitalWrite(LAMP_PIN, LOW);
    }
  }
  else {
    if (button.isHold()) {
      digitalWrite(POMP_PIN, HIGH);
      digitalWrite(LAMP_PIN, HIGH);
    }
    if (button.isRelease()) {
      digitalWrite(POMP_PIN, LOW);
      digitalWrite(LAMP_PIN, LOW);      
    }
  }
}