#include <Arduino.h>
#include <OneButton.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LAMP_PIN 1
#define POMP_PIN 2
#define BUTT_PIN 0

OneButton button(0, true);
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "ERRS"; // Название WIFI
const char* password = "enes5152"; // Код от WIFI
const char* mqtt_server = "192.168.1.112"; // IP mqtt сервера
const char* name = "flower_water";

String topics[] = {"water/now", "water/time", "water/info"};

unsigned long timer;

bool flag_water;

int int_timer_water = 60;

String str_timer_water;
char char_timer_water[3];



// TODO: Добавить настройку времени, после которого происходит полив



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


  if (strTopic == "water/now"){ // Полив по команде
    if (strPayload == "on"){
      flag_water = true;
    }
    else if (strPayload == "off"){
      flag_water = false;
    }
  }
  
  if (strTopic == "water/time"){ // Как долго происходит полив
    int_timer_water = strPayload.toInt();
  }

  if (strTopic == "water/info"){
    if (strPayload == "update"){
      if (flag_water){
        client.publish("water/info/now", "on");
      }
      else if (!flag_water){
        client.publish("water/info/now", "off");
      }
      delay(500);

      client.publish("water/info/time", char_timer_water);
    }
  }
}




void connect() {
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(name)) {
      for (int i = 0; i < (int)sizeof(topics); i++) {
        char topic[topics[i].length()];
        topics[i].toCharArray(topic, topics[i].length());
        client.subscribe(topic);
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


void topicSub(){ // Подписываемся на топики
  client.subscribe("water/now");
  client.subscribe("water/time");
  client.subscribe("water/info");
}



void setup() {
  pinMode(POMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  
  initWifi();
  client.setServer(mqtt_server, 1883);

  digitalWrite(POMP_PIN, LOW);
}



void loop() {
  if (!client.connected()) // Если потеряли подключение
    connect(); // Переподключаемся

  button.tick();

  client.loop();

  str_timer_water = (String)int_timer_water;
  str_timer_water.toCharArray(char_timer_water, 3);

  if (flag_water){ // Если флаг активирован, то включаем помпу и следим за таймером отключения
    if (millis() - timer > int_timer_water * 1000){
      timer = millis();
      flag_water = false;
    }
    digitalWrite(POMP_PIN, HIGH);
    digitalWrite(LAMP_PIN, HIGH);
  }
  else {
    digitalWrite(POMP_PIN, LOW);
    digitalWrite(LAMP_PIN, LOW);
  }


  // Включение полива по большому таймеру
  if (!flag_water) { 
    if (millis() - timer > 10*1000) {
      timer = millis();
      flag_water= true;
    }
  }
}