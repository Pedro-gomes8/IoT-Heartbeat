import paho.mqtt.client as mqtt
import time
import random

# RabbitMQ MQTT broker details
BROKER = "localhost"
PORT = 1883
TOPIC = "heartbeat/sensor1/data"


def publish_heartbeat():
    client = mqtt.Client("Publisher")
    client.connect(BROKER, PORT, 60)

    while True:
        # Simulate heartbeat data
        heartbeat = random.randint(60, 100)  # Example: random BPM
        message = f"Heartbeat: {heartbeat}"

        # Publish the data to the topic
        client.publish(TOPIC, message)
        print(f"Published: {message}")

        time.sleep(1)  # Send data every second


if __name__ == "__main__":
    publish_heartbeat()
