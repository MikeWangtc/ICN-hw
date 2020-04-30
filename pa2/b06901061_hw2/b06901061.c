#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   
   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
                        /* and write a routine called B_output */
#define WINDOW_SIZE 8

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg
{
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    float time_sent;
    char payload[20];
};
extern int TRACE;           /* for my debugging */
extern int nsim;            /* number of messages from 5 to 4 so far */
extern int nsimmax;         /* number of msgs to generate, then stop */
extern float time;
extern float lossprob;    /* probability that a packet is dropped  */
extern float corruptprob; /* probability that one bit is packet is flipped */
extern float lambda;      /* arrival rate of messages from layer 5 */
extern int ntolayer3;     /* number sent into layer 3 */
extern int nlost;         /* number lost in media */
extern int ncorrupt;      /* number corrupted by media*/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int FIRST_SEQNUM = 1;                   // the first number of slot in A's packet buffer
int mydebug = 1;

// Information of entity A
int txBase;                     // first sequence number of window
int txLast;                     // index of the last packet in the buffer           
int pktCnt;                     // numnber of packets in pktBuffer
int nextSeqNum;                 // index of the first available slot
struct pkt *pktBuffer;          // packet buffer

int num_packet = 0;
int num_rtt_list = 500;
int rtt_index = 0;
float Retransmit_TIMEOUT = 30;
float alpha = 0.125;
float estimatedRTT = 30;
float estimatedRTT_list[500];
float sampleRTT_list[500];
float time_list[500];

FILE* file;
char* filename = "rtt.csv";
char* filename_packet_num = "pktnum.csv";

// Information of entity B
int expectSeqNum;               // expected sequence number
int lastAckNum;                 // last acknowledgement number

// Callable function definition
void stoptimer(int AorB);
void starttimer(int AorB, float increment);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[20]);

// Auxiliary function definition
void createPacket_A(struct msg message, struct pkt* outPacket);
void createPacket_B(struct pkt* inPacket, struct pkt* outPacket, int acknum);
int calculateChecksum(struct pkt packet);
int isValidSeqnum(int base, int i);
int isCorrupt(struct pkt);
void recordRTT(struct pkt* packet);
void write2CSV();
void setTimeSent(struct pkt* packet);

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){
    struct pkt new_packet;
    createPacket_A(message, &new_packet);
    
    pktBuffer[txLast] = new_packet;
    txLast += 1;
    

    if(isValidSeqnum(txBase, nextSeqNum)){
        struct pkt send_packet = pktBuffer[nextSeqNum];

        if(mydebug){
            printf("A | send packet: [%d]\n", send_packet.seqnum);
            printf("A | Current Base: [%d]\n", txBase);
        }
        setTimeSent(&send_packet);
        tolayer3(A, send_packet);
        
        if(nextSeqNum == txBase){
            if(mydebug)
                printf("A | Empty buffer, restart timer\n");
            starttimer(A, Retransmit_TIMEOUT);
        }   
        nextSeqNum += 1;
    }
    else{
        if(mydebug){
            printf("A | Window is full. Buffered [%d]\n", new_packet.seqnum);
        }
    }
}

void B_output(struct msg message)
{
    printf("  B_output: uni-directional. ignore.\n");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
    recordRTT(&packet);

    if(isCorrupt(packet)){
        if(mydebug)
            printf("A | received corrupt ACK:  [%d]\n", packet.acknum);
    }
    else{
        if(packet.acknum >= txBase){
            if(mydebug)
                printf("A | received ACK: [%d]\n", packet.acknum);

            txBase = packet.acknum + 1;

            if(mydebug) 
                printf("A | New base: [%d]\n", txBase);
            
            // Update nextSeqNum if txLast is valid
            if(isValidSeqnum(txBase, txLast)){
                nextSeqNum = txLast;
            }
            if(txBase + WINDOW_SIZE < txLast){
                nextSeqNum = txBase + WINDOW_SIZE;
            }

            // Termination: no packets need to be transmitted
            if(txBase == nextSeqNum){
                if(mydebug)
                    printf("A | Empty buffer\n");
                stoptimer(A);
            }
            else{
                stoptimer(A);
                starttimer(A, Retransmit_TIMEOUT);
            }
        }
        else{
            if(mydebug)
                printf("A | ACK is out of date: [%d]\n", packet.acknum);
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(void){
    if(mydebug)
        printf("Time interrupt: base [%d], nextseqnum [%d]\n", txBase, nextSeqNum);
    if(txBase < nextSeqNum){
        for(int i = txBase; i < nextSeqNum; ++i){
            if(mydebug)
                printf("A | send packet: [%d]\n", pktBuffer[i].seqnum);
            setTimeSent(&pktBuffer[i]);
            tolayer3(A, pktBuffer[i]);
        }
    }
    starttimer(A, Retransmit_TIMEOUT);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void){
    pktBuffer = (struct pkt *)malloc(sizeof(struct pkt)*20000);

    // states in pktBuffer
    txBase = FIRST_SEQNUM;
    txLast = FIRST_SEQNUM;
    nextSeqNum = FIRST_SEQNUM;
}


/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
    struct pkt new_packet;

    if(isCorrupt(packet) || packet.seqnum != expectSeqNum){
        if(mydebug){
            if (isCorrupt(packet))
                printf("B | received corrupt packet: [%d]\n", packet.seqnum);
            else   
                printf("B | received out of order packet: [%d]\n", packet.seqnum);
        }
        
        createPacket_B(&packet, &new_packet, lastAckNum);
        tolayer3(B, new_packet);
    }
    else if(packet.seqnum == expectSeqNum){
        if(mydebug)
            printf("B | received in order packet: [%d]\n", packet.seqnum);

        createPacket_B(&packet, &new_packet, packet.seqnum);
        tolayer3(B, new_packet);
        tolayer5(B, packet.payload);

        lastAckNum = new_packet.acknum;
        expectSeqNum += 1;
    }
    if(mydebug)
        printf("B | send ACK: [%d]\n", new_packet.acknum);
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void){
    expectSeqNum = FIRST_SEQNUM;
    lastAckNum = FIRST_SEQNUM - 1;
}


// Auxiliary function implementation
void createPacket_A(struct msg message, struct pkt* outPacket){
    outPacket -> acknum = 0;
    outPacket -> seqnum = txLast;
    memcpy(outPacket -> payload, message.data, sizeof(message.data));
    outPacket -> checksum = calculateChecksum(*outPacket);
}
void createPacket_B(struct pkt* inPacket, struct pkt* outPacket, int acknum){
    outPacket -> seqnum = 0;
    outPacket -> acknum = acknum;
    outPacket -> time_sent = inPacket -> time_sent;
    memcpy(inPacket -> payload, outPacket -> payload, sizeof(inPacket -> payload));
    outPacket -> checksum = calculateChecksum(*outPacket);
}
int calculateChecksum(struct pkt packet){
    int checksum = 0;

    checksum += packet.seqnum;
    checksum += packet.acknum;

    for(int i = 0 ; i < sizeof(packet.payload) ; ++i)
        checksum += packet.payload[i];
    
    return checksum;
}

int isValidSeqnum(int base, int i){
    return i < base + WINDOW_SIZE;
}
int isCorrupt(struct pkt packet){
    return calculateChecksum(packet) != packet.checksum;
}
void recordRTT(struct pkt* packet){
    if (rtt_index < num_rtt_list){
        float sampleRTT = time - packet -> time_sent;
        estimatedRTT = (1- alpha) * estimatedRTT + alpha * sampleRTT;
        sampleRTT_list[rtt_index] = sampleRTT;
        estimatedRTT_list[rtt_index] = estimatedRTT;
        time_list[rtt_index] = time;
        rtt_index += 1;
        Retransmit_TIMEOUT = estimatedRTT;
    }
}
void write2CSV(){
    file = fopen(filename, "w");
    fprintf(file, "time,samplertt,estimatedrtt\n");
    for(int i = 0 ; i < rtt_index ; ++i){
        fprintf(file, "%f, %f,%f\n", time_list[i], sampleRTT_list[i], estimatedRTT_list[i]);
    }
    fclose(file);
}
void setTimeSent(struct pkt* packet){
    packet -> time_sent = time;
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)
THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

int TRACE = 1;
int nsim = 0;
int nsimmax = 0;
float time = 0.000;
float lossprob; 
float corruptprob;
float lambda;
int ntolayer3;
int nlost; 
int ncorrupt;

void init(int argc, char **argv);
void generate_next_arrival(void);
void insertevent(struct event *p);

int main(int argc, char **argv)
{
    struct event *eventptr;
    struct msg msg2give;
    struct pkt pkt2give;

    int i, j;
    char c;

    init(argc, argv);
    A_init();
    B_init();

    while (1)
    {   
        ++num_packet;

        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER5)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in msg to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 20; i++)
                    msg2give.data[i] = 97 + j;
                msg2give.data[19] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 20; i++)
                        printf("%c", msg2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(msg2give);
                else
                    B_output(msg2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            pkt2give.time_sent = eventptr->pktptr->time_sent;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give);       /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
        time, nsim);
    // write2CSV();
    printf("Next Seq num: [%d], txLast: [%d]\n", nextSeqNum, txLast);
    printf("Total number of sent packets: %d\n", num_packet);
}

void init(int argc, char **argv) /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    if (argc != 6)
    {
        printf("usage: %s  num_sim  prob_loss  prob_corrupt  interval  debug_level\n", argv[0]);
        exit(1);
    }

    nsimmax = atoi(argv[1]);
    lossprob = atof(argv[2]);
    corruptprob = atof(argv[3]);
    lambda = atof(argv[4]);
    TRACE = atoi(argv[5]);
    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("the number of messages to simulate: %d\n", nsimmax);
    printf("packet loss probability: %f\n", lossprob);
    printf("packet corruption probability: %f\n", corruptprob);
    printf("average time between messages from sender's layer5: %f\n", lambda);
    printf("TRACE: %d\n", TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;          /* individual students may need to change mmm */
    x = rand() / mmm; /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
                                 /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist; /* q points to header of list in which p struct inserted */
    if (q == NULL)
    { /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)
        { /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)
        { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else
        { /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;        /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)
            { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else
            { /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to stop timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
void tolayer3(int AorB /* A or B is trying to stop timer */, struct pkt packet)
{
    struct pkt *mypktptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    mypktptr->time_sent = packet.time_sent;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
                                      /* finally, compute the arrival time of packet at the other end.
                                         medium can not reorder, so make sure packet arrives between 1 and 10
                                         time units after the latest arrival time of packets
                                         currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer5(int AorB, char datasent[20])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}
