import socket
import struct
import time
import threading
from prettytable import PrettyTable

local_ip="172.17.0.1"
local_port=4242
bufferSize=2048
euid_bytes = 8
session_rand_bytes = 4
ctr_bytes = 4

table_header = ["UID", "X (m/s²)", "Y (m/s²)", "Z (m/s²)", "Packets received", "Packets lost", "Error rate (%)", "Bytes received", "Bitrate (Bps)"]
stats_dic = {}

def print_stats_table():

    while 1:
        # Specify the Column Names while initializing the Table
        myTable = PrettyTable(table_header)

        # Add rows
        for key in stats_dic:
            myTable.add_row([key, stats_dic[key][8], stats_dic[key][9], stats_dic[key][10], stats_dic[key][2], stats_dic[key][3], round(stats_dic[key][4], 2), stats_dic[key][5], round(stats_dic[key][6], 1)])

        print(myTable)

        time.sleep(1)

# creating thread to draw the stats table
table_draw = threading.Thread(target=print_stats_table)
table_draw.start()

#Create a datagram socket
sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

#Bind
server = (local_ip, local_port)
sock.bind(server)

#Listen
while (True):
    payload, client_address = sock.recvfrom(bufferSize)
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
