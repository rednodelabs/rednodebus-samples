import time
from prettytable import PrettyTable
import random
import json
from paho.mqtt import client as mqtt_client

# Update the following variables with your specific values
broker = 'localhost'
port = 1883

euid_bytes = 8
session_rand_bytes = 4
ctr_bytes = 4

table_header = ["UID", "X (m/s²)", "Y (m/s²)", "Z (m/s²)", "Packets received", "Packets lost", "Error rate (%)", "Bytes received", "Bitrate (Bps)"]
stats_dic = {}

topic = "rednodebus/events/#"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

def process_msg(payload):
    len_payload=len(payload)
    euid = hex(int.from_bytes(payload[0:euid_bytes], byteorder='little', signed=False)).replace("0x","").upper()
    session_rand = int.from_bytes(payload[euid_bytes: euid_bytes + session_rand_bytes], byteorder='little', signed=False)
    ctr = int.from_bytes(payload[euid_bytes + session_rand_bytes: euid_bytes + session_rand_bytes + ctr_bytes], byteorder='little', signed=False)
    data = [payload[i:i + 4] for i in range(euid_bytes + session_rand_bytes + ctr_bytes,len_payload,4)]
    x_val1 = data[0]
    x_val2 = data[1]
    y_val1 = data[2]
    y_val2 = data[3]
    z_val1 = data[4]
    z_val2 = data[5]
    x = float(int.from_bytes(x_val1, byteorder='little', signed=True)) + float(int.from_bytes(x_val2, byteorder='little', signed=True)) / 1000000
    y = float(int.from_bytes(y_val1, byteorder='little', signed=True)) + float(int.from_bytes(y_val2, byteorder='little', signed=True)) / 1000000
    z = float(int.from_bytes(z_val1, byteorder='little', signed=True)) + float(int.from_bytes(z_val2, byteorder='little', signed=True)) / 1000000

    if euid in stats_dic:
        if stats_dic[euid][0] != session_rand:
            stats_dic[euid] = [session_rand, ctr, 1, 0, 0, len(payload), 0, time.time(), x, y, z]
        else:
            stats_dic[euid][3] += ctr - stats_dic[euid][1] - 1
            stats_dic[euid][1] = ctr
            stats_dic[euid][2] += 1
            stats_dic[euid][4] = 100*stats_dic[euid][3]/(stats_dic[euid][3] + stats_dic[euid][2])
            stats_dic[euid][5] += len(payload)
            stats_dic[euid][6] = stats_dic[euid][5]/(time.time() - stats_dic[euid][7])
            stats_dic[euid][8] = x
            stats_dic[euid][9] = y
            stats_dic[euid][10] = z
    else:
        stats_dic[euid] = [session_rand, ctr, 1, 0, 0, len(payload), 0, time.time(), x, y, z]

    # Specify the Column Names while initializing the Table
    myTable = PrettyTable(table_header)

    # Add rows
    for key in stats_dic:
        myTable.add_row([key, stats_dic[key][8], stats_dic[key][9], stats_dic[key][10], stats_dic[key][2], stats_dic[key][3], round(stats_dic[key][4], 2), stats_dic[key][5], round(stats_dic[key][6], 1)])

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
