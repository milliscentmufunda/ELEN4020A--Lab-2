#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <string.h>



void display (int N, int A[][N]){

	for(int i = 0; i < N; i++){

            	for(int j = 0; j < N; j++){


                	printf("%d", A[i][j]);
			printf("  ");
            	}
		putchar('\n');

        }

}



void basicAlgorithm (int N, int A[][N]){
	
	for(int i = 0; i < N; i++){

			
            	for(int j = 0; j < N; j++){
					
			if(j >= i){

				// swap the corresponding elements
				int temp = A[i][j];
                		A[i][j] = A[j][i];
				A[j][i] = temp; 
			}				
            	}
        }
}



void openMPBlocked(int N, int A[][N], int numberOfThreads){

	// number of elements in each block/ sub-matrix
	int elementsInBlock = (N * N) / numberOfThreads;
	

	// code to be executed by each thread
	#pragma omp parallel num_threads(numberOfThreads) 
	{	
		// getting the thread number, and the row number and column number for each sub matrix for each thread
		int threadNumber = omp_get_thread_num();


		// getting the starting row and column for each thread
		int startingRow = 0; 

		while(threadNumber >= (sqrt(numberOfThreads) * (startingRow+1))){

			startingRow++;
		}
		
		int startingColumn = threadNumber - startingRow * sqrt(numberOfThreads);
		startingColumn = startingColumn * (N / sqrt(numberOfThreads));

		startingRow = startingRow * (N / sqrt(numberOfThreads));


		
		// declaring a sub matrix
		int t =	floor(sqrt(elementsInBlock));	
		int a[t][t];

		
		// filling the sub matrix
		int row_ = 0;

		for(int i = startingRow; i < (startingRow + sqrt(elementsInBlock)); i++){
			int column = 0;
            		for(int j = startingColumn; j < (startingColumn + sqrt(elementsInBlock)); j++){
				
                		a[row_][column] = A[i][j];
				++column; 
            		}
			++row_;
        	}
		

		// transposing the sub matrix
		int y = sqrt(elementsInBlock);
		basicAlgorithm(y, a);


		// the second transposition
		int n = floor(sqrt(numberOfThreads));
		int threads[n][n];
		int thread = 0;

		for(int i = 0; i < sqrt(numberOfThreads); i++){

            		for(int j = 0; j < sqrt(numberOfThreads); j++){

                		threads[i][j] = thread++;
            		}
        	}

		

		int newThreadNumber;

		for(int i = 0; i < sqrt(numberOfThreads); i++){

            		for(int j = 0; j < sqrt(numberOfThreads); j++){

                		if(threadNumber == threads[i][j]){
					newThreadNumber = threads[j][i];
				}	
            		}
        	}


		// getting the new starting row and column for each thread
		int newStartingRow = 0; 

		while(newThreadNumber >= (sqrt(numberOfThreads) * (newStartingRow+1))){

			newStartingRow++;
		}
		
		int newStartingColumn = newThreadNumber - newStartingRow * sqrt(numberOfThreads);
		newStartingColumn = newStartingColumn * (N / sqrt(numberOfThreads));

		newStartingRow = newStartingRow * (N / sqrt(numberOfThreads));



		// adding sub matricies back to the matrix
		row_ = 0;

		for(int i = newStartingRow; i < (newStartingRow + sqrt(elementsInBlock)); i++){

			int column = 0;
            		for(int j = newStartingColumn; j < (newStartingColumn + sqrt(elementsInBlock)); j++){

                		A[i][j] = a[row_][column];
				++column; 
            		}
			++row_;
        	}
	}
}




void NaiveThreaded(int N, int ArrA[][N])
{
    int ArrB[row][colm];
    #pragma omp parallel
    {
        for(int i = 0; i < row; i++)
        {
            for(int j = 0; j < colm; j++)
            {
                ArrB[i][j] = ArrA[j][i];
                //printf("%d ", ArrA[i][j]);
            }
            //printf("\n");
        }
    }
    memcpy(ArrA, ArrB, sizeof(ArrB));  
}



void DiagonalThreading(int N, int ArrA[][N])
{
    int ArrB[N][N];
    #pragma omp parallel
    {
        int ArrTemp1[N];
        int ArrTemp2[N];
        //#pragma omp parallel for default(none), shared(ArrA, ArrB, ArrTemp1, ArrTemp2)
        for(int i = 0; i < N; i++)
        {
            for(int j = 0; j < N; j++)
            {
                if(i == j)
                {
                    for(int x = i; x < N; x++)
                        {
                            ArrTemp1[x] = ArrA[i][x];
                            ArrTemp2[x] = ArrA[x][j];
                            //printf("%d ", ArrTemp1[x]);
                        }
                        //printf("\n");
                        for(int y = i; y < N; y++)
                        {
                            ArrB[i][y] = ArrTemp2[y];
                            ArrB[y][j] = ArrTemp1[y];
                        }
                }
            }
        }
    }
    memcpy(ArrA, ArrB, sizeof(ArrB));
}




int main()
{
    	const int NUMBEROFTHREADS = 4;

	const int N = 1024;

    	srand(time(NULL));

	int A [N][N];

	// Filling the 2D array with random numbers between 0 and 2 000 000
        for(int i = 0; i < N; i++){

            	for(int j = 0; j < N; j++){

                	A[i][j] = rand() % 2001;
				
            	}

        }

	clock_t startBasic = clock();

	basicAlgorithm(N, A);
	
	clock_t startOMPBlocked = clock();
      
	openMPBlocked(N, A, NUMBEROFTHREADS);

	clock_t startOMPNaive = clock();

    	NaiveThreaded(N, A);

	clock_t startOMPDiagonal = clock();

    	DiagonalThreading(N, A);

	clock_t endOMPDiagonal = clock();

	
	printf("Basic algorithm takes  %d\n OpenMP Blocked algorithm takes %d\n OpenMP Naive algorithm takes %d\n OpenMP Diagonal algorithm takes %d\n", (startOMPBlocked - startBasic), (startOMPNaive - startOMPBlocked), (startOMPDiagonal- startOMPNaive), ( endOMPDiagonal - startOMPDiagonal) );
    

    	return 0;
}
