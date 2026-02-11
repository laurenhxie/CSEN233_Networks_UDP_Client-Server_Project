*****************************************
NAME: Lauren Xie
STUDENT ID: 00001582246
CLASS: CSEN 233
SESSION: Winter 2025 Afternoon Session
ASSIGNMENT: Assignment 1
*****************************************
This was coded and ran using MAC OS
** I used localhost 8080 as my port number for testing **

HOW TO COMPILE AND RUN (Server):
1. Open a terminal window in the folder the project code is in
2. Run line 'gcc server.c -o server' in terminal
3. Then run './server <port number>'
4. After the client is done, use [CMD + C] or [CTRL + C] to terminate the server program

HOW TO COMPILE AND RUN (Client):
1. Open a terminal window in the folder the project code is in
2. Run line 'gcc client.c -o client' in terminal
3. Then run './client localhost <port number>'

ABOUT THE ASSIGNMENT:
This assignment covers UDP packet transportation between client and server. 
Client will send information packets to the server and the server wil send either an ACK or REJECT packet back to the client.
There are 4 cases for error handling
    1. Packets received out of order
    2. Packet payload length field does not match
    3. Packet does not have an end identifier
    4. Duplicate packets are sent to server

There is also an ack timer. After 3 seconds, if the client does not receive a response from the server, resend the packet.
After 3 tries and there is still no response message, terminate the program because the server does not respond.

There are 13 different packets to be sent from the client to the server
    First 5 packets are good packets. No errors expected to occur on both client and server
    After error packets will occur.
    1st error - Packet 7 will be sent before Packet 6 causing a out-of-order delivery
    Packet 8 is a good packet
    2nd error - Packet 9 has the wrong end identifier
    3rd error - Packet 10 has a payload length mismatch
    4th error - Duplicate of packet 10 is sent
    5th error - Server timeout