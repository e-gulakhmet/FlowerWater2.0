#ifndef _WIFI_HPP_
#define _WIFI_HPP_


#include <ESP8266WiFi.h>
#include <PubSubClient.h>



class Wifi {
    public:
        Wifi(String ssid, String password, String server_ip);

        void begin(std::function<void (char *, uint8_t *, unsigned int)> callback); // Подключение к wifi
        void update();
        void setTopics(String topics[]);
        void subOnTopics();
        String getResponse();

    private:
        String ssid_;
        String password_;
        String server_ip_;
        String response_[2];

        String topics_[];

        void connect();

        WiFiClient espClient_;
        PubSubClient client_;
    

        
};


#endif // _WIFI_HPP_