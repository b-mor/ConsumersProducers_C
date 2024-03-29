/*
 *  Signal example
 *  Based on Operating Systems: Three Easy Pieces by R. Arpaci-Dusseau and A. Arpaci-Dusseau
 *
 *  This example uses locks and a condition to synchronize a child thread and
 *  the parent.  The child generates matricies and the parent computes the
 *  average when the matrix is ready for processing.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Spring 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

//#define NUMTHREADS 8
#define OUTPUT 0
#define MATRICIES 100000

int ** bigmatrix;
int r = 5;
int c = 5;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

int ** AllocMatrix(int r, int c)
{
  int ** a;
  int i;
  a = (int**) malloc(sizeof(int *) * r);
  assert(a != 0);
  for (i = 0; i < r; i++)
  {
    a[i] = (int *) malloc(c * sizeof(int));
    assert(a[i] != 0);
  }
  return a;
}

void FreeMatrix(int ** a, int r, int c)
{
  int i;
  for (i=0; i<r; i++)
  {
    free(a[i]);
  }
  free(a);
}

void GenMatrix(int ** matrix, const int height, const int width)
{
  int i, j;
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      int * mm = matrix[i];
      //matrix[i * width + j] = rand() % 10;
      //mm[j] = rand() % 1000;
      mm[j] = 1;
#if OUTPUT
      printf("matrix[%d][%d]=%d \n",i,j,mm[j]);
#endif
    }
  }
}

int AvgElement(int ** matrix, const int height, const int width)
{
  int x=0;
  int y=0;
  int ele=0;
  int i, j;
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
    {
      int *mm = matrix[i];
      y=mm[j];
      x=x+y;
      ele++;
#if OUTPUT
      printf("[%d][%d]--%d x=%d ele=%d\n",i,j,mm[j],x,ele);
#endif
    }
#if OUTPUT
  printf("AVG ELEMENT matrix-total-sum=%d element-count=%d avg-elem=%d\n",x, ele, x / ele);
#endif
  //printf("AVG ELEMENT matrix-total-sum=%d element-count=%d avg-elem=%d\n",x, ele, x / ele);
  return x / ele;
}

void *worker(void *arg)   // MATRIX PRODUCER
{
  int *loops = (int *) &arg;
#if OUTPUT
  printf("In worker...\n");
  printf("loops=%d\n",*loops);
#endif
  int i;
  int ** bm;
  for (i=0; i<*loops; i++)
  {
    bm = AllocMatrix(r,c);
    GenMatrix(bm, r, c);

    pthread_mutex_lock(&mutex);
    bigmatrix = bm;
    // signal parent
    ready=1;

    pthread_cond_signal(&cond);
    while (ready == 1)
      pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main (int argc, char * argv[])
{
  pthread_t p1;
  printf("main: begin \n");

  int loops = MATRICIES;
  int i;

  pthread_create(&p1, NULL, worker, loops);  // CREATE MATRIX PRODUCER THREAD

  // Consume matrices produced...  CONSUME THE MATRIX
  for(i=0;i<loops;i++)
  {
    pthread_mutex_lock(&mutex);
    while (ready==0)
      pthread_cond_wait(&cond, &mutex);
    int ** bm = *&bigmatrix;
    ready=0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    int avgele = AvgElement(bm, r, c);
    printf("Matrix #%d: avg element value=%d\n",i,avgele);
    FreeMatrix(bm, r, c);

    //ready=0;
    //pthread_cond_signal(&cond);
    //pthread_mutex_unlock(&mutex);
  }

  pthread_join(p1, NULL);
  return 0;
}
