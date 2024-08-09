#include "../include/simulator.h"
#include <stdio.h>
#include <string.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
#define WAIT_TIME 30.0

int A_in_msg_num = 0, A_out_pkt_num = 0;
int B_ack_pkt_num = 0;

struct pkt send_pkt; // declaring it global to access it in A_timerinterrupt() to transmit it to layer3 when timer goes off
int send_pkt_seq, wait_for_ack;  //  will be initialized through A_init() 
int expected_rcv_seq; // will be initialized through B_init()

struct msg msgBuffer[2000]; // Message Queue (First In First Out)
int buf_start_ptr = 0, buf_end_ptr = 0; // pointers to get the position of the first and last element in the msgBuffer


void memorySetting(void *addrOfData, size_t size) {
    memset(addrOfData, 0, size);
}

void sendPacket() {
    wait_for_ack = 1;              // changes wait_for_ack to 1 
	starttimer(0, WAIT_TIME);
	tolayer3(0, send_pkt);
	return;
}


int calcChecksum(struct pkt packet) {
    /* Checksum is calculated as the sum of the (integer) sequence and ack field values,
        added to a character-by-character sum of the payload field of the packet (i.e., treat each
        character as if it were an 8 bit integer and just add them together).
    */
    int cs = 0, counter = 0;

    cs = packet.seqnum + packet.acknum;

    while (counter < 20) {
        cs += packet.payload[counter++];
    }

    return cs;
}

int checkIfCorrupted(struct pkt packet) {
    if (packet.checksum != calcChecksum(packet)) {
        return 1;
    }
    return 0; 
}

struct pkt make_pkt(int sequenceNumber, char *payload, int acknum)
{
    struct pkt new_packet;
    memorySetting(&new_packet, sizeof(new_packet)); // all values are initialized to 0
    new_packet.seqnum = sequenceNumber;
    new_packet.acknum = acknum;
    strcpy(new_packet.payload, payload);
    new_packet.checksum = calcChecksum(new_packet);    
    return new_packet;
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{

    // if A waits for ack (wait_for_ack == 1), then add the incoming message to msgbuffer
    if (wait_for_ack == 1) {
        msgBuffer[buf_end_ptr++] = message; 
    } else {
        send_pkt = make_pkt(send_pkt_seq, message.data, 0);
        sendPacket();

        A_out_pkt_num++;
        printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);
    }
    A_in_msg_num++;
    printf("** Entity: A --> Received incoming message number %d **\n", A_in_msg_num);
    return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{

    if (checkIfCorrupted(packet) == 0 && packet.acknum == send_pkt_seq) {  
        printf("Ack received and no corruptions detected.\n");
        stoptimer(0);
        B_ack_pkt_num++;
        printf("** Entity: B --> Sent a valid ack packet of number %d **\n", B_ack_pkt_num);
        send_pkt_seq = send_pkt_seq == 0 ? 1 : 0;
        if (buf_start_ptr < buf_end_ptr) {
            struct msg message = msgBuffer[buf_start_ptr];
            send_pkt = make_pkt(send_pkt_seq, message.data, 0);
            sendPacket();

            A_out_pkt_num++;
            printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);

            buf_start_ptr++;


        } else {
            wait_for_ack = 0;
        }
    }
    return;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    printf("Time out so sending again\n");
    sendPacket();
    printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);
    return;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    printf("Initializing A\n");

    // wait_for_ack is initialized as 0
    send_pkt_seq = 0;
    wait_for_ack = 0;
    memorySetting(&send_pkt, sizeof(send_pkt));
    memorySetting(&msgBuffer, sizeof(msgBuffer));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{

    // if not corrupted, send ACK to A
    if (checkIfCorrupted(packet) == 0) {
        // create ACK
        struct pkt ack_pkt = make_pkt(0, "", packet.seqnum);
        // Send ACK
        printf("Sending ACK %d\n", ack_pkt.acknum);
        tolayer3(1, ack_pkt);

        // if the received packet has the expected sequence then transmit it to the layer 5 
        if (expected_rcv_seq == packet.seqnum) {
            tolayer5(1, packet.payload);
            expected_rcv_seq = expected_rcv_seq == 0 ? 1 : 0;
        }
    } else {
        printf("Corrupted packet! Hence not sending ack\n");
    }
    
    return;
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    // Expected packet sequence number to be received is 0 at first. Hence Initializing
    expected_rcv_seq = 0;
}