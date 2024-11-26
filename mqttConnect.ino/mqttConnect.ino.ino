#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define HBTHRESHOLD(x) ((x) < 60 || (x) > 100) // Threshold


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

  nextMesure = 1000;
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
  nextMesure = 2000;
  pox.update();
      if (millis() - tsLastReport > nextMesure) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");

        tsLastReport = millis();
    }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Send info via MQTT
  StaticJsonDocument<200> doc;
  // int heartbeat_value =  random(60, 100); 
  float heartbeat_value = pox.getHeartRate();
  Serial.println(heartbeat_value);

  // // Check abnormalities
  // if (HBTHRESHOLD(heartbeat_value)){
  //   blinkandBuzz();
  // }

  // if (movingAverageCount < MOVINGAVERAGEQUANTITY){
  //   movingAverage[idx] = heartbeat_value;
  //   sum+= heartbeat_value;
  //   movingAverageCount++;
  // }else{
  //   sum-= movingAverage[idx];
  //   movingAverage[idx] = heartbeat_value;
  //   sum+= heartbeat_value;
  // }
  // idx = (idx + 1 ) % MOVINGAVERAGEQUANTITY;


  doc["heartbeat"] = heartbeat_value;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(mqtt_topic, jsonBuffer);
  
  // delay(nextMesure);
    
}

/*
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Configuration réseau
const char* ssid = "Redmi Note 9S";
const char* password = "commentestvotreblanquette?";

// Configuration MQTT
const char* mqtt_server = "fe80::cb7:507b:5a50:6f58"; // Adresse IP du broker MQTT (ordinateur ou cloud)
const char* topic = "topic/topic"; // Topic pour publier les données

WiFiClient espClient;
PubSubClient client(espClient);

#define REPORTING_PERIOD_MS     1000
#define SDA 18
#define SCL 17

PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup() {
  Serial.begin(115200);

  // Connexion au WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au WiFi!");

  // Connexion au broker MQTT
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connecté au broker MQTT !");
    } else {
      Serial.print("Échec de la connexion, code d'erreur: ");
      Serial.println(client.state());
    }
  }

  Wire.begin(SDA, SCL);
  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }

  // The default current for the IR LED is 50mA and it could be changed
  //   by uncommenting the following line. Check MAX30100_Registers.h for all the
  //   available options.
  // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");

        tsLastReport = millis();
    }

  // Convertir la valeur en chaîne et la publier sur MQTT
  char message[50];
  sprintf(message, "%d", pox.getHeartRate());
  client.publish(topic, message);
  
  Serial.println("Pouls publié: " + String(pox.getHeartRate()));

  // Assurez-vous que le client MQTT fonctionne correctement
  client.loop();

  // Délai avant la prochaine lecture (par exemple, toutes les secondes)
  delay(1000);
}*/
