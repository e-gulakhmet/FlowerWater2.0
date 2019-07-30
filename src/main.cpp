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

unsigned long timer;

bool flag_water;

byte timer_water = 60;
bool flag_day1;
bool flag_day2;
bool flag_day3;

String strTimer;



void showInfo(int count){
  for(int i = 0; i < count; i++){
      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);
  }
}



void setupWifi(){ // Настройка WIFI
  delay(10);
  // We start by connecting to a WiFi network
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



void reconnect() { // Если мы потеряли подключение то включаем перезагрузку
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void topicSub(){ // Подписываемся на топики
  client.subscribe("water/now");
  client.subscribe("water/day1");
  client.subscribe("water/day2");
  client.subscribe("water/day3");
  client.subscribe("water/time");
  client.subscribe("water/time");
  client.subscribe("water/info");
}



void callback(char* topic, byte* payload, unsigned int length) { // Функция в которой обрабатываются все присланные команды
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


  if(strTopic == "water/now"){ // Полив по нажатию кнопку на телефоне 
    if(strPayload == "on"){
      flag_water = true;
    }
    else if(strPayload == "off"){
      flag_water = false;
    }
  }
  
  if(strTopic == "water/time"){ // Как долго происходит полив
    timer_water = strPayload.toInt();
  }

  if(strTopic == "water/day1"){ // Раз в день
    if(strPayload == "on") {
      flag_day1 = true;
      flag_day2 = false;
      flag_day3 = false;
      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);
    }
    else if(strPayload == "off"){
      flag_day1 = false;
    }
  }

  if(strTopic == "water/day2"){ // Раз в два дня
    if(strPayload == "on") {
      flag_day2 = true;
      flag_day1 = false;
      flag_day3 = false;
      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);

      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);
    }
    else{
      flag_day2 = false;
    }
  }

  if(strTopic == "water/day3"){ // Раз в три дня
    if(strPayload == "on") {
      flag_day3 = true;
      flag_day1 = false;
      flag_day2 = false;
      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);

      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);

      delay(500);
      digitalWrite(LAMP_PIN, HIGH);
      delay(100);
      digitalWrite(LAMP_PIN, LOW);
    }
    else{
      flag_day3 = false;
    }
  }




  if(strTopic == "water/info"){
    if(strPayload == "update"){
      if(flag_water){
        client.publish("water/info/now", "on");
      }
      else if(!flag_water){
        client.publish("water/info/now", "off");
      }
      delay(500);
      
      if(flag_day1){
        client.publish("water/info/day1", "on");
      }
      else if(!flag_day1){
        client.publish("water/info/day1", "off");
      }
      delay(500);

      if(flag_day2){
        client.publish("water/info/day2", "on");
      }
      else if(!flag_day2){
        client.publish("water/info/day2", "off");
      }
      delay(500);

      if(flag_day3){
        client.publish("water/info/day3", "on");
      }
      else if(!flag_day3){
        client.publish("water/info/day3", "off");
      }
      delay(500);

      client.publish("water/info/time", strTimer);
    }
  }



}



void click(){
  if(flag_day1){ // Изменение с помощью кнопки
    flag_day1 = false;
    flag_day2 = true;
    showInfo(2);
  }
  else if(flag_day2){
    flag_day1 = false;
    flag_day2 = false;
    flag_day3 = true;
    showInfo(3);
  }
  else if(flag_day3){
    flag_day2 = false;
    flag_day3 = false;
    flag_day1 = true;
    showInfo(1);
  }
  else{
    flag_day1 = false;
    flag_day2 = true;
    showInfo(2);
  }
}

void startPress(){
  flag_water = true;
}

void stopPress(){
  flag_water = false;
}



void setup() {
  pinMode(POMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  
  setupWifi();
  client.setServer(mqtt_server, 1883);

  button.attachClick(click);
  button.attachLongPressStop(stopPress);
  button.attachLongPressStart(startPress);
  digitalWrite(POMP_PIN, LOW);
}



void loop() {
  if (!client.connected()){ // Если потеряли подключение
    reconnect(); // Переподключаемся
    topicSub();
    client.setCallback(callback);
  }

  button.tick();

  strTimer = String(50);

  if(flag_water){ // Если флаг активирован, то включаем помпу и следим за таймером отключения
    if(millis() - timer > timer_water * 1000){
      timer = millis();
      flag_water = false;
    }
    digitalWrite(POMP_PIN, HIGH);
    digitalWrite(LAMP_PIN, HIGH);
  }
  else{
    digitalWrite(POMP_PIN, LOW);
    digitalWrite(LAMP_PIN, LOW);
  }


  // Включение полива по большому таймеру
  if(flag_day1 && !flag_water){ 
    if(millis() - timer > 10*1000){
      timer = millis();
      flag_water= true;
    }
  }
  else if(flag_day2 && !flag_water){
    if(millis() - timer > 2*24*60*60*1000){
      timer = millis();
      flag_water= true;
    }
  }
  else if(flag_day2 && !flag_water){
    if(millis() - timer > 3*24*60*60*1000){
      timer = millis();
      flag_water= true;
    }
  }


  client.loop();
}