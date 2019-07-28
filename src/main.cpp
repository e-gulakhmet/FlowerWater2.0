#include <Arduino.h>
#include <OneButton.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

OneButton button(0, true);
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "ERRS"; // Название WIFI
const char* password = "enes5152"; // Код от WIFI
const char* mqtt_server = "192.168.1.112"; // IP mqtt сервера

bool water;



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
  client.subscribe("water");
}



void click(){
  digitalWrite(2, HIGH);
}


void startPress(){
  digitalWrite(2, LOW);
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


  if(strTopic == "water"){
    if(strPayload == "on"){
      water = true;
    }
    else if(strPayload == "off"){
      water = false;
    }
  }

}


void setup() {
  pinMode(2, OUTPUT);
  
  setupWifi();
  client.setServer(mqtt_server, 1883);

  button.attachClick(click);
  button.attachLongPressStart(startPress);
  digitalWrite(2, LOW);
}

void loop() {
  if (!client.connected()){ // Если потеряли подключение
    reconnect(); // Переподключаемся
    topicSub();
    client.setCallback(callback);
  }

  button.tick();

  if(water){
    digitalWrite(2, HIGH);
  }
  else{
    digitalWrite(2, LOW);
  }


  client.loop();
}