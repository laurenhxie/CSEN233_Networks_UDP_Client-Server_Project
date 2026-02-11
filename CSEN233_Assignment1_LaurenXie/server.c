#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct Packet
{
    int start_id;
    int client_id;
    int data;
    int seg_num;
    int p_len;
    int payload;
    int end_id;
};

struct Response
{
    int start_id;
    int client_id;
    int response_type;
    int rej_sub_code;
    int recv_seg_num;
    int end_id;
};

int main(int argc, char *argv[])
{
    int sock, length, n;
    unsigned int fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    //char buf[1024];
    struct Packet *temp = malloc(sizeof(struct Packet));

    // Error message to print if no port is provided - argument number when running connection is < 2
    if(argc < 2)
    {
        fprintf(stderr, "ERROR: no port provided\n");
        exit(0);
    }

    // Create the socket for UDP. sock should be > 0 if created successfully
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        fprintf(stderr, "Opening socket");
        exit(0);
    }

    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
    if(bind(sock, (struct sockaddr *)&server, length) < 0)
    {
        fprintf(stderr, "Binding");
        exit(0);
    }
    fromlen = sizeof(struct sockaddr_in);
    int packet_count = 0;
    // Continuous loop to receive requests from client
    while(1)
    {
        n = recvfrom(sock, temp, sizeof(*temp), 0, (struct sockaddr *)&from, &fromlen);
        
        //clear buffer
        if(n < 0)
        {
            fprintf(stderr, "recvfrom");
            exit(0);
        }
        //printf("Packet details: %d, %d, %d, %d, %d, %d, %d\n", temp->start_id, temp->client_id, temp->data, temp->seg_num, temp->p_len, temp->payload, temp->end_id);
        printf("Received a datagram: %d\n", temp->seg_num);        
        
        // Error handling
        if(temp->seg_num > (packet_count+1))
        {
            // Case 1: Wrong sequence
            printf("REJECT: wrong sequence\n");
            struct Response reject_packet = {0XFFFF, temp->client_id, 0XFFF3, 0XFFF4, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
        }
        else if (temp->p_len != sizeof(temp->payload))
        {
            // Case 2: length and payload mismatch
            printf("REJECT: wrong length\n");
            struct Response reject_packet = {0XFFFF, temp->client_id, 0XFFF3, 0XFFF5, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
        }
        else if (temp->end_id != 0XFFFF)
        {
            // Case 3: no 'End of Packet Identifier'
            printf("REJECT: no end identifier\n");
            struct Response reject_packet = {0XFFFF, temp->client_id, 0XFFF3, 0XFFF6, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
        }
        else if (temp->seg_num == packet_count)
        {
            // Case 4: duplicate packet/sequence number
            printf("REJECT: dup\n");
            struct Response reject_packet = {0XFFFF, temp->client_id, 0XFFF3, 0XFFF7, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
        }
        // Simulate ack timeout. ACK timeout packet does not have data value 0XFFF1 like the good packets should
        else if(temp->data == 0XFFF1)
        {
            packet_count++;
            // Create and send Ack for received packet
            struct Response ack_packet = {0XFFFF, temp->client_id, 0XFFF2, 0, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&from, fromlen);
            if(n < 0)
            {
                fprintf(stderr, "sendto");
                exit(0);
            }
        }
       
    }
}