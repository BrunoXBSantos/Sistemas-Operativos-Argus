#include <sys/types.h> 	// open
#include <sys/stat.h> 	// open
#include <fcntl.h>  	// open

#include <unistd.h> 	// write & read
#include <stdio.h>		// sprintf
#include <string.h>		// sprintf

#include <sys/wait.h>
#include <stdlib.h>




void comunicacao();
int separarComandos();
void rmv(char *str);
int separarComandos2();
void executarTarefa();

char *fifo = "/tmp/fifo";  // FIFO file path
int fd_log, fd_fifo;
char buffer[80];

int main(int argc, char *argv[]){

	// Declaracao de variaveis 
	
	int  n;
	
	char *file_log = "log.txt"; // ficheiro que guarda a comunicacao com o cliente
	char *aceite = "ACEITE";
	char *comunicar = "COMUNICAR";

	/* Abrir || Criar o ficheiro log.txt  */
	if((fd_log = open(file_log,O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1){
		sprintf(buffer,"Abrir ou criar ficheiro log.txt");
		perror(buffer);
		_exit(1);
	}

	/* Criar o ficheiro especial - FIFO FILE  creating the named file (FIFO) */
	if(mkfifo(fifo,0644) == -2){
		sprintf(buffer,"Criar o fifo file");
		perror(buffer);
		_exit(2);
	}

	fd_fifo=open(fifo,O_RDONLY);
	

	while((n=read(fd_fifo,buffer,1024)) > 0  || 1){
		if(n>0){
			/* Verifica qual das 2 opcoes vai executar o server*/
			if(strcmp(buffer,"COMUNICAR") == 0){
				close(fd_fifo);
				fd_fifo = open(fifo,O_WRONLY);
				write(fd_fifo,aceite,strlen(aceite)+1);
				close(fd_fifo);
				comunicacao();
				pause();
			}
			else{
				printf("%s\n",buffer);
			}
		}
	}


}

void executarTarefa(){
	char buffer[80] = "";
	int n, i;
	char comandos[20][20];
	int n_tokens = 0;
	char *comandos2[20][20];
	int n_pipes = 0;
	int fd[10][2], pid[10];
	int flag = 0,j;

	fd_fifo=open(fifo,O_RDONLY);
	n=read(fd_fifo,buffer,80);
	puts(buffer);
	close(fd_fifo);
	buffer[strlen(buffer)-1] = '\0';

	/* coloca as palavras num array de strings*/
	n_tokens = separarComandos(comandos,buffer);

	printf("%d\n",n_tokens);
	for(i=0;i<n_tokens;i++){
		printf(" %s %d\n",comandos[i], strlen(comandos[i]));
	}

	/* coloca em comandos2 a lista definitiva de comandos a executar*/
	n_pipes = separarComandos2(comandos,comandos2,n_tokens);

	printf("%d\n",n_pipes);

	for(i=0;i<n_pipes+1;i++){
		for(j=0;j<20; j++){
			if(comandos2[i][j]!=NULL){
				printf("%s ",comandos2[i][j]);
			}
			else{
				j=19;
				printf("NULL");
			}
		}
		putchar('\n');
	}

	// criar os n_pipes pipes
	for(i=0;i<n_pipes;i++){
		if(pipe(fd[i]) == -1){
			sprintf(buffer,"Erro ao criar o pipe %d",i+1);
			perror(buffer);
			_exit(EXIT_FAILURE);
		}
	}

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
					sprintf(buffer,"Erro no dup2 %duuuu",i+1);
					perror(buffer);
					_exit(EXIT_FAILURE);
				}
				close(fd[i][1]);
				printf("\nDebug %d\n",i);
				execvp(comandos2[i][0],comandos2[i]);
				sprintf(buffer,"Erro no execvp %d",i+1);
				perror(buffer);
				_exit(EXIT_FAILURE);
			}
			else {
				if(i==n_pipes){
					if(dup2(fd[i-1][0],0)==-1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(EXIT_FAILURE);
					}
					close(fd[i-1][0]);
					execvp(comandos2[i][0],comandos2[i]);
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
					execvp(comandos2[i][0],comandos2[i]);
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

void comunicacao(){
	executarTarefa();
}



int separarComandos2(char comandos[20][20], char *comandos2[20][20], int n_tokens){
	int n_pipes = 0;
	int j,k,i;
	int flag;
	printf("%d\n",n_tokens);
	for(k=i=0;k<n_tokens;i++){
		flag = 0;
		printf("Debug %d\n",i);
		for(j=0; !flag; j++){
			if(strcmp(comandos[k],"|") != 0){
				comandos2[i][j]=&(comandos[k][0]);
				printf("%s ",comandos2[i][j]);
				k++;
			}
			else{
				comandos2[i][j]=NULL;
				n_pipes++;
				flag = 1;
				k++;
			}
			
			if(k>=n_tokens)
				flag=1;
		}
		putchar('\n');
	}
	comandos2[i-1][j]=NULL;
	return n_pipes;
}

/* Coloca em comandos as palavras compostas no buffer  */
/* Retorna o numero de palavras - tamanho do vetor*/
int separarComandos(char comandos[20][20], char buffer[80]){
	int i = 0;
	const char s[2] = " ";
	char *token;

	/* get the first token */
	token = strtok(buffer, s);

	/* walk through other tokens */
	while( token != NULL ) {	
	  rmv(token);
	  printf( " %s\n", token );
	  strcpy(comandos[i],token);
	  printf( "%s\n", comandos[i] );
	  i++;
	  token = strtok(NULL, s);
	  
   }
   return i; 
}

void rmv(char *str){ 
    int count=0,i;
    for(i=0;str[i];i++){
        if(str[i]!=' ' || str[i]!='\n'){
            str[count++]=str[i];
        }
    }
    str[count]=0;
} 
