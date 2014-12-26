trp-protocol
============

RUN the client with 5 arguements 

arg 1 : Client IP 

arg 2 : Interface name

arg 3 : Server IP

arg 4 : No of packets to be sent

eg : ./a.out 10.129.78.153 eth0 10.126.78.90 60

means running the client(10.129.78.153) to send 60 packets to server(10.126.78.90) with fast trp protocol implementation.

The server to be used is in trpserver directory.

A TCP like transport layer protocol, using raw socket programming.
