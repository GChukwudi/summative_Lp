#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define QUEUE_SIZE 10
#define PRODUCER_SLEEP 2
#define CONSUMER_SLEEP 3
#define TOTAL_ITEMS 100

// define the assembly line
typedef struct {
    int queue[QUEUE_SIZE];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full, not_empty;
    int total_produced, total_consumed;
} AssemblyLine;

// initialize the assembly line
AssemblyLine assembly_line;

void initialize_assembly_line() {
    assembly_line.front = 0;
    assembly_line.rear = -1;
    assembly_line.count = 0;
    assembly_line.total_produced = 0;
    assembly_line.total_consumed = 0;
    pthread_mutex_init(&assembly_line.mutex, NULL);
    pthread_cond_init(&assembly_line.not_full, NULL);
    pthread_cond_init(&assembly_line.not_empty, NULL);
}

// put item into the assembly line
void add_item(int item) {
    assembly_line.rear = (assembly_line.rear + 1) % QUEUE_SIZE;
    assembly_line.queue[assembly_line.rear] = item;
    assembly_line.count++;
    assembly_line.total_produced++;
}

// get item from the assembly line
int remove_item() {
    int item = assembly_line.queue[assembly_line.front];
    assembly_line.front = (assembly_line.front + 1) % QUEUE_SIZE;
    assembly_line.count--;
    assembly_line.total_consumed++;
    return item;
}

// producer thread
void *producer(void *arg) {
    int item = 1;

    while (assembly_line.total_produced < TOTAL_ITEMS) {
        sleep(PRODUCER_SLEEP);

        pthread_mutex_lock(&assembly_line.mutex);
        while (assembly_line.count == QUEUE_SIZE && assembly_line.total_produced < TOTAL_ITEMS) {
            printf("Producer: Assembly line is full, waiting...\n");
            pthread_cond_wait(&assembly_line.not_full, &assembly_line.mutex);
        }

        if (assembly_line.total_produced >= TOTAL_ITEMS) {
            pthread_mutex_unlock(&assembly_line.mutex);
            break;
        }

        add_item(item);
        printf("Producer: Produced item %d added to the assembly line. Item in queue: %d\n", item, assembly_line.count);
        item++;
        
        pthread_cond_signal(&assembly_line.not_empty);
        pthread_mutex_unlock(&assembly_line.mutex);
    }

    printf("Producer: Finished producing %d items\n", assembly_line.total_produced);

    pthread_mutex_lock(&assembly_line.mutex);
    pthread_cond_signal(&assembly_line.not_empty);
    pthread_mutex_unlock(&assembly_line.mutex);

    return NULL;
}

// consumer thread
void *consumer(void *arg) {
    while (assembly_line.total_consumed < TOTAL_ITEMS) {
        pthread_mutex_lock(&assembly_line.mutex);

        while (assembly_line.count == 0) {
            if (assembly_line.total_consumed >= assembly_line.total_produced && assembly_line.total_produced >= TOTAL_ITEMS) {
                pthread_mutex_unlock(&assembly_line.mutex);
                return NULL;
            }               
            
            printf("Consumer: Assembly line is empty, waiting...\n");
            pthread_cond_wait(&assembly_line.not_empty, &assembly_line.mutex);

            if (assembly_line.count == 0 && assembly_line.total_consumed >= assembly_line.total_produced && assembly_line.total_produced >= TOTAL_ITEMS) {
                pthread_mutex_unlock(&assembly_line.mutex);
                return NULL;
            }
        }

        int item = remove_item();
        printf("Consumer: Item %d removed from the assembly line. Item in queue: %d\n", item, assembly_line.count);

        pthread_cond_signal(&assembly_line.not_full);
        pthread_mutex_unlock(&assembly_line.mutex);

        sleep(CONSUMER_SLEEP);
    }

    printf("Consumer: Finished consuming %d items\n", assembly_line.total_consumed);
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    initialize_assembly_line();

    printf("Starting assembly line simulation...\n");
    printf("Producer sleep time: %d seconds\n", PRODUCER_SLEEP);
    printf("Consumer sleep time: %d seconds\n", CONSUMER_SLEEP);
    printf("Maximum queue size: %d\n", QUEUE_SIZE);
    printf("Total items to produce/consume: %d\n", TOTAL_ITEMS);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&assembly_line.mutex);
    pthread_cond_destroy(&assembly_line.not_full);
    pthread_cond_destroy(&assembly_line.not_empty);

    printf("Assembly line simulation finished\n");
    printf("Total items produced: %d\n", assembly_line.total_produced);
    printf("Total items consumed: %d\n", assembly_line.total_consumed);

    return 0;
}
