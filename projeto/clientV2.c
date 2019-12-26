#include <stdlib.h> // malloc e free
#include<string.h>
#include <stdio.h>

#include <unistd.h>



// int stricmp(str1, str2);

// Malloc aloca um determinado número de bytes na memória
// retornando um ponteiro para o primeiro byte alocado, ou NULL caso não tenha conseguido alocar.
// Free, por outro lado, libera o espaço alocado
/* Sizeof retorna o número de bytes de um determinado tipo (inteiro, float, estrutura, o que for - exceto vetores, strings e matrizes): */

void executar();


int main(int argc, char *argv[]){
	int total;
	printf("DEBUGGGGGGGG\n");

	printf("%s\n",argv[1]);

	if(strcmp(argv[1],"executar") == 0){
		total = argc-2; // numero total de comandos a executar

		executar(&argv[2],total);  // envio o 
	}

	return 0;
}


void executar(char **s, int total){
	// total comandos; total-1 pipes anonimos; total-1 descritores

	char *comandos; // lista de comandos a executar 
	const char token[2] = "/"; // token a dividir os comandos pelos pipes
	int flag = 0, i, j;
	int n_pipes = 0; 
	int k = 0;

	for(i=0;i<total;i++){
		printf("%s  ",s[i]);

	}

	putchar('\n');

	/* Conta o numero de pipes */
	for(i=0;i<total;i++){
		if(strcmp(s[i],token) == 0)
			n_pipes++;
	}

	printf("Numero de comandos: %d\n",n_pipes+1);


	/* alocar espaco para a matriz com os comandos */
	if(!(comandos = (char *) malloc((n_pipes+1) * 7 * sizeof(char *)))){
		perror("Alocar a matriz");
		_exit(1);
	}

	printf("%ld\n",sizeof(comandos));


	for(i=0;i<n_pipes+1;i++){
		for(j=0;j<7;j++){
			if(strcmp(s[k],token) != 0){
				*(comandos + (i*7) + j) = &s[k][0];
				k++;
			}
			else{
				while(j<7){
					*(comandos + i*j + k) = 0;
					j++;
					k++;
				}
			}

		}
		
	}

	k=0;

	for(i=0;i<n_pipes+1;i++){
		for(j=0;j<7;j++){
			printf("%s  ",  *(comandos + i*j + k));
		}
		putchar('\n');
	}

}