/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Spring 2019
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"


// Define Locks and Condition variables here
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;   // Condition variable that threads will check.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Lock that threads will acquire to do work.

// full starts at 0 (false), empty starts at 1 (true).
pthread_cond_t full;   // Producers should check, if 1, buffer is full, don't produce.
pthread_cond_t empty;  // Consumers should check, if 1, buffer is empty, don't consume.
int count = 0;         // Counter variable keeps track of the "size" of our buffer.
int prod_ptr = 0;      // Index variable for where producers insert new Matrices in the buffer.
int cons_ptr = 0;      // Index variable for where consumers take new Matrices from in buffer.

// Producer consumer data structures

// Bounded buffer bigmatrix defined in prodcons.h
// bigmatrix = (Matrix**) malloc(sizeof(Matrix) * MAX);

/*
  Takes a pointer to a Matrix and adds that Matrix to the buffer.
  Does not handle checking of buffer space or availability, all
  case coverage must be handled by the calling worker thread.

  @param value: Pointer to the Matrix to be added to the buffer.
  @return The index of the next available spot in the buffer.
 */
int put(Matrix * value)
{
    bigmatrix[prod_ptr] = value;
    // Increment the index variable, setting it back to 0 if we've reached
    // the end of the buffer's capacity.
    prod_ptr = (prod_ptr + 1) % MAX;
    count++;
    return 0;
}

/*
  Returns a Matrix from the buffer to the caller to be consumed and used
  in some way. Does not handle checking of buffer space or availability,
  all coverage must be handled by the calling worker thread.

  @return The Matrix from the buffer at index determined by cons_ptr.
*/
Matrix * get()
{
    Matrix *temp = bigmatrix[cons_ptr];
    // Increment the index variable, setting it back to 0 if we've reached
    // the end of the buffer's capacity.
    cons_ptr = (cons_ptr + 1) % MAX;
    count--;
    return temp;
}

// Matrix PRODUCER worker thread
//TODO put in and test if this works
void *prod_worker(void *arg)
{
  counters_t *myCounters = arg;
  printf("incrementing\n");
  increment_cnt(myCounters->prod);
  #if OUTPUT
  printf("Welcome to the producer thread...\n");
  printf("testing increment %d\n", get_cnt(myCounters->prod));
  #endif
  int i;
  for (i = 0; i < LOOPS; i++) {
    pthread_mutex_lock(&mutex);
    while (count == MAX) {
      #if OUTPUT
      printf("in waiting producer\n");
      #endif
      pthread_cond_wait(&empty, &mutex);
    }
    put(GenMatrixRandom());
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
  }
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  int i;
  for (i = 0; i < LOOPS; i++) {
    pthread_mutex_lock(&mutex);
    while (count == 0)
      pthread_cond_wait(&full, &mutex);
    Matrix *temp = get();
    // TODO: While in the critical section, and after getting the Matrix
    //       from the buffer, perform some work here (matrix multiplication).

    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);


  }
}
