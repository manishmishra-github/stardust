#include <priority_queue.h>
#include <thread_queue.h>
#include <os/xmalloc.h>
#include <os/sched.h>

queue_t *create_queue()
{
    queue_t *queue = NULL;
    queue = (struct queue_t *)xmalloc(struct queue_t);

    if (NULL == queue)
    {
        return NULL;
    }

    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

int is_empty(queue_t *queue)
{
    if (NULL == queue)
    {
        return 0;
    }
    return (queue->front == NULL);
}

int pq_enqueue(queue_t *queue, struct thread *input)
{
    if (NULL == input || NULL == queue)
    {
        return 0;
    }

    if (is_empty(queue))
    {
        queue->front = input;
        queue->rear = input;
    }
    else
    {
        queue->rear->next = input;
        input->prev = queue->rear;
        queue->rear = input;
    }
    // printk("\nEnqueue struct thread Priority %d Id %d struct thread %p\n", input->priority, input->id, input);

    return 1;
}

struct thread *pq_dequeue(queue_t *queue)
{
    if (NULL == queue)
    {
        return NULL;
    }

    if (is_empty(queue))
    {
        // printk("queue is empty. Cannot dequeue.\n");
        return NULL;
    }

    struct thread *front_thread = queue->front;
    if (NULL == front_thread)
    {
        return NULL;
    }

    if (queue->front == queue->rear)
    {
        queue->front = NULL;
        queue->rear = NULL;
    }
    else
    {
        queue->front = front_thread->next;
        queue->front->prev = NULL;
    }

    // printk("\nDequeue struct thread Priority %d Id %d struct thread %p\n", front_thread->priority, front_thread->id, front_thread);
    // free(frontstruct thread);
    return front_thread;
}

queue_result_pair enqueue(thread_queue_t *data_struct, struct thread *input)
{
    queue_result_pair result;

    if (NULL == data_struct || NULL == input)
    {
        result.queue = NULL;
        result.result = 0;
        return result;
    }

    priority_queue_t *pq = (priority_queue_t *)data_struct;

    int priority = input->priority;
    // printk("PRIORITY%d ID %d \n", priority, input->id);

    if (priority < 0 || priority >= NUM_PRIORITIES)
    {
        // printk("Invalid priority. Cannot enqueue.\n");
        result.queue = NULL;
        result.result = 0;
        return result;
    }

    queue_t *queue = pq->queues[priority];
    if (NULL == queue)
    {
        result.queue = NULL;
        result.result = 0;
        return result;
    }

    if(!pq_enqueue(queue, input))
    {
        result.queue = pq;
        result.result = 0;
        return result;
    }

    if (input->priority < pq->current_priority_index)
    {
        pq->current_priority_index = input->priority;
    }
    result.queue = pq;
    result.result = 1;
    return result;
}

struct thread *dequeue(thread_queue_t *tq)
{
    if (NULL == tq)
    {
        return NULL;
    }

    priority_queue_t *pq = (priority_queue_t *)tq;

    // Only dequeue from the 0th queue
    queue_t *queue = pq->queues[0];
    if (NULL == queue || is_empty(queue))
    {
        // If the 0th queue is empty, find the next non-empty queue and promote it
        int next_non_empty_index = NUM_PRIORITIES;
        for (int i = 1; i < NUM_PRIORITIES; i++)
        {
            if (!is_empty(pq->queues[i]))
            {
                next_non_empty_index = i;
                break;
            }
        }

        if (next_non_empty_index < NUM_PRIORITIES)
        {
            // Move all threads from the next non-empty queue and all queues below it up by the difference
            for (int j = 0; j < NUM_PRIORITIES - next_non_empty_index; j++)
            {
                struct thread *thread = pq_dequeue(pq->queues[j + next_non_empty_index]);
                if (thread != NULL)
                {
                    add_to_rear(pq->queues[j], thread);
                }
                // Set the front and rear pointers of the dequeued queue to NULL
                pq->queues[j + next_non_empty_index]->front = NULL;
                pq->queues[j + next_non_empty_index]->rear = NULL;
            }
        }

        queue = pq->queues[0];
        if (NULL == queue || is_empty(queue))
        {
            // If still no threads to schedule, return NULL
            return NULL;
        }
    }

    struct thread *thread = pq_dequeue(queue);
    if (NULL == thread)
    {
        return NULL;
    }

    // Set a flag in the thread to indicate that it has been scheduled at least once
    // thread->schedule_count++;

    return thread;
}

// Get thread by ID
struct thread *getByID(int id, thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return NULL;
    }
    priority_queue_t *pq = (priority_queue_t *)tq;
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        queue_t *queue = pq->queues[i];
        if (queue != NULL)
        {
            struct thread *temp = queue->front;
            while (temp != NULL)
            {
                if (temp->id == id)
                {
                    return temp;
                }
                temp = temp->next;
            }
        }
    }
    return NULL; // return NULL if no thread with such ID found
}

// Check if queue contains a thread with a given ID
int contains(int id, thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return 0;
    }
    return getByID(id, tq) != NULL;
}

// Get the size of the priority queue
int size(thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return 0;
    }
    priority_queue_t *pq = (priority_queue_t *)tq;
    int size = 0;
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        queue_t *queue = pq->queues[i];
        if (queue != NULL)
        {
            struct thread *temp = queue->front;
            while (temp != NULL)
            {
                size++;
                temp = temp->next;
            }
        }
    }
    return size;
}

struct thread *remove_by_id(int id, thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return NULL;
    }
    priority_queue_t *pq = (priority_queue_t *)tq;
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        queue_t *queue = pq->queues[i];
        if (queue != NULL)
        {
            struct thread *temp = queue->front;
            while (temp != NULL)
            {
                if (temp->id == id)
                {
                    // If the thread to be removed is at the front of the queue
                    if (temp == queue->front)
                    {
                        queue->front = temp->next;
                        if (queue->front != NULL)
                        {
                            queue->front->prev = NULL;
                        }
                    }
                    // If the thread to be removed is at the rear of the queue
                    else if (temp == queue->rear)
                    {
                        queue->rear = temp->prev;
                        if (queue->rear != NULL)
                        {
                            queue->rear->next = NULL;
                        }
                    }
                    // If the thread to be removed is in the middle of the queue
                    else
                    {
                        temp->prev->next = temp->next;
                        temp->next->prev = temp->prev;
                    }

                    return temp;
                }
                temp = temp->next;
            }
        }
    }
    return NULL; // return NULL if no thread with such ID found
}

// Free the queue
void free_queue(thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return;
    }
    priority_queue_t *pq = (priority_queue_t *)tq;
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        queue_t *queue = pq->queues[i];
        if (queue != NULL)
        {
            struct thread *temp;
            while ((temp = pq_dequeue(queue)) != NULL)
            {
                xfree(temp);
            }
            xfree(queue);
        }
    }
    xfree(pq);
}

void add_to_rear(queue_t *queue, struct thread *thread)
{
    if (queue == NULL || thread == NULL)
    {
        return;
    }

    // If the queue is empty, the front and rear both point to the new thread
    if (is_empty(queue))
    {
        queue->front = thread;
    }
    else
    {
        // The old rear thread should now point to the new thread
        queue->rear->next = thread;
        // The new thread's previous pointer should point to the old rear thread
        thread->prev = queue->rear;
    }

    // Traverse to the end of the list of threads and update the rear pointer
    struct thread *temp = thread;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    queue->rear = temp;
}

// void demote_processes(priority_queue_t *scheduler)
// {
//     if (scheduler == NULL)
//     {
//         // printf("Scheduler is NULL.\n");
//         return;
//     }

//     queue_t *highest_priority_queue = scheduler->queues[0]; // Get the queue at the highest priority level
//     if (!is_empty(highest_priority_queue))
//     {                                                          // If the queue is not empty
//         struct thread *thread = highest_priority_queue->front; // Get the front thread in the queue
//         while (thread != NULL)
//         {
//             // Save the next thread before modifying pointers
//             struct thread *next_thread = thread->next;

//             // If the thread has run once and is not at its original priority
//             if (thread->schedule_count > 0 && thread->priority != thread->original_priority)
//             {
//                 // If the thread is at the front of the queue
//                 if (thread == highest_priority_queue->front)
//                 {
//                     highest_priority_queue->front = next_thread;
//                     if (next_thread != NULL)
//                     {
//                         next_thread->prev = NULL;
//                     }
//                 }
//                 // If the thread is at the rear of the queue
//                 else if (thread == highest_priority_queue->rear)
//                 {
//                     highest_priority_queue->rear = thread->prev;
//                     if (thread->prev != NULL)
//                     {
//                         thread->prev->next = NULL;
//                     }
//                 }
//                 // If the thread is in the middle of the queue
//                 else
//                 {
//                     thread->prev->next = next_thread;
//                     next_thread->prev = thread->prev;
//                 }

//                 // Reset the schedule count
//                 thread->schedule_count = 0;

//                 // Add the thread back to its original priority queue
//                 if (thread->original_priority >= 0 && thread->original_priority < NUM_PRIORITIES)
//                 {
//                     pq_enqueue(scheduler->queues[thread->original_priority], thread);
//                 }
//                 else
//                 {
//                     // printf("Original priority of the thread is out of range.\n");
//                 }
//             }

//             thread = next_thread; // Move to the next thread
//         }
//     }
// }

// void promote_processes(priority_queue_t *pq)
// {
//     // Iterate over the priority levels from second highest to lowest.
//     for (int i = 1; i < NUM_PRIORITIES; i++)
//     {
//         // If the queue at the current priority level is not empty...
//         if (!is_empty(pq->queues[i]))
//         {
//             // Dequeue the threads from the current queue...
//             struct thread *thread = pq_dequeue(pq->queues[i]);
//             // And add them to the rear of the queue at the next higher priority level.
//             add_to_rear(pq->queues[i - 1], thread);
//         }
//     }
// }

// void ageProcesses(priority_queue_t *scheduler)
// {
//     demote_processes(scheduler);
//     promote_processes(scheduler);
// }

// struct thread *create_thread_entry(int priority, int id)
// {
//     struct thread *newstruct thread = NULL;
//     newstruct thread = (struct struct thread *)xmalloc(struct struct thread);
//     if (newstruct thread == NULL)
//     {
//         printk("ERROR");
//         return NULL;
//     }
//     newstruct thread->id = id;
//     newstruct thread->priority = priority;
//     newstruct thread->original_priority = priority;
//     newstruct thread->schedule_count = 0;
//     newstruct thread->next = NULL;
//     newstruct thread->prev = NULL;

//     return newstruct thread;
// }

int has_next(iterator_t *iterator)
{
    // Check if there's a next thread at the current priority level
    if (iterator->current_thread != NULL)
        return 1;

    // Check subsequent priority levels
    for (int i = iterator->current_priority + 1; i < NUM_PRIORITIES; i++)
    {
        if (iterator->priority_queue->queues[i]->front != NULL)
            return 1;
    }

    return 0; // No more threads to iterate through
}

struct thread *next(iterator_t *iterator)
{
    if (iterator->current_thread == NULL)
    {
        iterator->current_priority++;
        // Set current_thread to the front of the next non-empty priority queue
        for (int i = iterator->current_priority; i < NUM_PRIORITIES; i++)
        {
            if (iterator->priority_queue->queues[i]->front != NULL)
            {
                iterator->current_priority = i;
                iterator->current_thread = iterator->priority_queue->queues[i]->front;
                break;
            }
        }
    }
    if (iterator->current_thread != NULL)
    {
        struct thread *next_thread = iterator->current_thread;
        iterator->current_thread = iterator->current_thread->next;

        // If there are no more threads at the current priority level, increment current_priority
        if (iterator->current_thread == NULL)
        {
            iterator->current_priority++;
            // Set current_thread to the front of the next non-empty priority queue
            for (int i = iterator->current_priority; i < NUM_PRIORITIES; i++)
            {
                if (iterator->priority_queue->queues[i]->front != NULL)
                {
                    iterator->current_priority = i;
                    iterator->current_thread = iterator->priority_queue->queues[i]->front;
                    break;
                }
            }
        }

        return next_thread;
    }

    return NULL; // Return NULL if there are no more threads to iterate through
}

iterator_t *new_iterator(thread_queue_t *tq)
{
    if (tq == NULL)
    {
        return NULL;
    }
    iterator_t *iterator = (iterator_t *)xmalloc(iterator_t);
    if (iterator == NULL)
    {
        return NULL; // Memory allocation failure
    }

    priority_queue_t *pq = (priority_queue_t *)tq;

    iterator->has_next = &has_next;
    iterator->next = &next;
    iterator->current_priority = 0;
    iterator->current_thread = pq->queues[0]->front;
    iterator->priority_queue = pq;
    return iterator;
}

priority_queue_t *create_priority_queue()
{
    priority_queue_t *pq = NULL;
    pq = (struct priority_queue_t *)xmalloc(struct priority_queue_t);

    if (NULL == pq)
    {
        return NULL;
    }

    pq->current_priority_index = NUM_PRIORITIES - 1;

    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        pq->queues[i] = create_queue();

        if (NULL == pq->queues[i])
        {
            return NULL;
        }
    }

    // Initialize the function pointers
    pq->enqueue = &enqueue;
    pq->dequeue = &dequeue;
    pq->getByID = &getByID;
    pq->contains = &contains;
    pq->remove_by_id = &remove_by_id;
    pq->size = &size;
    pq->free_queue = &free_queue;
    // pq->ageProcesses = &ageProcesses;
    pq->iterator = &new_iterator;
    pq->pq_enqueue = &pq_enqueue;

    return pq;
}
