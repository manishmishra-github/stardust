#ifndef THREAD_QUEUE_H_
#define THREAD_QUEUE_H_

#include <os/sched.h>

#define NUM_PRIORITIES 128

typedef struct thread_queue_t thread_queue_t;
typedef struct queue_result_pair queue_result_pair;
typedef struct iterator_t iterator_t;

struct queue_result_pair
{
    void *queue;
    int result;
};

struct thread_queue_t
{
    queue_result_pair (*enqueue)(thread_queue_t *, struct thread *); // Inputs: queue, enqueue element. Output: queue_result_pair
    struct thread *(*dequeue)(thread_queue_t *);                     // Input: queue. Output: dequeued element
    struct thread *(*getByID)(int, thread_queue_t *);                // Inputs: ID, queue. Output: removed element
    struct thread *(*remove_by_id)(int, thread_queue_t *);
    iterator_t *(*iterator)(thread_queue_t *);
    int (*contains)(int, thread_queue_t *); // Inputs: ID, queue. Output: success/failure return value
    int (*size)(thread_queue_t *);          // Input: queue. Output: size of queue
    void (*free_queue)(thread_queue_t *);   // Input: queue. Frees the queue
};

#endif // THREAD_QUEUE_H_
