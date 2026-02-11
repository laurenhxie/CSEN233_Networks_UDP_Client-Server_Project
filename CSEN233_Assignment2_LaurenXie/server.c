#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

struct Data
{
    char num[11];
    int technology;
    int paid;
};

int main(int argc, char *argv[])
{
    int sock, length, n, sent;
    unsigned int fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char f_num[13];
    int f_tech;
    int f_paid;
    int i = 0;
    char ran1[20];
    char ran2[20];
    char ran3[20];
    char ran4[20];
    char buffer[11];

    // Allocate space for the response message - Ack or Reject packet
    struct Packet *temp = malloc(sizeof(struct Packet));

    // Create an array to store the database items
    struct Data array[3];

    // Open and read the database txt 
    FILE *verify = fopen("Verification_Database.txt", "r");
    if(verify == NULL)
    {
        printf("No such file to open\n");
        exit(1);
    }
    // Read the title but do not add the data to the array for item consistency. ran1-4 are random char arrays to throw away later
    int title = fscanf(verify, "%s  %s  %s  %s\n", ran1, ran2, ran3, ran4);
    // Read and store the data from the database txt
    while(fscanf(verify, "%s    %d  %d", f_num, &f_tech, &f_paid) == 3)
    {
        char buffer[11];
        unsigned char final[11];
        int j, m;

        // printf("From file: %s %d %d\n", f_num, f_tech, f_paid);
        // Loop to remove the '-' from the phone number. Loop for formatting
        for(m = j = 0; m < (sizeof(f_num)/sizeof(f_num[0])); m++)
        {
            if(f_num[m] != '-')
            {
                f_num[j++] = f_num[m];
            }
        }
        // Copy newly formatted char array to the array of database items
        strncpy(array[i].num, f_num, 11);
        array[i].technology = f_tech;
        array[i].paid = f_paid;
        i++;
    }

    /*for(int m = 0; m < 3; m++)
    {
        printf("Data array %d: %s %d %d\n", m, array[m].num, array[m].technology, array[m].paid);
    }*/

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
    int array_len = sizeof(array)/sizeof(array[0]);

    // Loop to listen for packets from client
    while(1)
    {
        n = recvfrom(sock, temp, sizeof(*temp), 0, (struct sockaddr *)&from, &fromlen);
        if(n < 0)
        {
            fprintf(stderr, "recvfrom");
        }

        // Converts subscriber number type from long to char array (string) to compare to database subscriber number
        sprintf(buffer, "%ld", temp->sub_num);

        // printf("Number int to string: %s\n", buffer);
        // printf("Packet received: %d %d %d %lu\n", temp->client_id, temp->seg_num, temp->tech, temp->sub_num);

        for(int k = 0; k < array_len; k++)
        {
            // printf("strcmp: %d\n", strcmp(buffer, array[k].num));
            // printf("k: %d | array length: %d\n", k, array_len);
            // sent variable used to determine if a response message has already been sent. If response message has not been sent, then sub_number is not in database
            sent = -1;
            if(strcmp(buffer, array[k].num) == 0)
            {
                printf("CHECK: number matches!\n");
                // printf("paid: %d | tech: %d %d\n", array[k].paid, temp->tech, array[k].technology);
                // Subscriber has not paid
                if(array[k].paid == 1 && (temp->tech == array[k].technology))
                {
                    // Access OK
                    //printf("Packet received: %d %d %d %d\n", temp->client_id, temp->seg_num, temp->tech, temp->sub_num);
                    printf("Access OK\n");
                    struct Packet ack_packet = {0XFFFF, temp->client_id, 0XFFFB, temp->seg_num, temp->p_len, temp->tech, temp->sub_num, 0XFFFF};
                    sent = sendto(sock, (struct Response*)&ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&from, fromlen);
                    break;
                }
                else if (array[k].paid == 0)
                {
                    printf("REJECT: Subscriber has not paid\n");
                    struct Packet reject_packet = {0XFFFF, temp->client_id, 0XFFF9, temp->seg_num, temp->p_len, temp->tech, temp->sub_num, 0XFFFF};
                    sent = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
                    break;
                }
                else if(array[k].technology != temp->tech)
                {
                    // Subscriber does not exist (tech does not match)
                    printf("REJECT: Subscriber does not exist. Technology does not match database.\n");
                    struct Packet reject_packet = {0XFFFF, temp->client_id, 0XFFFA, temp->seg_num, temp->p_len, temp->tech, temp->sub_num, 0XFFFF};
                    sent = sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
                    break;
                }
            }

        }
        // printf("Out of for loop\n");
        // sent variable used to determine if a response message has already been sent. If response message has not been sent, then sub_number is not in database
        if (sent < 0)
        {
            printf("REJECT: Subscriber does not exist. Number not in database.\n");
            struct Packet reject_packet = {0XFFFF, temp->client_id, 0XFFFA, temp->seg_num, temp->p_len, temp->tech, temp->sub_num, 0XFFFF};
            sendto(sock, (struct Response*)&reject_packet, sizeof(reject_packet), 0, (struct sockaddr *)&from, fromlen);
        }

    }
    // Continuous loop to receive requests from client
    /*while(1)
    {
        n = recvfrom(sock, temp, sizeof(*temp), 0, (struct sockaddr *)&from, &fromlen);
        
        //clear buffer
        if(n < 0)
        {
            fprintf(stderr, "recvfrom");
            exit(0);
        }
        printf("Packet details: %d, %d, %d, %d, %d, %d, %d\n", temp->start_id, temp->client_id, temp->data, temp->seg_num, temp->p_len, temp->payload, temp->end_id);
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
        // Simulate ack timeout?
        else if(temp->data == 0XFFF1)
        {
            packet_count++;
             write(1, "Received a datagram: ", 21);
            //write(1, temp->payload, n);
            printf("%d\n", temp->payload);

            // Create and send Ack for received packet
            struct Response ack_packet = {0XFFFF, temp->client_id, 0XFFF2, 0, temp->seg_num, 0XFFFF};
            n = sendto(sock, (struct Response*)&ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&from, fromlen);
            if(n < 0)
            {
                fprintf(stderr, "sendto");
                exit(0);
            }
        }
       
    }*/
}