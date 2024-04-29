#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "threads.h"
#include <time.h>
#include <unistd.h>

sem_t mailbox_sem_producers[MAX_THREADS + 1];  /* semaphores for producers. + 1 for main thread */
sem_t mailbox_sem_consumers[MAX_THREADS + 1];  /* semaphores for consumers. + 1 for main thread*/
struct msg* mailbox[MAX_THREADS + 1]; /* mailboxes */
pthread_t threads[MAX_THREADS];  /* threads */
void initMailbox(sem_t *mailbox, int thread_count, int init_count);
void destroyMailbox(sem_t *mailbox, int thread_count);

int enableNB = 0;

#define MAIN_THREAD_MAILBOX 0

int main(int argc, char** argv) {

    if ( argc > 2 && strcmp(argv[2], "nb") == 0) {
        enableNB = 1;
    }

    // creating a queue in c --> start and end of queue, initially they point to the same thing
    Queue* q = (Queue*) malloc(sizeof(Queue));
    q->front = NULL;
    q->rear = NULL;

    
    int thread_count = atoi(argv[1]);
    
    if(thread_count > MAX_THREADS) {
        printf("invalid input. too many threads.\n");
        return -1;
    }

    initMailbox(mailbox_sem_producers, thread_count, PRODUCER_STARTING_COUNT); // each producer can generate n messages
    initMailbox(mailbox_sem_consumers, thread_count, CONSUMER_STARTING_COUNT);

    // thread numbers are from 0 .. thread number
    // so need to +1 for correct mailbox
    for(long i = 1; i < thread_count + 1; i++) {
        if(pthread_create(&threads[i-1], NULL, communicate, (void *) i) != 0) {
            perror("thread creation");
            exit(1);
        }
    }

    int buffer_size = 16;
    char* buffer = (char *) malloc(buffer_size * sizeof(char));

    // while there is still a line to get, it will return the string
    // on end of file, it will return NULL or zero. stopping it
    while(fgets(buffer, buffer_size, stdin)) {

        // could simply split the message by space, but this works as well
        if (strlen(buffer) <= 2) {
            // if a line contains less than two items we assume it is the end of file
            // since we have the whitespace character, the length must be equal to two
            break;
        }
        int msg_val = atoi(strtok(buffer, " "));
        int recipient = atoi(strtok(NULL, " "));

        if(recipient < 0 || recipient > thread_count) {
            printf("%d is an invalid mailbox\n", recipient);
            continue;
        }

        if(msg_val < 0) {
            printf("%d is an invalid message value\n", msg_val);
            continue;
        }

        printf("to: %d, value: %d\n", recipient, msg_val);
        struct msg* msg = (struct msg*) malloc(sizeof(struct msg)); // create a message to send to a thread
        msg->iFrom = 0; // message coming from main thread
        msg->value = msg_val;
        
        if (enableNB) {

            if(SendMsgNB(recipient, msg) == -1) {
                // don't block, add to the queue
                enqueue(q, recipient, msg);
            }
        } else {

            SendMsg(recipient, msg);
        }

    }

    // when end of file is received, it should try to send all queued messages
    if (enableNB) {
        printf("Here is what the queue looks like right now\n");
        visualize_queue(q);
    }

    // as soon as all messages are sent we are all good to get
    while(!isEmpty(q)) {
        Node * temp = dequeue(q);
        int recipient = temp->recipient;
        struct msg* msg = temp->msg;

        // if SendMsgNB is successful we still need to free temp
        if(SendMsgNB(recipient, msg) == -1) {
            enqueue(q, recipient, msg);
        }

        free(temp); // I wonder if this also frees the msg pointer. (i do not think it does)

    }

        // send termination message to all threads and wait for them. then exit.
        // since there is nothing else to grab
        terminateMsgs(thread_count);

   // once all is complete, destroy all semaphores
    destroyMailbox(mailbox_sem_producers, thread_count); // each producer can generate n messages
    destroyMailbox(mailbox_sem_consumers, thread_count);

    return 0;
}

void enqueue(Queue* q, int recipient, struct msg* msg) {
    
    Node* newNode = (Node *) malloc(sizeof(Node));
    newNode->recipient = recipient;
    newNode->msg = msg;
    newNode->next = NULL;

    if(isEmpty(q)) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

int isEmpty(Queue* q) {
    return (q->front == NULL);
}

Node* dequeue(Queue* q) {

    if(isEmpty(q)) {
        printf("Queue is empty\n");
        return NULL;
    }

    Node * temp = q->front;
    q->front = q->front->next;

    if(q->front == NULL) {
        q->rear = NULL;
    }

    return temp;

}

void visualize_queue(Queue* q) {

    Node * temp = q->front;

    while(temp != NULL) {
        printf("%d -> ", temp->recipient);
        temp = temp->next;
    }

    printf("\n");
}

int SendMsgNB(int iTo, struct msg *pMsg) {
    // decrement producer semaphore
    // if err_num = -1, it is currently full. we return and do not block
    int err_num = sem_trywait(&mailbox_sem_producers[iTo]);
    
    if(err_num == -1) {
        return -1;
    }

    // if it is not full, then we can proceed
    mailbox[iTo] = pMsg; // placing message in mailbox
    sem_post(&mailbox_sem_consumers[iTo]); // increment the consumer counter to now consume message
    printf("placed it in mailbox: %d\n", iTo);
    printf("----------------------------\n");

    return 0; // success

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
    sem_wait(&mailbox_sem_producers[iTo]); // decrement producer counter
    mailbox[iTo] = pMsg; // placing message in mailbox
    sem_post(&mailbox_sem_consumers[iTo]); // increment the consumer counter to now consume message
    printf("placed it in mailbox: %d\n", iTo);
    printf("----------------------------\n");

}

/**
 * @brief I am assuming when you receive a message, you are essentially consuming it.
 * 
 * @param iRecv mailbox to receive from
 * @param pMsg message structure to fill in with received message
 */
struct msg* RecvMsg(int iRecv, struct msg* pMsg) {
    struct msg* received_value;
    // is this removing the message from the mailbox? I am going to assume yes
    sem_wait(&mailbox_sem_consumers[iRecv]); // removing item from mailbox so we will have a new space available
    received_value = mailbox[iRecv];
    printf("received from: %d\n", iRecv);
    sem_post(&mailbox_sem_producers[iRecv]);
    printf("----------------------------\n");


    return received_value;
}


// need + 1 because we have 0..N mailboxes (N + 1)
void initMailbox(sem_t *mailbox, int thread_count, int init_count) {
    for(int i = 0; i < thread_count + 1; i++) {
        if (sem_init(mailbox, 0, init_count) < 0) {
            perror("sem init error");
            exit(1);
        }
        mailbox++; // increase the pointer location
    }
}

void destroyMailbox(sem_t *mailbox, int thread_count) {
    for(int i = 0; i < thread_count + 1; i++) {
        if (sem_destroy(mailbox) < 0) {
            perror("sem init error");
            exit(1);
        }
        mailbox++; // increase the pointer location
    }

    printf("destroyed mailboxes\n");
}

/*
    thread function where each thread loops until termination method
*/

void adder(struct msg * tracker, struct msg * new_msg) {
    tracker->value = tracker->value + new_msg->value;
    tracker->cnt++;
}

void * communicate(void* arg) {

    long thread_number = (long) arg;
    long mailbox_number = thread_number; // OHHH, this is what is causing me my error, I was making it 2

    struct msg* new_msg;
    time_t start = time(NULL);

    struct msg * msg = (struct msg *) malloc(sizeof(struct msg));
    msg->iFrom = thread_number;
    msg->value = 0;
    msg->cnt = 0;

    while((new_msg = RecvMsg(mailbox_number, msg))) {
        // this behavior is only for threads
        // therefore, does not make sense to keep in RecvMsg (main thread can use this)
        printf("Got the message of: %d\n", new_msg->value);
        if(new_msg->value < 0) {
            free(new_msg); // gotta free it if we cant use it 
            break;
        }

        adder(msg, new_msg);
        sleep(1);
        free(new_msg); // once we have consumed/used message, we free it 
    }

    time_t end = time(NULL);
    msg->tot = end - start;

    // send msg to main thread's mailbox --> msg must be dynamically allocated
    SendMsg(MAIN_THREAD_MAILBOX, msg);

}

void terminateMsgs(int thread_count) {

    // creating a queue in c --> start and end of queue, initially they point to the same thing
    Queue* q = (Queue*) malloc(sizeof(Queue));
    q->front = NULL;
    q->rear = NULL;
   
    struct msg *new_msg = NULL;
    int mailbox_number = 0;
    int msg_count = 0;
    
    // send all thread mailbox a negative 1 message
    for(int i = 1; i < thread_count + 1; i++) {
        struct msg* termination_msg = (struct msg *) malloc(sizeof(struct msg));
        termination_msg->iFrom = 0; // coming from main thread
        termination_msg->value = -1;

         if (enableNB) {

            if(SendMsgNB(i, termination_msg) == -1) {
                // don't block, add to the queue
                enqueue(q, i, termination_msg);
            }
        } else {

            SendMsg(i, termination_msg);
        }

    }

    // send to any ones we were not able to send to
    while(!isEmpty(q)) {
        Node * temp = dequeue(q);
        int recipient = temp->recipient;
        struct msg* msg = temp->msg;

        // if SendMsgNB is successful we still need to free temp
        if(SendMsgNB(recipient, msg) == -1) {
            enqueue(q, recipient, msg);
        }

        free(temp); // I wonder if this also frees the msg pointer. (i do not think it does)

    }

    while((new_msg = RecvMsg(mailbox_number, new_msg)) != NULL) {

        msg_count++;
        
        int thread_num = new_msg->iFrom;
        int value = new_msg->value;
        int ops = new_msg->cnt;
        int tot = new_msg->tot;

        printf("The result from thread %d is %d from %d operations during %d secs.\n", thread_num, value, ops, tot);
        free(new_msg);

        if(msg_count == thread_count) {
            return;
        }
    }

    // Recv all messages --> have a counter of how many messages you have received
    // once the counter equals the number of threads, you know you have received from all
    // and we are done. 
}