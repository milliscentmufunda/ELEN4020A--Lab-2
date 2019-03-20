#define _OPEN_THREADS

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <omp.h>
#include <math.h>
#include <time.h>

#define No  1024
#define N1  1024
// Struct created to hold data related to the matric. Therfore, able to be passed into matrix
struct user{
    int no;
    int n1;
    int a[No][N1];
    int temp[No][N1];
    int index;
};

// smaller memonory structure to help for block transposition
struct block{

        int a, b, c, d;
        int index1, index2;

};

// bigger structure to contain block transposition
struct pthreadBlockArguments{
    int index1;
    int index2;
    int index3;
    struct block tempBlock[N1/2][No/2];

};

// fills a temporary array
void createTempArray(int row, int col, int array[row][col]){

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){

            array[i][j] = -1;
        }
    }
}

// updates original array
void updateOriginalArray(int col, int row, int temp[row][col],int array[row][col]){

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            if(temp[i][j] != -1) array[i][j] = temp[i][j];
        }
    }

}

// prints array to thee screen
void show(int *p,int row,int col){

    int arry[row][col];


    printf("\n\n\n");

    for(int i = 0; i < row; i++){

        for(int j = 0; j < col; j++){

            printf("\t %d" , *(p+i*col+j));

        }
        printf("\n");
    }
}

// pthreading function takes a void pointer
void * diagonalThreading(void *arg){

    struct user *sptr = (struct user *)arg;

    int temp[sptr->n1][sptr->no];

    createTempArray(sptr->no,sptr->n1,temp);

    for(int x = sptr->index; x < sptr->n1; x++){

        temp[x][sptr->index] = sptr->a[sptr->index][x];
        temp[sptr->index][x] = sptr->a[x][sptr->index];
    }

    updateOriginalArray(sptr->n1, sptr->no, temp, sptr->a);

    return NULL;

}

// higher level pthreading function  creates a thread for each diagonal run
void diagonalPThreading(void * array){


    struct user *sptr = (struct user *)array;

    pthread_t newthread[sptr->n1];

    for(int i = 0; i < sptr->n1; i++){

        for(int j = 0; j < sptr->n1; j++){

            if(i == j){

                sptr->index = j;

                pthread_create(&newthread[i], NULL, diagonalThreading, sptr);

                pthread_join(newthread[i], NULL);

            }
        }
    }
}


// swaps and transposes the smaller blocks. This is the pthreading fuctiion that runs in paraalel for each block
void * swapBlocks(void * arg){

    struct block blocks;
    struct pthreadBlockArguments *sptr = (struct pthreadBlockArguments *)arg;
    int i = sptr->index2;
    int k = sptr->index1;

    int temp = sptr->tempBlock[i][k].b;
    sptr->tempBlock[i][k].b = sptr->tempBlock[i][k].c;
    sptr->tempBlock[i][k].c = temp;

    blocks = sptr->tempBlock[i][k];
    sptr->tempBlock[i][k] = sptr->tempBlock[k][i];
    sptr->tempBlock[k][i] = blocks;

    if(i != k){
        temp = sptr->tempBlock[i][k].b;
        sptr->tempBlock[i][k].b = sptr->tempBlock[i][k].c;
        sptr->tempBlock[i][k].c = temp;
    }

}


// creates and joins the treading fucntion for each blcok
void blockTranspose(int n1, int no, void * arg){

    struct pthreadBlockArguments *sptr = (struct pthreadBlockArguments *)arg;

    for(int i = 0; i < no; i++){

        for(int j = 0; j < n1; j++){
            if(i == j){

                pthread_t newthread[n1];

                for(int k = j; k < no; k++){

                    sptr->index1 = k;
                    sptr->index2 = i;

                    pthread_create(&newthread[i], NULL, swapBlocks, sptr);

                    pthread_join(newthread[i], NULL);

                }
            }
        }
    }
}

//
void blockAlgorithmPThreading(void * array){

     struct user *sptr = (struct user *)array;

     struct pthreadBlockArguments blockMethod;

     struct block tempBlock[sptr->no/2][sptr->n1/2];

     int k = 0;
     int n = 0;

     for(int i = 0; i < sptr->n1; i++){

        for(int j = 0; j < sptr->no; j++){
            if(i%2 == 0 && j%2 == 0){

                struct block temp;

                temp.a = sptr->a[i][j];
                temp.b = sptr->a[i][j+1];
                temp.c = sptr->a[i+1][j];
                temp.d = sptr->a[i+1][j+1];

                if(k < sptr->n1/2){
                    blockMethod.tempBlock[n][k] = temp;
                    k++;

                }

                else{

                    k = 1;
                    n++;
                    blockMethod.tempBlock[n][0] = temp;

                }
            }
        }
     }

    blockTranspose(sptr->no/2, sptr->n1/2, &blockMethod);

     k = 0;
     n = 0;

     for(int i = 0; i < sptr->n1/2; i++){
        n = 0;
        for(int j = 0; j < sptr->no/2; j++){

            sptr->a[k][n++] = blockMethod.tempBlock[i][j].a;
            sptr->a[k][n++] = blockMethod.tempBlock[i][j].b;

        }
        k++;
        n = 0;

        for(int j = 0; j < sptr->no/2; j++){

            sptr->a[k][n++] = blockMethod.tempBlock[i][j].c;
            sptr->a[k][n++] = blockMethod.tempBlock[i][j].d;

        }
        k++;

     }

}

void randomGenerator(int no, int n1, int array[no][n1]){

    for(int i = 0; i < n1; i++){

        for(int j = 0; j < no; j++){

            array[i][j] = rand() % 2001;
        }

    }

}

int main()
{

    // creates an array structure and contains data for the array
    struct user arrayData;

    // initializing the size of the array
    arrayData.n1 = N1;
    arrayData.no = No;

    srand(time(NULL));

    // populate the array randomly
    randomGenerator(arrayData.no, arrayData.n1, arrayData.a);

    //show(&arrayData.a, N1, No);
    clock_t timeDiagonalPtreadBegin = clock();
    // runs the diagonal algorithm for pthreads
    diagonalPThreading(&arrayData);

    clock_t timeDiagonalPtreadEnd = clock();
    float time_spent_PthreadinDiagonal = (float)(timeDiagonalPtreadEnd - timeDiagonalPtreadBegin) / CLOCKS_PER_SEC;

    //show(&arrayData.a, N1, No);

    clock_t timeBlockPtreadBegin = clock();
    // runs block algorihm for pthreads
    blockAlgorithmPThreading(&arrayData);

    clock_t timeBlockPtreadEnd = clock();
    float time_spent_PthreadinBlock = (float)(timeBlockPtreadEnd - timeBlockPtreadBegin) / CLOCKS_PER_SEC;

    //show(&arrayData.a, N1, No);
    //printf("%f ", time_spent_PthreadinDiagonal);
    //printf("%f", time_spent_PthreadinBlock);




    return 0;

}

