#ifndef PRIORITY_Q_H_
#define PRIORITY_Q_H_

#include "thread_queue.h"

typedef struct queue_t queue_t;
typedef struct priority_queue_t priority_queue_t;
typedef struct q_entry_t q_entry_t;

struct iterator_t
{
    int (*has_next)(iterator_t *);
    struct thread *(*next)(iterator_t *);
    int current_priority;
    struct thread *current_thread;
    priority_queue_t *priority_queue;
};

struct queue_t
{
    struct thread *front;
    struct thread *rear;
};

struct priority_queue_t
{
    // common interface
    queue_result_pair (*enqueue)(thread_queue_t *, struct thread *); // Inputs: queue, enqueue element. Output: queue_result_pair
    struct thread *(*dequeue)(thread_queue_t *);                     // Input: queue. Output: dequeued element
    struct thread *(*getByID)(int, thread_queue_t *);                // Inputs: ID, queue. Output: removed element
    struct thread *(*remove_by_id)(int, thread_queue_t *);
    iterator_t *(*iterator)(thread_queue_t *);
    int (*contains)(int, thread_queue_t *); // Inputs: ID, queue. Output: success/failure return value
    int (*size)(thread_queue_t *);          // Input: queue. Output: size of queue
    void (*free_queue)(thread_queue_t *);   // Input: queue. Frees the queue

    // specific fields
    queue_t *queues[NUM_PRIORITIES];
    int current_priority_index;
    void (*age_processes)(priority_queue_t*);
};

priority_queue_t *create_priority_queue();

queue_t *create_queue();

int is_empty(queue_t *);

int pq_enqueue(queue_t *, struct thread *);

struct thread *pq_dequeue(queue_t *);

//void add_to_rear(queue_t *queue, struct thread *thread);
void add_to_rear(queue_t *dest_queue, int priority, queue_t *src_queue);

struct thread *create_thread_entry(int, int);

#endif // PRIORITY_Q_H_
