#include <stdlib.h> // malloc e free
#include<string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>




// int stricmp(str1, str2);

// Malloc aloca um determinado número de bytes na memória
// retornando um ponteiro para o primeiro byte alocado, ou NULL caso não tenha conseguido alocar.
// Free, por outro lado, libera o espaço alocado
/* Sizeof retorna o número de bytes de um determinado tipo (inteiro, float, estrutura, o que for - exceto vetores, strings e matrizes): */

void executar();


int main(int argc, char *argv[]){
	int total;

	if(strcmp(argv[1],"executar") == 0){
		total = argc-2; // numero total de comandos a executar

		executar(&argv[2],total);  // envio o 
	}

	return 0;
}


void executar(char *s[], int total){
	// total comandos; total-1 pipes anonimos; total-1 descritores

	char *comandos[10][7]; // lista de comandos a executar 
	const char token[2] = "|"; // token a dividir os comandos pelos pipes
	int flag = 0, i, j;
	int n_pipes = 0; 
	int k = 0;

	int **fd;
	int *pid;
	char buffer[23];



	for(i=0;i<total;i++){
		printf("%s  ",s[i]);

	}
	putchar('\n');
	printf("Total %d\n",total);

	/* Conta o numero de pipes */
	for(i=0;i<total;i++){
		if(strcmp(s[i],token) == 0)
			n_pipes++;  // numero de comandos
	}

	printf("Numero de comandos: %d\n",n_pipes+1);

	

	/*  Colocar na variavel comandos os comandos encadeados a executar  */
	for(i=0; k<total; i++){
		flag=0;
		for(j=0;(j<7 && !flag && k<total);j++){
			if(strcmp(s[k],token) != 0){
				comandos[i][j]=s[k];
				//strcpy(comandos[i][j],s[k]);
				printf("%s\n", comandos[i][j]);
				k++;
			}
			else{
				comandos[i][j]=NULL;
				printf("%s\n", comandos[i][j]);
				flag=1;
				k++;
			}

		}
		printf("DEBUGGGGGGGG3\n");
	}

	comandos[i][j]=0; 
	

	/* Alocaçao do espaco para os descritroes  */
	fd = malloc (n_pipes * sizeof (int *));
	for ( i = 0; i < n_pipes; ++i)
		fd[i] = malloc (2 * sizeof (int));


	/*  Criar os n_pipes pipes anonimos  */
	for(i=0;i<n_pipes;i++){
		if(pipe(fd[i]) == -1){
			sprintf(buffer,"Erro ao criar o pipe %d",i+1);
			perror(buffer);
			_exit(2);
		}
	}

	/* Alocacao de espaco para guardar os pids*/
	pid = malloc(n_pipes * sizeof(int));


	/*  Executar atraves de pipes anonimos o camando encadeado  */
	for(i=0;i<n_pipes+1;i++){
		if((pid[i]=fork()) == -1){
			sprintf(buffer, "Erro no fork %d",i+1);
			perror(buffer);
			_exit(EXIT_FAILURE);
		}
		if(pid[i]==0){
			if(i==0){
				close(fd[i][0]);
				if(dup2(fd[i][1],1) == -1){
					sprintf(buffer,"Erro no dup2 %d",i+1);
					perror(buffer);
					_exit(EXIT_FAILURE);
				}
				close(fd[i][1]);
				execvp(comandos[i][0],comandos[i]);
				sprintf(buffer,"Erro no execvp %d",i+1);
				perror(buffer);
				_exit(EXIT_FAILURE);
			}
			else {
				if(i==(n_pipes-1)){
					if(dup2(fd[i-1][0],0)==-1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(EXIT_FAILURE);
					}
					close(fd[i-1][0]);
					execvp(comandos[i][0],comandos[i]);
					sprintf(buffer,"Erro no execvp %d",i+1);
					perror(buffer);
					_exit(EXIT_FAILURE);
				}
				else{
					// tenho que fechar os descritores anteriores
					//close(fd[i-1][1]);
					// Duplica o stdin para a leitura do pipe anterior. A leitura anterior tem que estar no stdin
					if(dup2(fd[i-1][0],0) == -1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(EXIT_FAILURE);
					}
					close(fd[i-1][0]);
					close(fd[i][0]);  // fecha a leitura atual
					if(dup2(fd[i][1],1) == -1){
						sprintf(buffer,"Erro no dup2 %d",i+1);
						perror(buffer);
						_exit(EXIT_FAILURE);
					}
					close(fd[i][1]);
					execvp(comandos[i][0],comandos[i]);
					sprintf(buffer,"Erro no execvp %d",i+1);
					perror(buffer);
					_exit(EXIT_FAILURE);
				}
			}
		}
		else{
			close(fd[i][1]);
			wait(NULL);
		}
	}	


}


/*






*/