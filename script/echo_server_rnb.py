import random
import json
from paho.mqtt import client as mqtt_client

# Update the following variables with your specific values
broker = 'localhost'
port = 1883

topic = "rednodebus/events/#"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, reason_code, properties):
        if reason_code == 0:
            print("Connected to MQTT Broker!")
            subscribe(client)
        else:
            print("Failed to connect!")

    def on_disconnect(client, userdata, flags, reason_code, properties):
        print("Disconnected from MQTT Broker!")

    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

        msg = json.loads(msg.payload.decode())
        if msg["name"] == "user_data":
            msg["name"] = "send_user_data"
            msg["flip"] = "off"
            print("Echoing " + str(int(len(msg["data"])/2)) + " bytes back to " + str(msg["uid"]))
            msg = json.dumps(msg)
            client.publish("rednodebus/commands", msg)

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    client.loop_forever()


if __name__ == '__main__':
    run()
