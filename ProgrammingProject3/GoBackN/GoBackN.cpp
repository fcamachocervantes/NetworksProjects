#include "includes.h"

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************
//state variables
int base;
int nextSeqNum;
int expectedSeqNum;
double startTime;
double endTime;
double EstimatedRTT;

//window for packets and ackpkt to be sent from B
struct pkt *sndpkt [WINDOW_SIZE];
struct pkt ackpkt;

//function to estimate rtt and have it update based on sampledRtt
void calculateRTT(double SampleRTT) {
    EstimatedRTT = (1-ALPHA) * (EstimatedRTT) + (ALPHA) * (SampleRTT);
}

//checksum function
int computeChecksum(struct pkt packet) {
    int checksum = 0;
    checksum += packet.seqnum;
    checksum += packet.acknum;
    for (int i = 0; i < 20; i++) {
        checksum += packet.payload[i];
    }
    return checksum;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init() {
    base = 1;
    nextSeqNum = 1;

    //just making an educated guess on how long we should wait before timing out 
    //on the first packet that is sent over to Host B
    EstimatedRTT = 250;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init() {
    expectedSeqNum = 1;
    ackpkt.acknum = 0;
    ackpkt.seqnum = 0;
    ackpkt.checksum = computeChecksum(ackpkt);
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
bool rdt_sendA(struct msg message) {
    INFO << "RDT_SEND_A: Layer 4 on side A has received a message from the application that should be sent to side B: "
              << message << ENDL;
    
    bool accepted = true;

    //if we have space to store new packets we continue 
    if(nextSeqNum < (base + WINDOW_SIZE)) {
        //creating a new packet to send over to Host B
        struct pkt newPacket;
        newPacket.seqnum = nextSeqNum;
        newPacket.acknum = 0;
        memcpy(newPacket.payload, message.data, 20); 
        newPacket.checksum = computeChecksum(newPacket);

        //storing the packet incase we need to retransmit 
        sndpkt[nextSeqNum % WINDOW_SIZE] = &newPacket;

        //getting the time when the packet is sent to calculate RTT later
        startTime = simulation->getSimulatorClock();
        simulation->udt_send(A, *(sndpkt[nextSeqNum % WINDOW_SIZE]));

        if(base == nextSeqNum) {
            //start_timer
            simulation->start_timer(A, EstimatedRTT);
        }
        //moving on to the next packet
        nextSeqNum++;
    }
    else {
        bool accepted = false;
    }
    return (accepted);
}


// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void rdt_rcvA(struct pkt packet) {
    INFO << "RTD_RCV_A: Layer 4 on side A has received a packet from layer 3 sent over the network from side B:"
         << packet << ENDL;

    //if recieved packet isn't corrupt and the ack is greater than the base we continue
    if(computeChecksum(packet) == packet.checksum && packet.acknum >= base) {
        //getting time it took to send and recieve packet to calculate RTT
        endTime = simulation->getSimulatorClock();
        calculateRTT((endTime - startTime));

        //increasing base to newest packet that's been acked
        base = packet.acknum + 1;
        //if the base >= nextSeqNum we know that all the previous and current packets have been acked
        if(base >= nextSeqNum) {
            //stop_timer;
            simulation->stop_timer(A);
        } else {
            //start_timer;
            simulation->start_timer(A, EstimatedRTT);
        }
    }
}


// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendB(struct msg message) {
    INFO<< "RDT_SEND_B: Layer 4 on side B has received a message from the application that should be sent to side A: "
              << message << ENDL;

    bool accepted = false;

    return (accepted);
}


// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B 
// ***************************************************************************
void rdt_rcvB(struct pkt packet) {
    INFO << "RTD_RCV_B: Layer 4 on side B has received a packet from layer 3 sent over the network from side A:"
         << packet << ENDL;

    //sending ack packet if the recieved packet isn't corrupt 
    if(packet.checksum == computeChecksum(packet) && packet.seqnum == expectedSeqNum) {
        //constructing message
        struct msg newMessage;
        memcpy(newMessage.data, packet.payload, 20);

        //delivering message
        simulation->deliver_data(B, newMessage);

        //updating ack packet that will be sent to Host A
        ackpkt.acknum = expectedSeqNum;
        ackpkt.checksum = computeChecksum(ackpkt);

        //sending packet to host A
        simulation->udt_send(B, ackpkt);

        //moving on to next expected packet
        expectedSeqNum++;
    } else {
        //resending ack packet
        simulation->udt_send(B, ackpkt);
    }
}

// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timeout() {
    INFO << "A_TIMEOUT: Side A's timer has gone off. " << endTime << " " << startTime << " " << EstimatedRTT << ENDL;

    //start_timer
    simulation->start_timer(A, EstimatedRTT);

    //resending packets that haven't been acknowledeged yet
    for(int i = base; i <= nextSeqNum-1; i++) {
        simulation->udt_send(A, *(sndpkt[(i % WINDOW_SIZE)]));
    }
}

// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timeout() {
    INFO << "B_TIMEOUT: Side B's timer has gone off." << ENDL;
}
