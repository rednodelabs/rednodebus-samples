import time
from prettytable import PrettyTable
import random
import json
from paho.mqtt import client as mqtt_client

# Update the following variables with your specific values
broker = 'localhost'
port = 1883

euid_len = 12
session_rand_len = 4
ctr_len = 4

table_header = ["UID", "Packets received", "Packets lost", "Error rate (%)", "Bytes received", "Bitrate (Bps)"]
stats_dic = {}

topic = "rednodebus/events/#"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

def process_msg(payload):
    euid = str(payload[0:euid_len], 'utf-8')
    session_rand = int.from_bytes(payload[euid_len: euid_len + session_rand_len], byteorder='little', signed=False)
    ctr = int.from_bytes(payload[euid_len + session_rand_len: euid_len + session_rand_len + ctr_len], byteorder='little', signed=False)

    if euid in stats_dic:
        if stats_dic[euid][0] != session_rand:
            stats_dic[euid] = [session_rand, ctr, 1, 0, 0, len(payload), 0, time.time()]
        else:
            stats_dic[euid][3] += ctr - stats_dic[euid][1] - 1
            stats_dic[euid][1] = ctr
            stats_dic[euid][2] += 1
            stats_dic[euid][4] = 100*stats_dic[euid][3]/(stats_dic[euid][3] + stats_dic[euid][2])
            stats_dic[euid][5] += len(payload)
            stats_dic[euid][6] = stats_dic[euid][5]/(time.time() - stats_dic[euid][7])
    else:
        stats_dic[euid] = [session_rand, ctr, 1, 0, 0, len(payload), 0, time.time()]

    # Specify the Column Names while initializing the Table
    myTable = PrettyTable(table_header)

    # Add rows
    for key in stats_dic:
        myTable.add_row([key, stats_dic[key][2], stats_dic[key][3], round(stats_dic[key][4], 2), stats_dic[key][5], round(stats_dic[key][6], 1)])

    print(myTable)

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
        msg = json.loads(msg.payload.decode())
        if msg["name"] == "user_data":
            msg["name"] = "send_user_data"
            process_msg(bytearray.fromhex(msg["data"]))

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    client.loop_forever()


if __name__ == '__main__':
    run()


