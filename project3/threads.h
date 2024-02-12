#ifndef THREADS_H
#define THREADS_H

#define MAX_THREADS 10
#define REQUEST 1
#define REPLY 2
#define PRODUCER_STARTING_COUNT 1
#define CONSUMER_STARTING_COUNT 0

struct msg {
    int iFrom; /* who sent the message (0 .. number-of-threads) */
    int value; /* its value */
    int cnt;   /* count of operations (not needed by all messages) */
    int tot;   /* total time (not needed by all msgs) */
};

void SendMsg(int iTo, struct msg *pMsg);
struct msg* RecvMsg(int iRecv, struct msg *pMsg);
void adder(struct msg * tracker, struct msg * new_msg);
void * communicate(void* arg);
void terminateMsgs(int thread_count);

#endif