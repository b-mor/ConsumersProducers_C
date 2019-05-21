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

// Producer consumer data structures

// Bounded buffer bigmatrix defined in prodcons.h
//Matrix ** bigmatrix;

// Bounded buffer put() get()
int put(Matrix * value)
{
    __counter_t producer = __counters_t->prod;

}

Matrix * get()
{
  return NULL;
}

// Matrix PRODUCER worker thread
//TODO put in and test if this works
void *prod_worker(void *arg)
{
  int i;
  for (i = 0; i < LOOPS; i++) {
    pthread_mutex_lock(&mutex);
    while (count == MAX) {
      pthread_cond_wait(&EMPTY, &mutex);
    }
    put(i);
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
  }
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  int i;
  for (i = 0 i < LOOPS; i++) {
    pthread_mutex_lock(&mutex);
    while (count == 0)
      pthread_cond_wait(&fill, &mutex);
    Matrix tmp = get();
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    printf("%d\n", tmp);
  }
}
