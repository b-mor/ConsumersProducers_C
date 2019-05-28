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
int totalMade = 0;
int totalConsumed = 0;

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
    #if OUTPUT
    //printf("buffer size: %d\n", count);
    #endif
    Matrix *temp = bigmatrix[cons_ptr];
    // Increment the index variable, setting it back to 0 if we've reached
    // the end of the buffer's capacity.
    cons_ptr = (cons_ptr + 1) % MAX;
    #if OUTPUT
    //printf("cons ptr: %d\n", cons_ptr);
    #endif
    count--;
    totalConsumed++;
    return temp;
}

// Matrix PRODUCER worker thread
//TODO put in and test if this works
void *prod_worker(void *arg)
{
    ProdConsStats *prodStats = arg;

    #if OUTPUT
    printf("Starting producer thread.\n");
    #endif

    int i, j;
    for (i = 0; i < NUMBER_OF_MATRICES; i++) { // Producing Matrix loop.

        #if OUTPUT
        printf("producer loop: %d\n", i);
        #endif

        pthread_mutex_lock(&mutex);
        while (count == BOUNDED_BUFFER_SIZE) {
            #if OUTPUT
            printf("*** BUFFER FULL, producer is waiting. ***\n\n");
            #endif
            pthread_cond_wait(&empty, &mutex);
        }
        Matrix *m = GenMatrixRandom();
        i = put(m);
        j++;    // counts how many matrices this particular thread has made.

        prodStats->sumtotal += SumMatrix(m);
        prodStats->matrixtotal++;
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }

    printf("Producer thread done.\nI made %d matrices.\n",j);
    printf("Total matrices made: %d\n", totalMade);
    pthread_cond_broadcast(&fill);

}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
    ProdConsStats *conStats = arg;
    #if OUTPUT
    printf("Starting consumer thread.\n");
    #endif
    int i;
    for (i = 0; i < NUMBER_OF_MATRICES; i += 2) {
        #if OUTPUT
        #endif
        pthread_mutex_lock(&mutex);
        while (count < 2 && totalMade != NUMBER_OF_MATRICES) {  // Make sure we have two matrices to multiply.
            #if OUTPUT
            printf("*** BUFFER EMPTY, consumer is waiting. ***\n\n");
            #endif
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
}
