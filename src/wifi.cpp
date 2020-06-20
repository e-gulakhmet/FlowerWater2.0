#include <wifi.hpp>


Wifi::Wifi(String ssid, String password, String server_ip)
    : ssid_(ssid)
    , password_(password)
    , server_ip_(server_ip)
    , client_(espClient_)
    {

    }



void Wifi::begin(std::function<void (char *, uint8_t *, unsigned int)> callback) {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid_);

  WiFi.begin(ssid_, password_);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(100);
  connect();
  client_.setCallback(callback);
}



void Wifi::update() {
  if (!client_.connected()){ // Если потеряли подключение
    // Переподключаемся
    connect();
    subOnTopics();
  }
}



void Wifi::connect() {
    // Loop until we're connected
    while (!client_.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client_.connect(clientId.c_str())) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        client_.publish("outTopic", "hello world");
        // ... and resubscribe
        client_.subscribe("inTopic");
        } else {
        Serial.print("failed, rc=");
        Serial.print(client_.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        }
    }
}


void Wifi::setTopics(String topics[]) {
    for (int i = 0; i < (int)sizeof(topics); i++)
        topics_[i] = topics[i];
}



void Wifi::subOnTopics() {
    for (int i = 0; i < (int)sizeof(topics_); i++) {
        char topic[topics_[i].length()];
        topics_[i].toCharArray(topic, topics_[i].length());
        client_.subscribe(topic);
    }
}