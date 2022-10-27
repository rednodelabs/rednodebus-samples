import socket
import time
import threading
from prettytable import PrettyTable

local_ip="172.17.0.1"
local_port=4242
bufferSize=2048
euid_len = 12
session_rand_len = 4
ctr_len = 4

table_header = ["UID", "Packets received", "Packets lost", "Error rate (%)", "Bytes received", "Bitrate (Bps)"]
stats_dic = {}

def print_stats_table():

    while 1:
        # Specify the Column Names while initializing the Table
        myTable = PrettyTable(table_header)

        # Add rows
        for key in stats_dic:
            myTable.add_row([key, stats_dic[key][2], stats_dic[key][3], round(stats_dic[key][4], 2), stats_dic[key][5], round(stats_dic[key][6], 1)])

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



