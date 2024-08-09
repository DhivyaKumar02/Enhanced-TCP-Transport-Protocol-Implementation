#include "../include/simulator.h"
#include<stdio.h>
#include<string.h>
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
#define WAITTIME 30.0
#define WINDOW getwinsize()
// returns the current simulation time
#define CURR_TIME get_sim_time()

struct pkt sent_pkts[2000];
struct pkt rcv_pkts[2000];

int A_in_msg_num = 0, A_out_pkt_num = 0;
int B_ack_pkt_num = 0;

int next_seq_num, wait_for_ack, base; 
int expected_rcv_seq;

struct msg msgBuffer[2000]; // Message Queue (First In First Out)
int buf_start_ptr = 0, buf_end_ptr = 0; // pointers to get the position of the first and last element in the msgBuffer

// array to mark the ack of particular packet sequence is delivered to upper layer from B
int to_be_delivered_to_upper_layer[2000];

// array to mark the ack of particular packet sequence is received from B
int ack_received[2000];



// array to store timer for each packet sequence
float individual_timers[2000];
float next_wait_time;

void memorySetting(void *addrOfData, size_t size) {
    memset(addrOfData, 0, size);
}

void sendPacket() {
	tolayer3(0, sent_pkts[next_seq_num]);
    if (base == next_seq_num)
		starttimer(0, WAITTIME);
    individual_timers[next_seq_num] = CURR_TIME + WAITTIME;
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
    int idx;
    if (checkIfCorrupted(packet) == 0) {  
        printf("Ack received and no corruptions detected.\n");
        // check if it inside the sender's WINDOW
        // If an ACK is received, the SR sender marks that packet as having been received, provided it is in the window.
        if (packet.acknum >= base || packet.acknum < base + WINDOW) {
            individual_timers[packet.acknum] = 0;
            ack_received[packet.acknum] = 1;
            // If the packet’s sequence number is equal to send_base, the window base is moved forward to the unacknowledged packet with the smallest sequence number.
            if (packet.acknum == base) {
                for (idx = base; idx < next_seq_num; idx++) {
                    if (ack_received[idx] != 1) {
                        break;
                    }
                    base++;
                }
            }
            while (next_seq_num < base + WINDOW && buf_start_ptr < buf_end_ptr) {
                struct msg message = msgBuffer[buf_start_ptr];
                sent_pkts[next_seq_num] = make_pkt(next_seq_num, message.data, 0);
                sendPacket();
                buf_start_ptr++;
                next_seq_num++;
                A_out_pkt_num++;
                printf("** Entity: A --> Sending outcoming message number %d **\n", A_out_pkt_num);   
            }
        }
    }
    if (base == next_seq_num) {
            stoptimer(0);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    int idx;
    for (idx= base; idx < next_seq_num; idx++)
	{
		if (individual_timers[idx] == CURR_TIME)
		{
			if(ack_received[idx] == 0)
			{
				tolayer3(0, sent_pkts[idx]);
				individual_timers[idx] = CURR_TIME+WAITTIME;		
				break;
			}
		}
	}
	next_wait_time  = CURR_TIME+WAITTIME;
	for (idx= base; idx < next_seq_num; idx++)
	{
		if((individual_timers[idx] == 0) && (individual_timers[idx] < next_wait_time))
			next_wait_time = individual_timers[idx];
	}
	if (next_wait_time > 0)
		starttimer(0, next_wait_time - CURR_TIME);
	return;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    base = 0;
    next_seq_num = 0;
    wait_for_ack = 0; 
    base = 0;
    memorySetting(&msgBuffer, sizeof(msgBuffer));
	memorySetting(&sent_pkts, sizeof(sent_pkts));
	memorySetting(&rcv_pkts, sizeof(rcv_pkts));
	memorySetting(&individual_timers, sizeof(individual_timers));
	memorySetting(&ack_received, sizeof(ack_received));
	return;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    if (checkIfCorrupted(packet) == 0) {
        // ACK if received packet's seq number in either of the ranges [rcv_base, rcv_base+N-1] or [rcv_base-N, rcv_base-1]
        // Otherwise ignore the product
        if (packet.seqnum < expected_rcv_seq + WINDOW && packet.seqnum >= expected_rcv_seq - WINDOW ) {
            // create ACK
            struct pkt ack_pkt = make_pkt(0, "", packet.seqnum);
            // Send ACK
            printf("Sending ACK %d\n", ack_pkt.acknum);
            tolayer3(1, ack_pkt);

            if (packet.seqnum == expected_rcv_seq) {
                tolayer5(1, packet.payload);
                expected_rcv_seq++;            

                for(int idx=expected_rcv_seq; idx < expected_rcv_seq+WINDOW; idx++) {
                    if (to_be_delivered_to_upper_layer[idx] == 0) {
                        break;
                    }
                    tolayer5(1, rcv_pkts[idx].payload);
                }
            } else {
                rcv_pkts[packet.seqnum] = packet;
                to_be_delivered_to_upper_layer[packet.seqnum] = 1;

            }

        }
    }

}


/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    expected_rcv_seq = 0;
    memorySetting(&rcv_pkts, sizeof(rcv_pkts));
    memorySetting(&to_be_delivered_to_upper_layer, sizeof(to_be_delivered_to_upper_layer));
    return;
}