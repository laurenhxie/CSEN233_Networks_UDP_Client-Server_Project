#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

struct Packet
{
    int start_id;
    int client_id;
    int data;
    int seg_num;
    int p_len;
    int tech;
    long sub_num;
    int end_id;
};

int main(int argc, char *argv[])
{
    int sock, n, retry_counter, ack_timer;
    unsigned int length;
    struct sockaddr_in server;
    struct sockaddr_in from;
    struct hostent *hp;
    fd_set rset;
    struct timeval timer; 
    
    // printf("variables create\n");
    // Error message to print if no port is provided - argument number when running connection is < 2
    if(argc != 3)
    {
        printf("Usage: server port\n");
        exit(1);
    }

    // Create the socket for UDP. sock should be > 0 if created successfully
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        fprintf(stderr, "Opening socket");
        exit(0);
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if(hp == 0)
    {
        fprintf(stderr, "Unknown host");
        exit(0); 
    }

    // printf("Creating port\n");
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));
    length = sizeof(struct sockaddr_in);

    // Setting up variable for select(). Timer variables for timeout
    timer.tv_sec = 3;
    timer.tv_usec = 0;
    FD_ZERO(&rset);
    FD_SET(sock, &rset);
    retry_counter = 0;

    struct Packet pac1 = {0XFFFF, 1, 0XFFF8, 1, 0, 04, 4085546805, 0XFFFF};
    // Subscriber not paid
    struct Packet pac2 = {0XFFFF, 2, 0XFFF8, 2, 0, 03, 4086668821, 0XFFFF};
    // Subscriber does not exist - wrong technology
    struct Packet pac3 = {0XFFFF, 3, 0XFFF8, 3, 0, 05, 4086808821, 0XFFFF};
    // Subscriber does not exist - number not found
    struct Packet pac4 = {0XFFFF, 4, 0XFFF8, 4, 0, 02, 4089387233, 0XFFFF};
    
    struct Packet array[4] = {pac1, pac2, pac3, pac4};
    
    // Setting packet payload length per packet
    for(int i = 0; i < (sizeof(array)/sizeof(array[0])); i++)
    {
        //printf("size of sub_num: %lu\n", sizeof(array[i].sub_num));
        array[i].p_len = sizeof(array[i].tech) + sizeof(array[i].sub_num);
    }

    // Sending packets to server
    for(int i = 0; i < (sizeof(array)/sizeof(array[0])); i++)
    {
        // Timeout error
        if(retry_counter == 3)
        {
            printf("Server does not respond\n");
            exit(0);
        }
        printf("Sending packet %d now\n", array[i].seg_num);

        n = sendto(sock, (struct Packet*)&array[i], sizeof(array[i]), 0, (struct sockaddr *)&server, length);
        if(n < 0)
        {
            printf("Sendto error\n");
            fprintf(stderr, "sendto");
            exit(0); 
        }

        ack_timer = select(sock + 1, &rset, NULL, NULL, &timer);
        if(ack_timer == 0)
        {  
            // Timeout instance. After timer passes and no response message, send the previous packet (i--) again
            retry_counter++;
            i--;
        }
        else if (ack_timer > 0 && FD_ISSET(sock, &rset))
        {
            retry_counter = 0;
            struct Packet *temp = malloc(sizeof(struct Packet));
            n = recvfrom(sock, temp, sizeof(*temp), 0, (struct sockaddr *)&from, &length);

            // Checks the packet message. If there is an error, let the client know. Also let the client know if received an ACK
            if(temp->data == 65529)
            {
                printf("Sorry, subscriber %ld has not paid yet.\n", temp->sub_num);
            }
            else if (temp->data == 65530)
            {
                printf("Sorry, subscriber %ld is not in the database.\n", temp->sub_num);
            }
            else
            {
                printf("Access granted!\n");
            }
            
            
        }
        
    }
    
}

/*void check_then_send(struct Response response, struct Packet nextPacket, struct Packet oldPack, int sock, unsigned int length, struct sockaddr_in server, struct sockaddr_in from)
{
    int n;
    struct Response *temp_resp = malloc(sizeof(struct Response));
    if (response.response_type == 65522)
    {
        printf("Received an ack for packet %d\n", response.recv_seg_num);
    }
    else if (response.response_type == 65523)
    {
        if (response.rej_sub_code == 65524)
        {
            n = sendto(sock, (struct Packet*)&oldPack, sizeof(oldPack), 0, (struct sockaddr *)&server, length);
            recvfrom(sock, temp_resp, sizeof(*temp_resp), 0, (struct sockaddr *)&from, &length);
            check_then_send(*temp_resp, )
        }
        
    }
    
    n = sendto(sock, (struct Packet*)&nextPacket, sizeof(nextPacket), 0, (struct sockaddr *)&server, length);
}*/