#include <sys/types.h> 	// open
#include <sys/stat.h> 	// open
#include <fcntl.h>  	// open

#include <unistd.h> 	// write & read
#include <stdio.h>		// sprintf
#include <string.h>		// sprintf

#include <sys/wait.h>
#include <stdlib.h>

#include <signal.h> // sigaction(), sigsuspend(), sig*()




void comunicacao();
int separarComandos();
void rmv(char *str);
int separarComandos2();
void executarTarefa();
char *getPrimeiraPalavra(char buffer[80]);

char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";
int fd_log, fd_fifo1, fd_fifo2;
char buffer[80];
int numeracaoTarefas = 0;
int *pt_numeracaoTarefas = &numeracaoTarefas;

int fd_tarefasConcluidas;


typedef struct idtarefa{
	int identificacao;
	char tarefa[80];
	struct idtarefa *prox;
} *LLista;

// criar um novo elemento da lista ligada, que no contexto do problema e um artigo
// Devolve o endereco do elemento criado em caso de sucesso, em caso de insucesso, devolve NULL
LLista criarElemento(int identificacao, char tarefaAdicionar[80]){
	LLista novaTarefa = (LLista) malloc(sizeof(struct idtarefa));
	if(novaTarefa!=NULL){
		(novaTarefa->identificacao)=identificacao;
		strcpy((novaTarefa->tarefa),tarefaAdicionar);
		novaTarefa->prox = NULL;
	}
	return novaTarefa;
}

// Procura se existe um artigo pela sua referencia na lista
//A funcao rece a cabeca da lista e a referencia a procurar
// Devolve o endereco do artigo se encontrar a sua referencia. Devolve NULL caso a referencia nao existe na
//lista
LLista procurar(LLista l, int refe){
	while(l!=NULL && (l->identificacao) != refe)
		l=l->prox;
	return l;
}


// Insere um artigo na lista.
// A lista esta ordenada por ordem crescente do numero da referencia do artigo
// Devolve, -1 se nao conseguir criar o artigo. 0 se o artigo ja exitir em stock(atualiza o stock). 1 se
//for criado um novo artigo.

int inserirElementoLista(LLista *l,int identificacao, char tarefaAdicionar[80]){
	LLista p = NULL, novo = criarElemento(identificacao,tarefaAdicionar);
	if(novo==NULL){
		;
	}
	if((*l)==NULL){
		(*l)=novo;
		return 1;
	}
	p=procurar((*l), identificacao-1);
	p->prox=novo; 
	return 1;

}

// Inicializacao das listas ligadas
LLista tarefasTerminadas = NULL;

void filhoterminou() {
	int j;
	j=wait(NULL);
	puts("FILHO TERMINOU");
	printf("pid do filho %d\n",j);
}

int main(int argc, char *argv[]){

	int pid[10];
	int i = 0;

	signal(SIGCHLD, filhoterminou);

	// Declaracao de variaveis 
	
	int  n;
	char comandosRecebidos[80]="";
	

	/* Criar o ficheiro especial - FIFO FILE  creating the named file (FIFO) */
	if(mkfifo(fifo1,0644) == -2){
		sprintf(buffer,"Criar o fifo file");
		perror(buffer);
		_exit(2);
	}

	mkfifo(fifo2,0644);

	
	fd_fifo1=open(fifo1,O_RDONLY);
	fd_fifo2=open(fifo2,O_WRONLY);	

	while((n=read(fd_fifo1,comandosRecebidos,1024)) > 0  || 1){
		int pid;
		if(n>0){
				(*pt_tarefasExecucao)+=1;
				pid = fork();
				if(pid==0){
					puts("FUNCAO PRINCIPAL");
					printf("pid do filho %d\n",getpid());
					comunicacao(comandosRecebidos);
					puts("finish da tarefinha");
					puts(comandosRecebidos);
					strcpy(comandosRecebidos,"");
					puts(comandosRecebidos);
					puts("DEBUGGGGGGGG\n\n\n\n");
					
					
				}
				i++;
					
		}
		wait(NULL);
	}
}



void comunicacao(char comandosRecebidos[80]){
	
	char *primeiraPalavra;
	int n;

	puts("FUNCAO comunicacao ");
	printf("%d\n",strlen(comandosRecebidos));
	comandosRecebidos[strlen(comandosRecebidos)-1] = '\0';
	puts(comandosRecebidos);

	strcpy(buffer,comandosRecebidos);	
	
	primeiraPalavra = getPrimeiraPalavra(buffer);
	
	printf("%d\n",strlen(comandosRecebidos));
	printf("primeira palavra: %s\n",primeiraPalavra);
	printf("tamanho primeira palavra: %d\n",strlen(primeiraPalavra));

	if(strcmp(primeiraPalavra,"tempo-inactividade") == 0){
		printf("tempo-inactividade\n");
	}
	else if(strcmp(primeiraPalavra,"tempo-execucao") == 0){
		printf("tempo-execucao\n");
	}
	else if(strcmp(primeiraPalavra,"executar") == 0){
		char temp[17];
		(*pt_numeracaoTarefas)+=1;
		numeroTarefaAtual = (*pt_numeracaoTarefas);
		printf("Numero da tarefa a ser executada: %d  %d\n",numeroTarefaAtual,(*pt_numeracaoTarefas));
		sprintf(temp, "nova tarefa #%2d\n",numeroTarefaAtual);
		write(fd_fifo2,temp,17);
		
		printf("%s\n",&(comandosRecebidos[strlen(primeiraPalavra)+1]));
		executarTarefa(&(comandosRecebidos[strlen(primeiraPalavra)+1]),numeroTarefaAtual);
		
	}
	else if(strcmp(primeiraPalavra,"listar") == 0){
		printf("listar\n");
	}
	else if(strcmp(primeiraPalavra,"terminar") == 0){
		printf("terminar\n");
	}
	else if(strcmp(primeiraPalavra,"historico") == 0){
		printf("historico\n");
		historico();

	}

	else if(strcmp(primeiraPalavra,"ajuda") == 0){
		printf("ajuda\n");
	}

	// executar grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l


	
}

void historico(){
	listar(tarefasTerminadas);
}

void executarTarefa(char *comandosRecebidos, int numeracaoTarefas){
	int n, i;
	char comandos[20][20];
	int n_tokens = 0;
	char *comandos2[20][20];
	int n_pipes = 0;
	int fd[10][2], pid[10];
	int flag = 0,j;
	int fd1 = open("teste.txt",O_CREAT | O_RDWR, 0644);
	int flag_execucaoTarefa = 0;


	char tarefa[80] = "";
	strcpy(tarefa,comandosRecebidos);



	/* coloca as palavras num array de strings*/
	n_tokens = separarComandos(comandos,comandosRecebidos);

	printf("Numero de palavras contidas no comando: %d\n",n_tokens);
	for(i=0;i<n_tokens;i++){
		printf("%s %d\n",comandos[i], strlen(comandos[i]));
	}

	/* coloca em comandos2 a lista definitiva de comandos a executar*/
	n_pipes = separarComandos2(comandos,comandos2,n_tokens);

	puts("FUNCAO executarTarefa ");
	printf("Numero de pipes existentes: %d\n",n_pipes);

	puts("Lista definitiva com NULL de comandos sem o pipe");
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

	for(i=0;i<n_pipes+1 && !flag_execucaoTarefa;i++){
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
					//sleep(5);
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
	if(flag_execucaoTarefa == 1){
		sprintf(buffer,"Tarefa #%2d executada sem exito",numeracaoTarefas);
		write(1,buffer,31);
		write(fd_fifo2,buffer,31);
	}
	else{
		sprintf(buffer,"Tarefa #%2d concluida com sucesso\n",numeracaoTarefas);
		write(1,buffer,34);
		write(fd_fifo2,buffer,34);
		inserirElementoLista(&tarefasTerminadas,numeracaoTarefas,tarefa);
		puts(tarefa);
		printf("Tarefa executada: %d\n",numeracaoTarefas);

		// colocar no ficheiro de tarefas concluidas
		printf("tamanho da TAREFAAAAA %d\n\n\n\n",strlen(tarefa));
		sprintf(buffer,"#%d, concluida: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,16+strlen(tarefa));
		write(fd_tarefasConcluidas,"\n",1);
	}
}



int separarComandos2(char comandos[20][20], char *comandos2[20][20], int n_tokens){
	int n_pipes = 0;
	int j,k,i;
	int flag;

	puts("FUNCAO separarComandos2 ");

	puts("Lista de comandos sem o pipe");
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
	puts("FUNCAO separarComandos ");

	puts("DEBUG SEPARAR COMANDOS");

	/* walk through other tokens */
	while( token != NULL ) {	
	  rmv(token);
	  //printf( " %s\n", token );
	  strcpy(comandos[i],token);
	  //printf( "%s\n", comandos[i] );
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

char *getPrimeiraPalavra(char q[80]){
	const char s[2] = " ";
	char *token;
	/* get the first token */
	token = strtok(q, s);
	return token;
}
