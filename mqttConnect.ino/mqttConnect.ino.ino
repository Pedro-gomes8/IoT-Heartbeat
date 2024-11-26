#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#define HBTHRESHOLD(x) ((x) < 60 || (x) > 100) // Threshold


#define LEDPIN 3 // a changer
#define BUZZER 4 // a changer
#define MOVINGAVERAGEQUANTITY 5

const char* ssid = "Redmi Note 9S";
const char* password = "commentestvotreblanquette?";

// MQTT Broker 
const char* mqtt_server = "192.168.43.119"; // Replace with your broker address
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/data";
int nextMesure = 5000;

int movingAverageCount = 0;
int idx = 0;
int sum = 0;
int movingAverage[MOVINGAVERAGEQUANTITY];

WiFiClient espClient;
PubSubClient client(espClient);

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
}

void blinkandBuzz(){

  nextMesure = 2000;
  for (int i = 0; i<5; i++){
  digitalWrite(LEDPIN,HIGH);

  digitalWrite(BUZZER,HIGH);
  delay(10);

  digitalWrite(BUZZER,LOW);
  delay(50);


  digitalWrite(LEDPIN,LOW);
  }

}



void checkAbnormalities(int heartbeat){\
  static int c;
  if (HBTHRESHOLD(heartbeat)){
    blinkandBuzz();
  }
}


void loop() {
  nextMesure = 5000;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Send info via MQTT
  StaticJsonDocument<200> doc;
  int heartbeat_value =  random(60, 100); 


  // Check abnormalities
  if (HBTHRESHOLD(heartbeat_value)){
    blinkandBuzz();
  }

  if (movingAverageCount < MOVINGAVERAGEQUANTITY){
    movingAverage[idx] = heartbeat_value;
    sum+= heartbeat_value;
    movingAverageCount++;
  }else{
    sum-= movingAverage[idx];
    movingAverage[idx] = heartbeat_value;
    sum+= heartbeat_value;
  }
  idx = (idx + 1 ) % MOVINGAVERAGEQUANTITY;


  doc["heartbeat"] = heartbeat_value;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(mqtt_topic, jsonBuffer);
  
  delay(nextMesure);
    
}
