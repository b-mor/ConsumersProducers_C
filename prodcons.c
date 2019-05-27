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
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;   // Producers should check, if 1, buffer is full, don't produce.
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;  // Consumers should check, if 1, buffer is empty, don't consume.
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
    #if OUTPUT
    printf("buffer size: %d\n", count);
    printf("prod_ptr: %d\n", prod_ptr);
    #endif
    bigmatrix[prod_ptr] = value;
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
  #if OUTPUT
  printf("buffer size: %d\n", count);
  #endif
    Matrix *temp = bigmatrix[cons_ptr];
    // Increment the index variable, setting it back to 0 if we've reached
    // the end of the buffer's capacity.
    cons_ptr = (cons_ptr + 1) % MAX;
    #if OUTPUT
    printf("cons ptr: %d\n", cons_ptr);
    #endif
    count--;
    return temp;
}

// Matrix PRODUCER worker thread
//TODO put in and test if this works
void *prod_worker(void *arg)
{
  counters_t *myCounters = arg;
  increment_cnt(myCounters->prod);

  #if OUTPUT
  printf("Welcome to the producer thread...\n");
  #endif

  int i;
  for (i = 0; i < LOOPS; i++) {

    #if OUTPUT
    printf("producer loop: %d\n", i);
    #endif

    pthread_mutex_lock(&mutex);
    while (count == MAX) {
      #if OUTPUT
      printf("in waiting producer\n");
      #endif
      pthread_cond_wait(&empty, &mutex);
    }
    put(GenMatrixRandom());
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
  }
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  #if OUTPUT
  printf("Welcome to the consumer thread...\n");
  #endif
  int i;
  for (i = 0; i < LOOPS; i++) {
    #if OUTPUT
    printf("consumer loop: %d\n", i);
    #endif
    pthread_mutex_lock(&mutex);
    while (count <= 0) {
      #if OUTPUT
      printf("in waiting consumer\n");
      #endif
      pthread_cond_wait(&fill, &mutex);
    }
    Matrix *m1 = get();
    Matrix *m2 = get();
    #if OUTPUT
    printf("matrix 1\n");
    DisplayMatrix(m1, stdout);
    printf("matrix 2\n");
    DisplayMatrix(m2, stdout);
    #endif
    Matrix *m3 = MatrixMultiply(m1, m2); //this is the resultant matrix
    while (m3 == NULL) { //problem probbably starts here... most likly supposed
      FreeMatrix(m2);
      while (count == 0) {
        #if OUTPUT
        printf("in waiting consumer INNER LOOP\n");
        #endif
        pthread_cond_wait(&fill, &mutex);
      }
      m2 = get();
      m3 = MatrixMultiply(m1,m2);
    }
    DisplayMatrix(m1,stdout);
    printf("    X\n");
    DisplayMatrix(m2,stdout);
    printf("    =\n");
    DisplayMatrix(m3,stdout);
    printf("\n");
    // FreeMatrix(m1);
    // FreeMatrix(m2);
    // FreeMatrix(m3);
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);


  }
}
