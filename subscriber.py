import sqlite3
import paho.mqtt.client as mqtt
import json


# MQTT Broker details
broker = "192.168.43.119"  # Replace with your broker's IP address
port = 1883
topic = "sensor/data"


# Database setup
conn = sqlite3.connect("heartbeat.db")
c = conn.cursor()
c.execute(
    """CREATE TABLE IF NOT EXISTS heartbeat (timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, value INTEGER)"""
)
conn.commit()


# Callback for when the client receives a CONNACK response from the server
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
        client.subscribe(topic)
    else:
        print(f"Failed to connect, return code {rc}")


# Callback for when a message is received from the server
def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    heartbeat = data["heartbeat"]
    print(f"Received heartbeat: {heartbeat}")
    c.execute("INSERT INTO heartbeat (value) VALUES (?)", (heartbeat,))
    conn.commit()


# Set up MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to broker
client.connect(broker, port, 60)

# Blocking loop to process network traffic and dispatch callbacks
client.loop_forever()
