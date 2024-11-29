#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define HBTHRESHOLD(x) ((x) < 30 || (x) > 100) // Threshold


#define LEDPIN 21 // a changer
#define BUZZER 38 // a changer
#define MOVINGAVERAGEQUANTITY 5
#define SDA 18
#define SCL 17
PulseOximeter pox;

// Setup internet connection
const char* ssid = "Redmi Note 9S";
const char* password = "commentestvotreblanquette?";

// MQTT Broker 
const char* mqtt_server = "192.168.43.119"; // Replace with your broker address
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/data";


// Initializing variables
int nextMesure = 2000;

int movingAverageCount = 0;
int idx = 0;
int sum = 0;
int movingAverage[MOVINGAVERAGEQUANTITY];
int average = 0;

WiFiClient espClient;
PubSubClient client(espClient);

uint32_t tsLastReport = 0;

void onBeatDetected()
{
    Serial.println("Beat!");
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
    } 
  }
}

void setup() {
  Serial.begin(115200);
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
  pinMode(LEDPIN,OUTPUT);
  pinMode(BUZZER,OUTPUT);

}

void blinkandBuzz(){

  digitalWrite(LEDPIN,HIGH);
  tone(BUZZER,6000,500);

}



void checkAbnormalities(int heartbeat){\
  static int c;
  if (HBTHRESHOLD(heartbeat)){
    blinkandBuzz();
  }
  else
  {
    tone(BUZZER,0,500);
    digitalWrite(LEDPIN,LOW);
  }
}


void loop() {
  pox.update();
  nextMesure = 2000;


  if (millis() - tsLastReport > nextMesure)
  {

    // Send info via MQTT
    StaticJsonDocument<200> doc;

    int heartbeat_value = (int)pox.getHeartRate();
    Serial.println(heartbeat_value);



    checkAbnormalities(heartbeat_value);

    if (movingAverageCount < MOVINGAVERAGEQUANTITY){
      movingAverage[idx] = heartbeat_value;
      sum+= heartbeat_value;
      movingAverageCount++;
    }else{
      sum-= movingAverage[idx];
      movingAverage[idx] = heartbeat_value;
      sum+= heartbeat_value;
    }
    average = sum /  MOVINGAVERAGEQUANTITY;
    idx = (idx + 1 ) % MOVINGAVERAGEQUANTITY;

    if(HBTHRESHOLD(average))
    {
      //blinkandBuzz();
      Serial.println("Blinkin n' buzzin moyenne");
    }


    doc["heartbeat"] = heartbeat_value;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);
    client.publish(mqtt_topic, jsonBuffer);

    tsLastReport = millis();
    }

    client.loop();
   
}