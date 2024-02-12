#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "threads.h"
#include <time.h>

sem_t mailbox_sem_producers[MAX_THREADS + 1];  /* semaphores for producers. + 1 for main thread */
sem_t mailbox_sem_consumers[MAX_THREADS + 1];  /* semaphores for consumers. + 1 for main thread*/
struct msg* mailbox[MAX_THREADS + 1]; /* mailboxes */
pthread_t threads[MAX_THREADS];  /* threads */
void initMailbox(sem_t *mailbox, int thread_count, int init_count);
void destroyMailbox(sem_t *mailbox, int thread_count);

#define MAIN_THREAD_MAILBOX 0

int main(int argc, char** argv) {

    int thread_count = atoi(argv[1]);
    
    if(thread_count > MAX_THREADS) {
        printf("invalid input. too many threads.");
        return -1;
    }

    initMailbox(mailbox_sem_producers, thread_count, thread_count); // each producer can generate n messages
    initMailbox(mailbox_sem_consumers, thread_count, CONSUMER_STARTING_COUNT);

    // thread numbers are from 0 .. thread number
    // so need to +1 for correct mailbox
    for(int i = 0; i < thread_count; i++) {
        if(pthread_create(threads[i], NULL, communicate, (void *) i) != 0) {
            perror("thread creation");
            exit(1);
        }
    }

    int buffer_size = 100;
    char buffer[buffer_size];
    char* val;

    while((val = fgets(buffer, buffer_size, stdin))) {

        if(val == EOF) {
            // send termination message to all threads and wait for them. then exit.
        }

        // could simply split the message by space, but this works as well
        int msg_val = atoi(val[0]);
        int recipient = atoi(val[2]);
        struct msg* msg = (struct msg*) malloc(sizeof(struct msg)); // create a message to send to a thread
        msg->iFrom = 0; // message coming from main thread
        msg->value = msg_val;
        SendMsg(recipient, msg);
    }

    /*
        Now here is where I add, my threads calling these methods. Not sure how to test this further.
        Will ask during office hours for OS on Monday.
    */

   // once all is complete, destroy all semaphores
    destroyMailbox(mailbox_sem_producers, thread_count); // each producer can generate n messages
    destroyMailbox(mailbox_sem_consumers, thread_count);
}

/**
 * @brief 
 * 
 * @param iTo mailbox to send to
 * @param pMsg message to be sent --> dynamically allocated I believe
 */
void SendMsg(int iTo, struct msg *pMsg) {
    // if we are sending a message we should decrease the producer semaphore
    // and increment the consumer mailbox

    sem_wait(&mailbox_sem_producers[pMsg->iFrom]); // decrement producer counter
    mailbox[iTo] = pMsg; // placing message in mailbox
    sem_post(&mailbox_sem_consumers[iTo]); // increment the consumer counter to now consume message

}

/**
 * @brief I am assuming when you receive a message, you are essentially consuming it.
 * 
 * @param iRecv mailbox to receive from
 * @param pMsg message structure to fill in with received message
 */
int RecvMsg(int iRecv, struct msg* pMsg) {
    int received_value;
    // is this removing the message from the mailbox? I am going to assume yes
    sem_wait(&mailbox_sem_consumers[iRecv]); // removing item from mailbox so we will have a new space available
    received_value = mailbox[iRecv]->value;
    free(mailbox[iRecv]); // free this. essentially removing it
    sem_post(&mailbox_sem_producers[pMsg->iFrom]);

    return received_value;
}


void initMailbox(sem_t *mailbox, int thread_count, int init_count) {
    for(int i = 0; i < thread_count; i++) {
        if (sem_init(mailbox, 0, init_count) < 0) {
            perror("sem init error");
            exit(1);
        }
        mailbox++; // increase the pointer location
    }
}

void destroyMailbox(sem_t *mailbox, int thread_count) {
    for(int i = 0; i < thread_count; i++) {
        if (sem_destroy(mailbox) < 0) {
            perror("sem init error");
            exit(1);
        }
        mailbox++; // increase the pointer location
    }
}

/*
    thread function where each thread loops until termination method
*/

void adder(struct msg * tracker, int val) {
    tracker->value = tracker->value + val;
    tracker->cnt++;
}

void * communicate(void* arg) {

    int thread_number = (int) arg;
    int mailbox_number = thread_number + 1;
    int received_value;
    time_t start = time(NULL);

    struct msg msg;
    msg.iFrom = thread_number;
    msg.value = 0;
    msg.cnt = 0;

    while((received_value = RecvMsg(mailbox_number, &msg)) > 0) {
        // this behavior is only for threads
        // therefore, does not make sense to keep in RecvMsg (main thread can use this)
        adder(&msg, received_value); 
    }

    time_t end = time(NULL);
    msg.tot = end - start;

    // send msg to main thread's mailbox --> msg must be dynamically allocated
    SendMsg(MAIN_THREAD_MAILBOX, &msg);

}