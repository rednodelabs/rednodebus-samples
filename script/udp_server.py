import socket

local_ip="0.0.0.0"
local_port=4242
bufferSize=1024

#Create a datagram socket
sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

#Bind
server = (local_ip, local_port)
sock.bind(server)

print("Listening on " + local_ip + ":" + str(local_port))

#Listen 

while (True):
    payload, client_address = sock.recvfrom(bufferSize)
    print("Echoing data back to " + str(client_address))
    sent = sock.sendto(payload, client_address)