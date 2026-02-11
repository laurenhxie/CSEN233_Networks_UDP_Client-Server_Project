*****************************************
NAME: Lauren Xie
STUDENT ID: 00001582246
CLASS: CSEN 233
SESSION: Winter 2025 Afternoon Session
ASSIGNMENT: Assignment 2
*****************************************
This was coded and ran using MAC OS
Varification_Database.txt should be ran in the same folder
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
This assignment builds upon assignment 1 and uses UDP to transport packets to the server.
Assignment 2 also uses subscriber verification to grant access to network services.
With a database text file, the server will check if the client packets sent have access to the network.
There are 4 situations:
    1. Subscriber number and technology match and have paid. Access is ok.
    2. Subscriber has not paid yet
    3. Subscriber technology request does not match the database (Does not exist)
    4. Subscriber number does not exist in the database

In the code, there are 4 packets to be sent from the client. 
    1 packet is a good packet
    1 packet simulates a subscriber that hasn't paid
    1 packet simulates a subscriber whose technology does not match the database (Does not exist)
    1 packet simulates a subscriber whose number does not exist on the database

There is also an ACK Timer of 3 seconds and a retry counter of 3 times that will resend the packet after 3 timeout attempts
After 3 timeouts, the client will receive an error message that says "Server does not respond".