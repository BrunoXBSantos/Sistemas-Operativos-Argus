#include <stdlib.h>
#include <stdio.h>


int main(){
	int **M,j,i; 
   	M = malloc (4 * sizeof (int *));
	for ( i = 0; i < 4; ++i)
		M[i] = malloc (2 * sizeof (int));




	for(i=0;i<4;i++){
		for(j=0;j<2;j++){
			M[i][j]=i+j;
		}
	}

	for(i=0;i<4;i++){
		for(j=0;j<2;j++){
			printf("%d ",M[i][j]);
		}
		putchar('\n');
	}

}

