# CSEN233_Networks_UDP_Client-Server_Project
A two-part UDP client-server project for CSEN 233 at Santa Clara University. 
Part 1: Client sends multiple packets to the server, some of which contain errors within the header. The server analyzes the packet headers to accept or reject packets.
If the server accepts the packet, the server sends an acknowledgement (ACK) to the client. If the server rejects the packet, the server sends the client an error message.

Part 2: Client sends multiple packets to the server. The server checks the client's access privilege against a database (verification_database.txt) to verify whether the client has permission to access the server. If the client has the proper permissions, the server sends acknowledgements (ACKs) to the client. If the client does not have the proper permissions, reject the client.
