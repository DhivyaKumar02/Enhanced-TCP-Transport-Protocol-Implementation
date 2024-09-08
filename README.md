
## Project Overview

This assignment involves implementing and analyzing three different data link layer protocols: Alternating Bit Protocol (ABT), Go-Back-N (GBN), and Selective Repeat (SR). The goal is to compare their performance in terms of throughput under various loss probabilities and window sizes.

## Protocols Implemented

### Alternating Bit Protocol (ABT)

- **Description:** A connectionless protocol that retransmits lost or corrupted messages using a First-In-First-Out method. The window size is 1, and sequence numbers alternate between 0 and 1.
- **Execution:** Implemented based on the Finite State Machine (FSM) diagram. The sender waits for an acknowledgment before sending the next packet. The receiver checks packet checksums and provides acknowledgments.
- **Timeout Scheme:** Uses a constant timeout value. Throughput decreases as loss probability increases due to repeated retransmissions of the same packet.

### Go-Back-N (GBN)

- **Description:** A sliding window protocol where the sender can send multiple packets before needing an acknowledgment, but must wait for cumulative acknowledgments.
- **Execution:** Packets are buffered if they exceed the window size. The sender retransmits all packets from the current base up to the next available sequence number if a timeout occurs.
- **Timeout Scheme:** An adaptive timer adjusts based on window size. The timer is used to resend unacknowledged data if packet corruption or loss occurs.
- **Observation:** Throughput improves with smaller window sizes under certain loss probabilities.

### Selective Repeat (SR)

- **Description:** A sliding window protocol where only erroneous packets are retransmitted, unlike GBN where entire window may be retransmitted.
- **Execution:** Each packet has an individual timer. The receiver acknowledges only the correctly received packets, and retransmission is handled per packet's timeout.
- **Timeout Scheme:** Utilizes a single hardware timer to simulate multiple logical timers. The timer is adjusted based on packet start times.
- **Observation:** Provides better throughput than ABT and GBN, especially as window sizes increase and loss probabilities are high.

## Experiments and Results

### Experiment 1: Comparison at Different Window Sizes

- **Window Size = 10:**
  - **ABT Throughput:** Decreases with higher loss probability.
  - **GBN Throughput:** Decreases linearly with higher loss probability.
  - **SR Throughput:** Performs better than ABT and GBN, particularly as loss probability increases.

- **Window Size = 50:**
  - **ABT Throughput:** Lowest throughput efficiency.
  - **GBN Throughput:** Decreases significantly with increasing loss probability.
  - **SR Throughput:** Consistently higher than ABT and GBN.

### Experiment 2: Comparison at Different Loss Probabilities

- **Loss Probability = 0.2:**
  - **ABT Throughput:** Decreases with increasing window size.
  - **GBN and SR Throughput:** Remain constant across window sizes.

- **Loss Probability = 0.5:**
  - **ABT Throughput:** Performs better than GBN with increasing window sizes.
  - **SR Throughput:** Highest among all protocols, unaffected by window size.

- **Loss Probability = 0.8:**
  - **ABT Throughput:** Lowest throughput efficiency.
  - **GBN Throughput:** Higher than ABT but lower than SR.
  - **SR Throughput:** Increases linearly with window size.

## Conclusion

- **Selective Repeat (SR)** consistently delivers the best performance across different scenarios.
- **Alternating Bit Protocol (ABT)** is efficient with lower loss probabilities.
- **Go-Back-N (GBN)** throughput decreases as both window size and loss probability increase.

