/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 *
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the
 *  second matrix row count.
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, one at a time, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for:
 *  - the total number of matrices multiplied (multtotal from consumer threads)
 *  - the total number of matrices produced (matrixtotal from producer threads)
 *  - the total number of matrices consumed (matrixtotal from consumer threads)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from producer and consumer threads)
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed.
 *
 *  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Spring 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

int main (int argc, char * argv[])
{
  // Process command line arguments
  int numw = NUMWORK;
  if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }

  // Seed the random number generator with the system time
  time_t t;
  srand((unsigned) time(&t));

  // Arrays that will hold the worker threads. numw determined by program args.
  pthread_t pr[numw];
  pthread_t co[numw];

  // Set up counters for worker threads.
  counters_t counters;
  counter_t prodInit;       // Initialize the producer counter.
  init_cnt(&prodInit);
  counters.prod = &prodInit;
  counter_t conInit;        // Initialize the consumer counter.
  init_cnt(&conInit);
  counters.cons = &conInit;

  // Create statistic variables
  int prs = 0;      // Sum of matrix elements produced.
  int cos = 0;      // Sum of matrix elements consumed.
  int prodtot = 0;  // Number of matrices produced.
  int constot = 0;  // Number of matrices consumed.
  int consmul = 0;  // Number of matrices multiplied.
  ProdConsStats proStats = {prs, 0, prodtot};
  ProdConsStats conStats = {cos, consmul, constot};

  // Allocate the buffer the worker threads will use.
  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * MAX);

  // Create and initialize the worker threads.
  int i;
  for (i = 0; i < numw; i++) {
      pthread_create(&pr[i], NULL, prod_worker, &proStats);
      pthread_create(&co[i], NULL, cons_worker, &conStats);
  }

  // Join the worker threads when they are complete.
  for (i = 0; i < numw; i++) {
      pthread_join(pr[i], NULL);
      pthread_join(co[i], NULL);
  }

  // Update statistic variables with final totals for display to user.
  prs = proStats.sumtotal;
  prodtot = proStats.matrixtotal;
  cos = conStats.sumtotal;
  constot = conStats.matrixtotal;
  consmul = conStats.multtotal;
  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n",prs,cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot,constot,consmul);

  return 0;
}
