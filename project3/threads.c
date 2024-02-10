#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "threads.h"

sem_t mailbox_sem_producers[MAX_THREADS];  /* semaphores for producers */
sem_t mailbox_sem_consumers[MAX_THREADS];  /* semaphores for consumers */
struct msg* mailbox[MAX_THREADS]; /* mailboxes */
pthread_t threads[MAX_THREADS];  /* threads */
void initMailbox(sem_t *mailbox, int thread_count, int init_count);
void destroyMailbox(sem_t *mailbox, int thread_count);

int main(int argc, char** argv) {

    int thread_count = atoi(argv[1]);
    
    if(thread_count > MAX_THREADS) {
        printf("invalid input. too many threads.");
        return -1;
    }

    initMailbox(mailbox_sem_producers, thread_count, thread_count); // each producer can generate n messages
    initMailbox(mailbox_sem_consumers, thread_count, CONSUMER_STARTING_COUNT);

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
void RecvMsg(int iRecv, struct msg* pMsg) {
    // is this removing the message from the mailbox? I am going to assume yes
    sem_wait(&mailbox_sem_consumers[iRecv]); // removing item from mailbox so we will have a new space available
    free(mailbox[iRecv]); // free this. essentially removing it
    sem_post(&mailbox_sem_producers[pMsg->iFrom]);
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
