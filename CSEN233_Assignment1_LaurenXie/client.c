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
    int sock, n, retry_counter, ack_timer;
    unsigned int length;
    struct sockaddr_in server;
    struct sockaddr_in from;
    struct hostent *hp;
    fd_set rset;
    struct timeval timer;
    
    //printf("variables create\n");
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

    //printf("Creating port\n");
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));
    length = sizeof(struct sockaddr_in);

    // Good Packets
    struct Packet pac1 = {0XFFFF, 1, 0XFFF1, 1, 4, 1, 0XFFFF};
    struct Packet pac2 = {0XFFFF, 1, 0XFFF1, 2, 4, 2, 0XFFFF};
    struct Packet pac3 = {0XFFFF, 1, 0XFFF1, 3, 4, 3, 0XFFFF};
    struct Packet pac4 = {0XFFFF, 1, 0XFFF1, 4, 4, 4, 0XFFFF};
    struct Packet pac5 = {0XFFFF, 1, 0XFFF1, 5, 4, 5, 0XFFFF};
    // Bad Packets
    struct Packet pac6 = {0XFFFF, 1, 0XFFF1, 6, 4, 1, 0XFFFF};
    struct Packet pac7 = {0XFFFF, 1, 0XFFF1, 7, 4, 2, 0XFFFF};
    struct Packet pac8 = {0XFFFF, 1, 0XFFF1, 8, 4, 3, 0XFFFF};
    // No end identifier packet
    struct Packet pac9 = {0XFFFF, 1, 0XFFF1, 9, 4, 4, 0XFFF0};
    struct Packet pac9_good = {0XFFFF, 1, 0XFFF1, 9, 4, 4, 0XFFFF};
    // Length mismatch packet 
    struct Packet pac10 = {0XFFFF, 1, 0XFFF1, 10, 1, 5, 0XFFFF};
    struct Packet pac10_good = {0XFFFF, 1, 0XFFF1, 10, 4, 5, 0XFFFF};
    struct Packet pac11 = {0XFFFF, 1, 0XFFF1, 11, 4, 11, 0XFFFF};
    struct Packet pac12 = {0XFFFF, 1, 0XFFFA, 12, 4, 12, 0XFFFF};

    //printf("Creating array\n");
    // Array of packets to send to server
    struct Packet array[12] = {pac1, pac2, pac3, pac4, pac5, pac7, pac8, pac9, pac10, pac10_good, pac11, pac12};

    timer.tv_sec = 3;
    timer.tv_usec = 0;
    FD_ZERO(&rset);
    FD_SET(sock, &rset);
    retry_counter = 0;

    // Sending good packets
    for(int i = 0; i < (sizeof(array)/sizeof(array[0])); i++)
    {
        // Timeout error. After 3 timeouts, terminate the program
        if(retry_counter == 3)
        {
            printf("Server does not respond\n");
            exit(0);
        }
        printf("sending packet %d now\n", array[i].seg_num);

        // Send packets in array to server
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
            // Timeout instance. After timer of 3 seconds passes and no response has come, resend the packet (i--)
            retry_counter++;
            i--;
        }
        else if (ack_timer > 0 && FD_ISSET(sock, &rset))
        {
            // Reset the retry counter since a response has come from server
            retry_counter = 0;
            struct Response *temp = malloc(sizeof(struct Response));
            n = recvfrom(sock, temp, sizeof(*temp), 0, (struct sockaddr *)&from, &length);

        // Check the response message if it is an ACK or REJECT
        if(temp->response_type == 65522)
        {
            // ACK
            printf("Received an ack for packet %d\n", temp->recv_seg_num);
        }
        else if (temp->rej_sub_code == 65524)
        {
            // Out of order reject
            printf("REJECT: PACKET %d IS OUT OF ORDER\n", temp->recv_seg_num);
            printf("SENDING PACKET %d NOW\n", pac6.seg_num);
            // Sending packet 6 since packet 6 was missed
            n = sendto(sock, (struct Packet*)&pac6, sizeof(pac6), 0, (struct sockaddr *)&server, length);
            // Receiving an ACK from server for packet 6
            struct Response *resp_temp = malloc(sizeof(struct Response));
            n = recvfrom(sock, resp_temp, sizeof(*resp_temp), 0, (struct sockaddr *)&from, &length);

            // Sending packet 7 again since server didn't accept packet 7 the first time (out of order)
            if(resp_temp->response_type == 65522)
            {
                printf("Received an ack for packet %d\n", resp_temp->recv_seg_num);
                i--;
                printf("SENDING PACKET %d AGAIN\n", array[i].seg_num);
            }
        }
        else if (temp->rej_sub_code == 65525)
        {
            // Length mismatch reject
            printf("REJECT: PACKET %d HAS LENGTH MISMATCH\n", temp->recv_seg_num);
            printf("RESENDING FIXED PACKET %d NOW\n", pac10_good.seg_num);
            // Sending the new good packet (pac10_good) with corrected length 
            n = sendto(sock, (struct Packet*)&pac10_good, sizeof(pac10_good), 0, (struct sockaddr *)&server, length);
            struct Response *resp_temp = malloc(sizeof(struct Response));
            n = recvfrom(sock, resp_temp, sizeof(*resp_temp), 0, (struct sockaddr *)&from, &length);

            if(resp_temp->response_type == 65522)
            {
                printf("Received an ack for packet %d\n", resp_temp->recv_seg_num);
            }
        }
        else if (temp->rej_sub_code == 65526)
        {
            // End identifier reject
            printf("REJECT: PACKET %d DOES NOT HAVE END IDENTIFIER\n", temp->recv_seg_num);
            printf("RESENDING FIXED PACKET %d NOW\n", pac9_good.seg_num);
            // Sending the new good packet (pac9_good) with end identifier
            n = sendto(sock, (struct Packet*)&pac9_good, sizeof(pac9_good), 0, (struct sockaddr *)&server, length);
            struct Response *resp_temp = malloc(sizeof(struct Response));
            n = recvfrom(sock, resp_temp, sizeof(*resp_temp), 0, (struct sockaddr *)&from, &length);

            if(resp_temp->response_type == 65522)
            {
                printf("Received an ack for packet %d\n", resp_temp->recv_seg_num);
            }
        }
        else if (temp->rej_sub_code == 65527)
        {
            // Duplicate reject. If there is a duplicate, do nothing and skip that packet
            printf("REJECT: PACKET %d IS A DUPLICATE\n", temp->recv_seg_num);
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