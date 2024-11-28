#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define HBTHRESHOLD(x) ((x) < 40 || (x) > 120) // Threshold
#define RAPIDCHANGE 30 // Rapid change in heartbeat

#define LEDPIN 3 // a changer
#define BUZZER 4 // a changer
#define MOVINGAVERAGEQUANTITY 5
#define SDA 18
#define SCL 17
PulseOximeter pox;

const char* ssid = "Redmi Note 9S";
const char* password = "commentestvotreblanquette?";


uint32_t tsLastReport = 0;

// MQTT Broker 
const char* mqtt_server = "192.168.43.119"; // Replace with your broker address
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/data";
int nextMesure = 2000;

int movingAverageCount = 0;
int idx = 0;
int sum = 0;
int movingAverage[MOVINGAVERAGEQUANTITY];

WiFiClient espClient;
PubSubClient client(espClient);

void onBeatDetected()
{
    Serial.println("Beat!");
}

bool checkRapidChange(int heartbeat){

  if (movingAverageCount < MOVINGAVERAGEQUANTITY){
    movingAverage[idx] = heartbeat;
    sum+= heartbeat;
    movingAverageCount++;
    idx = (idx + 1) % MOVINGAVERAGEQUANTITY;
    return false;
  }else{
    
    int avg = sum / movingAverageCount;
    bool rapid = abs(avg - heartbeat) > RAPIDCHANGE;
    sum -= movingAverage[idx];
    movingAverage[idx] = heartbeat;
    sum += heartbeat;
    idx = (idx + 1) % MOVINGAVERAGEQUANTITY;
    return rapid;
    }
  }
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  Wire.begin(SDA, SCL);
  client.connect("ESP32");


  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void blinkandBuzz(){
  for (int i = 0; i<5; i++){
    digitalWrite(LEDPIN,HIGH);

    digitalWrite(BUZZER,HIGH);
    delay(10);

    digitalWrite(BUZZER,LOW);
    delay(50);


    digitalWrite(LEDPIN,LOW);
    delay(50);
  }

}



void checkAbnormalities(int heartbeat){
  if (HBTHRESHOLD(heartbeat) || (checkRapidChange(heartbeat))){
    blinkandBuzz();
  }
}


void loop() {
  nextMesure = 2000;
  if (millis() - tsLastReport > nextMesure) {
    pox.update();
    int heartbeat_value = (int)pox.getHeartRate();
    Serial.println(heartbeat_value);
    checkAbnormalities(heartbeat_value);
    
    StaticJsonDocument<200> doc;
    doc["heartbeat"] = heartbeat_value;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);
    client.publish(mqtt_topic, jsonBuffer);

    tsLastReport = millis();
  }
  client.loop();

}
