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

pthread_mutex_t countLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t countCond = PTHREAD_COND_INITIALIZER;

// full starts at 0 (false), empty starts at 1 (true).
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;   // Producers should check, if 1, buffer is full, don't produce.
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;  // Consumers should check, if 1, buffer is empty, don't consume.
int count = 0;         // Counter variable keeps track of the "size" of our buffer.
int prod_ptr = 0;      // Index variable for where producers insert new Matrices in the buffer.
int cons_ptr = 0;      // Index variable for where consumers take new Matrices from in buffer.
int totalMade = 0;
int totalConsumed = 0;

// Producer consumer data structures
counter_t prodCount;

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
    //printf("buffer size: %d\n", count);
    //printf("prod_ptr: %d\n", prod_ptr);
    #endif
    bigmatrix[prod_ptr] = value;
    prod_ptr = (prod_ptr + 1) % MAX;
    count++;
    totalMade++;
    return totalMade;
}

/*
Returns a Matrix from the buffer to the caller to be consumed and used
in some way. Does not handle checking of buffer space or availability,
all coverage must be handled by the calling worker thread.

@return The Matrix from the buffer at index determined by cons_ptr.
*/
Matrix * get()
{
    Matrix *temp = bigmatrix[cons_ptr]; // Fetch the Matrix at proper location in buffer.
    cons_ptr = (cons_ptr + 1) % MAX;    // Bound checking for consumer pointer.
    count--;
    totalConsumed++;
    return temp;                        // Return the fetched Matrix.
}

/*
  Producer thread. Generates matrices to be stored in the shared buffer for consumer threads.
*/
void *prod_worker(void *arg)
{
    ProdConsStats *prodStats = arg;
    int i = 0;
    while (i <= NUMBER_OF_MATRICES) { // Producing Matrix loop.
        if(i >= NUMBER_OF_MATRICES) {
          goto final;
        }

        Matrix *m;
        pthread_mutex_lock(&mutex);
        while (count == BOUNDED_BUFFER_SIZE) {
            pthread_cond_wait(&empty, &mutex);
        }

        if (DEFAULT_MATRIX_MODE == 0) {
          m = GenMatrixRandom();
        } else {
          m = GenMatrixBySize(DEFAULT_MATRIX_MODE, DEFAULT_MATRIX_MODE);
        }

        pthread_mutex_lock(&countLock);
        i = put(m);
        pthread_mutex_unlock(&countLock);
        prodStats->sumtotal += SumMatrix(m);
        prodStats->matrixtotal++;
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
    final:
    pthread_cond_broadcast(&fill);
    return NULL;
}

/*
  Consumer thread. Gets matrices from the shared buffer and performs matrix multiplication.
*/
void *cons_worker(void *arg)
{
    ProdConsStats *conStats = arg;
    int i;
    for (i = 0; i < NUMBER_OF_MATRICES; i += 2) {
        #if OUTPUT
        #endif
        pthread_mutex_lock(&mutex);
        while (count < 2 && totalMade <= NUMBER_OF_MATRICES) {  // Make sure we have two matrices to multiply.
            pthread_cond_wait(&fill, &mutex);
        }

        if (totalConsumed < NUMBER_OF_MATRICES) {
            Matrix *m1 = get();
            Matrix *m2 = get();
            conStats->sumtotal += SumMatrix(m1);
            conStats->sumtotal += SumMatrix(m2);
            conStats->matrixtotal += 2;

            Matrix *m3 = MatrixMultiply(m1, m2); //this is the resultant matrix

            if (m3 != NULL) {  // If successfully multiplied, show results.
                DisplayMatrix(m1,stdout);
                printf("    X\n");
                DisplayMatrix(m2,stdout);
                printf("    =\n");
                DisplayMatrix(m3,stdout);
                printf("\n");

                conStats->multtotal++;
                FreeMatrix(m1);
                FreeMatrix(m2);
                FreeMatrix(m3);

            } else {    // Handle freeing matrices if multiply doesn't work.
                printf("Matrices were incompatible.\n\n");
                FreeMatrix(m2);
            }
        }

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
