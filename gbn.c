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

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define WAIT_TIME 30.0
// the window size value passed as parameter to -w
#define WINDOW getwinsize()

int A_in_msg_num = 0, A_out_pkt_num = 0;
int B_ack_pkt_num = 0;

struct pkt sent_pkts[2000]; // declaring it global to access it in A_timerinterrupt() to transmit them to layer3 when timer goes off
int next_seq_num, base;  //  will be initialized through A_init() 
int expected_rcv_seq; // will be initialized through B_init()

struct msg msgBuffer[2000]; // Message Queue (First In First Out)
int buf_start_ptr = 0, buf_end_ptr = 0; // pointers to get the position of the first and last element in the msgBuffer


void memorySetting(void *addrOfData, size_t size) {
    memset(addrOfData, 0, size);
}

void sendPacket() {
	tolayer3(0, sent_pkts[next_seq_num]);
    if (base == next_seq_num)
		starttimer(0, WAIT_TIME);
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
    printf("A_output %s\n", message.data);
    if ( next_seq_num < base + WINDOW ) {
        sent_pkts[next_seq_num] = make_pkt(next_seq_num, message.data, 0);
        sendPacket();
        next_seq_num++;
        A_out_pkt_num++;
        printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);
    } else {
        msgBuffer[buf_end_ptr++] = message; 
    }
        A_in_msg_num++;
    printf("** Entity: A --> Received incoming message number %d **\n", A_in_msg_num);
    return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    if (checkIfCorrupted(packet) == 0) {  
        printf("Ack received and no corruptions detected.\n");

        B_ack_pkt_num++;
        printf("** Entity: B --> Sent a valid ack packet of number %d **\n", B_ack_pkt_num);

        base = packet.acknum + 1;
        printf("BASE: %d", base);
        while (next_seq_num < base + WINDOW && buf_start_ptr < buf_end_ptr) {
            struct msg message = msgBuffer[buf_start_ptr];
            sent_pkts[next_seq_num] = make_pkt(next_seq_num, message.data, 0);
            sendPacket();
            buf_start_ptr++;
            next_seq_num++;
            A_out_pkt_num++;
            printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);   
        }
        if (base == next_seq_num) {
            stoptimer(0);
        } else {
            starttimer(0, WAIT_TIME);
        }
    }
    return;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    starttimer(0, WAIT_TIME);
    for (int i= base; i < next_seq_num; i++)
        tolayer3(0, sent_pkts[i]);
    return;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    base = 0;
    next_seq_num = 0;
    memorySetting(&msgBuffer, sizeof(msgBuffer));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    printf("Inside B_input %d %d", packet.seqnum, expected_rcv_seq);
    // if not corrupted, send ACK to A
    if (checkIfCorrupted(packet) == 0 && packet.seqnum == expected_rcv_seq) {
        // create ACK
        struct pkt ack_pkt = make_pkt(0, "", packet.seqnum);
        // Send ACK
        printf("Sending ACK %d\n", ack_pkt.acknum);
        tolayer3(1, ack_pkt);
        tolayer5(1, packet.payload);
        expected_rcv_seq++;

    }
    return;
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    expected_rcv_seq = 0;
}