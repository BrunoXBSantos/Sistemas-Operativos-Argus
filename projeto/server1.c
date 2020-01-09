#include <sys/types.h> 	// open
#include <sys/stat.h> 	// open
#include <fcntl.h>  	// open

#include <unistd.h> 	// write & read
#include <stdio.h>		// sprintf
#include <string.h>		// sprintf

#include <sys/wait.h>
#include <stdlib.h>

#include <signal.h> // sigaction(), sigsuspend(), sig*()

#define MAX_PIPES_COMANDO 200
#define MAX_PALAVRAS_PIPE 200
#define MAX_CARACTERES_COMANDO 200


typedef struct idtarefa{
	int identificacaoTarefa;
	char tarefa[80];
	int pidTarefa;
	struct idtarefa *prox;
} *LLista;


void comunicacao();
void executarTarefa();

int separarComandos();
void rmv(char *str);
int separarComandos2();
char *getPrimeiraPalavra(char buffer[80]);
char *getSegundaPalavra(char q[80]);
int readln(int fildes, char *buf, int nbyte);

void handle_filhoSecundTerminou(int sig);
void handle_filhoPrincipalTerminou(int sig);
void alarme(int sig);
void handle_sigTerminarTarefa(int sig);

void historico();
int listar();
int getPidTarefaExecucao(int numeroTarefaTerminar);

int removerTarefaListaExecucao(int pidTarefa);
int inserirElementoLista(int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa);
int inserirElementoLista(int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa);
LLista criarElemento(int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa);

char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";
int fd_fifo1, fd_fifo2;
char buffer[MAX_CARACTERES_COMANDO];
int numeracaoTarefas = 0;
int pid; // para cada tarefa guardar o seu pid
int statusFilhos;
int statusPrincipal;

int tempo_execucao = 100; // tempo maximo de execucao de uma tarefa
int tempo_inatividade = 100;

int flag_tempo_execucao = 0;  // se 0 o tempo de execucao não chegou ao fim. Se 1 chegou ao fim
int flag_tempo_inatividade = 0; // se 0 o tempo de execucao não chegou ao fim. Se 1 chegou ao fim
int flag_tarefa_terminada = 0;
int flag_erroExecucaoTarefa = 0;

int fd_tarefasConcluidas;
int contaPipesExecutados = 1;
int tempo = 0;
int filhoMorreu = 0;

// Inicializacao das listas ligadas
LLista tarefasExecucao = NULL;


int main(int argc, char *argv[]){

	int n = 0;

	fd_tarefasConcluidas = open("lista-tarefas-concluidas.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	char comandosRecebidos[MAX_CARACTERES_COMANDO];
	memset(comandosRecebidos,0,sizeof(comandosRecebidos));

	/* Criar o ficheiro especial - FIFO FILE  creating the named file (FIFO) */
	if(mkfifo(fifo1,0644) == -2){
		sprintf(buffer,"Criar o fifo file");
		perror(buffer);
		_exit(2);
	}

	mkfifo(fifo2,0644);

	
	fd_fifo1=open(fifo1,O_RDONLY);
	fd_fifo2=open(fifo2,O_WRONLY);	

	while((n=read(fd_fifo1,comandosRecebidos,MAX_CARACTERES_COMANDO)) > 0  || 1){
		if(n>0){
			comandosRecebidos[n]='\0';
			comunicacao(comandosRecebidos);
			memset(comandosRecebidos,0,sizeof(comandosRecebidos));
		}
		
	}
}

void comunicacao(char comandosRecebidos[MAX_CARACTERES_COMANDO]){
	
	char *primeiraPalavra;
	int numeroTarefaTerminar, o, pidTarefaTerminar;

	signal(SIGCHLD, handle_filhoPrincipalTerminou);

	comandosRecebidos[strlen(comandosRecebidos)-1] = '\0';   // para tirar o '\n'

	strcpy(buffer,comandosRecebidos);	
	
	primeiraPalavra = getPrimeiraPalavra(buffer);

	if((strcmp(primeiraPalavra,"tempo-inatividade") == 0) || (strcmp(primeiraPalavra,"-i") == 0)){
		strcpy(buffer,comandosRecebidos);
		tempo_inatividade = atoi(getSegundaPalavra(buffer));
	}
	else if((strcmp(primeiraPalavra,"tempo-execucao") == 0) || (strcmp(primeiraPalavra,"-m") == 0)){
		strcpy(buffer,comandosRecebidos);
		tempo_execucao = atoi(getSegundaPalavra(buffer));		
	}
	else if((strcmp(primeiraPalavra,"executar") == 0) || (strcmp(primeiraPalavra,"-e") == 0)){
		char temp[17];
		numeracaoTarefas++;
		
		/*	Envia para o argus o numero da tarefa a ser executada */
		sprintf(temp, "nova tarefa #%2d\n",numeracaoTarefas);
		write(fd_fifo2,temp,17);
		
		int pid = fork();

		if(pid == 0){
			executarTarefa(&(comandosRecebidos[strlen(primeiraPalavra)+1]),numeracaoTarefas);
		}
		
		/* Coloca na lista de tarefas a serem executadas */
		inserirElementoLista(numeracaoTarefas,&(comandosRecebidos[strlen(primeiraPalavra)+1]), pid);

	}
	else if((strcmp(primeiraPalavra,"listar") == 0) || (strcmp(primeiraPalavra,"-l") == 0)){	
		o = listar(&tarefasExecucao);
	
		
	}
	else if((strcmp(primeiraPalavra,"terminar") == 0) || (strcmp(primeiraPalavra,"-t") == 0)){
		strcpy(buffer,comandosRecebidos);
		numeroTarefaTerminar = atoi(getSegundaPalavra(buffer));
		pidTarefaTerminar = getPidTarefaExecucao(numeroTarefaTerminar);
		kill(pidTarefaTerminar,SIGUSR1);

	}
	else if((strcmp(primeiraPalavra,"historico") == 0) || (strcmp(primeiraPalavra,"-r") == 0)){
		historico();
	}

	else if((strcmp(primeiraPalavra,"ajuda") == 0) || (strcmp(primeiraPalavra,"-h") == 0)){
		printf("ajuda\n");
	}
	
}

void executarTarefa(char *comandosRecebidos, int numeracaoTarefas){
	int n, i;
	char comandos[MAX_PIPES_COMANDO+1][MAX_PALAVRAS_PIPE];
	int n_tokens = 0;
	char *comandos2[MAX_PIPES_COMANDO+1][MAX_PALAVRAS_PIPE];
	int n_pipes = 0;
	int fd[MAX_PIPES_COMANDO][2];
	int j;

	contaPipesExecutados = 1;
	flag_tempo_execucao = 0;
	flag_tempo_inatividade = 0;
	flag_tarefa_terminada=0;
	filhoMorreu = 0;
	tempo = 0;


	// ativar o alarme com o tempo maximo de execucao de uma tarefa
	signal(SIGALRM, alarme);
	signal(SIGCHLD, handle_filhoSecundTerminou);
	signal(SIGUSR1, handle_sigTerminarTarefa);
	
	// system("ps");

	char tarefa[MAX_CARACTERES_COMANDO] = "";
	strcpy(tarefa,comandosRecebidos);

	/* coloca as palavras num array de strings*/
	n_tokens = separarComandos(comandos,comandosRecebidos);

	/* coloca em comandos2 a lista definitiva de comandos a executar*/
	n_pipes = separarComandos2(comandos,comandos2,n_tokens);

	// criar os n_pipes pipes
	for(i=0;i<n_pipes+2;i++){
		if(pipe(fd[i]) == -1){
			sprintf(buffer,"Erro ao criar o pipe %d",i+1);
			perror(buffer);
			_exit(EXIT_FAILURE);
		}
	}

	alarm(1); // ativa o contador

	for(i=0;i<n_pipes+2 && !flag_tempo_execucao && !flag_tempo_inatividade && !flag_tarefa_terminada && !flag_erroExecucaoTarefa ;i++){
		if((pid=fork()) == -1){
			sprintf(buffer, "Erro no fork %d",i+1);
			perror(buffer);
			_exit(EXIT_FAILURE);
		}
		if(pid==0){
			if(i==0){
				close(fd[i][0]);
				if(dup2(fd[i][1],1) == -1){
					sprintf(buffer,"Erro no dup2 %duuuu",i+1);
					perror(buffer);
					_exit(-1);
				}
				close(fd[i][1]);
				execvp(comandos2[i][0],comandos2[i]);
				flag_erroExecucaoTarefa = 1;
				//sprintf(buffer,"Erro no execvp %d",i+1);
				//perror(buffer);
				_exit(-1);
			}
			else {
				if(i==n_pipes+1){
					if(dup2(fd[i-1][0],0)==-1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(-1);
					}

					sprintf(buffer,"\n\tRESULTADO TAREFA #%d:\n",numeracaoTarefas);
					write(1,buffer, strlen(buffer)+1);
					n=read(fd[i-1][0],buffer,1024);
					write(1,buffer,n);

					close(fd[i-1][0]);

					_exit(0);
				}
				else{
					// tenho que fechar os descritores anteriores
					//close(fd[i-1][1]);
					// Duplica o stdin para a leitura do pipe anterior. A leitura anterior tem que estar no stdin
					if(dup2(fd[i-1][0],0) == -1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(-1);
					}
					close(fd[i-1][0]);
					close(fd[i][0]);  // fecha a leitura atual
					if(dup2(fd[i][1],1) == -1){
						sprintf(buffer,"Erro no dup2 %d",i+1);
						perror(buffer);
						_exit(-1);
					}
					close(fd[i][1]);
					execvp(comandos2[i][0],comandos2[i]);
					flag_erroExecucaoTarefa = 1;
					//sprintf(buffer,"Erro no execvp %d",i+1);
					//perror(buffer);
					_exit(-1);
				}
			}
		}
		else{
			close(fd[i][1]);
			while(!filhoMorreu && !flag_tempo_inatividade && !flag_tempo_execucao){
				pause();
			}
			filhoMorreu = 0;

			if(flag_tempo_execucao==1){
				kill(pid,SIGKILL);
				alarm(0);
			}
			else if(flag_tempo_inatividade == 1){
				kill(pid,SIGKILL);
				alarm(0);
			}
			else if(flag_tarefa_terminada == 1){
				alarm(0);
			}
			else if(flag_erroExecucaoTarefa == 1){
				alarm(0);
			}

		}
	}	
	
	alarm(0);
	
	// colocar no ficheiro de tarefas concluidas
	if(flag_tempo_execucao){ // nao foi executada com exito
		// colocar no ficheiro de tarefas concluidas
		sprintf(buffer,"#%d, max. execucao: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,strlen(buffer));
		write(fd_tarefasConcluidas,"\n",1);
		sprintf(buffer,"\n\tRESULTADO TAREFA #%d:\nMaximo de execucao atingido\n",numeracaoTarefas);
		write(1,buffer, strlen(buffer));
	}
	else if(flag_tempo_inatividade){
		// colocar no ficheiro de tarefas concluidas
		sprintf(buffer,"#%d, max. inactividade: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,strlen(buffer));
		write(fd_tarefasConcluidas,"\n",1);
		sprintf(buffer,"\n\tRESULTADO TAREFA #%d:\nMaximo de inatividade atingido\n",numeracaoTarefas);
		write(1,buffer, strlen(buffer)+1);
	}
	else if(flag_tarefa_terminada){
		// colocar no ficheiro de tarefas concluidas
		sprintf(buffer,"#%d, cancelada: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,strlen(buffer));
		write(fd_tarefasConcluidas,"\n",1);
		sprintf(buffer,"\n\tRESULTADO TAREFA #%d:\nTarefa terminada pelo utilizador\n",numeracaoTarefas);
		write(1,buffer, strlen(buffer)+1);
	}
	else if(flag_erroExecucaoTarefa){
		// colocar no ficheiro de tarefas concluidas
		sprintf(buffer,"#%d, Erro ao executar: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,strlen(buffer));
		write(fd_tarefasConcluidas,"\n",1);
		sprintf(buffer,"\n\tRESULTADO TAREFA #%d:\nErro ao executar a tarefa\n",numeracaoTarefas);
		write(1,buffer, strlen(buffer)+1);
	}
	else{		// executada com exito
		sprintf(buffer,"#%d, concluida: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,strlen(buffer));
		write(fd_tarefasConcluidas,"\n",1);
	}

	_exit(0);	
}

int getPidTarefaExecucao(int numeroTarefaTerminar){
	LLista p = (tarefasExecucao);
	if(p==NULL)
		return 0;
	while(p!=NULL){
		if(p->identificacaoTarefa == numeroTarefaTerminar){
			return (p->pidTarefa);
		}
		p=p->prox;
	}
	return -1;
}

/////////           FUNCOES DE SINAIS              ///////////////////////

void handle_filhoPrincipalTerminou(int sig){
	int j;
	j=wait(NULL);
	removerTarefaListaExecucao(j);
}

void handle_filhoSecundTerminou(int sig){
	int pid = wait(&statusFilhos); 
	contaPipesExecutados++;
	if(WEXITSTATUS(statusFilhos) == 255 || !WIFEXITED(statusFilhos)){
		flag_erroExecucaoTarefa=1;
	}
	filhoMorreu=1;
}

void handle_sigTerminarTarefa(int sig){
	kill(pid, SIGKILL);
	flag_tarefa_terminada = 1;
}

void alarme(int sig){
	int j;
	tempo+=1;
	if(tempo >= tempo_execucao){
		flag_tempo_execucao = 1;
		alarm(0);
	}
	else if(tempo >= contaPipesExecutados * tempo_inatividade){
		flag_tempo_inatividade = 1;
		alarm(0);
	}
	else{
		alarm(1);
	} 	

}

int listar(){
	char temp[1024]="";
	LLista p=tarefasExecucao;
	if(p==NULL){
		write(fd_fifo2,"Nenhuma Tarefa em execucao\nFIM TRANSMISSAO\n",44);
		return 1;
	}
	else{
		while((p)!=NULL){
			if((p->prox) == NULL){
				sprintf(temp,"#%d: %s\nFIM TRANSMISSAO\n",(p)->identificacaoTarefa,(p)->tarefa);
				write(fd_fifo2,temp,strlen(temp)+1);
			}
			else{
				sprintf(temp,"#%d: %s\n",(p)->identificacaoTarefa,(p)->tarefa);
				write(fd_fifo2,temp,strlen(temp)+1);
			}
			(p)=(p)->prox;
		}
		return 1;
	}
	return -1;
}

void historico(){
	char temp[1024]="";
	int n, fd = open("lista-tarefas-concluidas.txt", O_RDONLY);
	while((n=readln(fd,temp,1024)) > 0){
		write(fd_fifo2,temp,n);
	}
	write(fd_fifo2,"FIM TRANSMISSAO\n",16);
	close(fd);
}

////////////////  FUNCOES DE MANIPLUCAO DE COMANDOS    /////////////

char *getSegundaPalavra(char q[80]){
	const char s[2] = " ";
	char *token;
	/* get the first token */
	token = strtok(q, s);
	/* get the second token*/
	token = strtok(NULL, s);
	return token;
}


int separarComandos2(char comandos[MAX_PIPES_COMANDO+1][MAX_PALAVRAS_PIPE], char *comandos2[MAX_PIPES_COMANDO+1][MAX_PALAVRAS_PIPE], int n_tokens){
	int n_pipes = 0;
	int j,k,i;
	int flag;

	for(k=i=0;k<n_tokens;i++){
		flag = 0;
		for(j=0; !flag; j++){
			if(strcmp(comandos[k],"|") != 0){
				comandos2[i][j]=&(comandos[k][0]);
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
	}
	comandos2[i-1][j]=NULL;
	return n_pipes;
}

/* Coloca em comandos as palavras compostas no buffer  */
/* Retorna o numero de palavras - tamanho do vetor*/
int separarComandos(char comandos[MAX_PIPES_COMANDO+1][MAX_PALAVRAS_PIPE], char buffer[MAX_CARACTERES_COMANDO]){
	int i = 0;
	const char s[2] = " ";
	char *token;

	/* get the first token */
	token = strtok(buffer, s);

	/* walk through other tokens */
	while( token != NULL ) {	
	  rmv(token);
	  strcpy(comandos[i],token);
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
	token = strtok(q, s);
	return token;
}

int readln(int fildes, char *buf, int nbyte){
	int n, flag = 0;
	char a;
	int count = 0;
	while(!flag){
		if((n=read(fildes,&a,1))<=0)
			flag = 1;
		if(a=='\n' || count >= nbyte){
			(*buf) = '\n';
			flag = 1;
		}
		else
			(*buf++) = a;
		count++;
	}
	if(n>0)
		return count;
	if(!n)
		return 0;

	return -1;
}	


//////////  FUNCOES DA LISTA LIGADA    /////////////////////////////


// criar um novo elemento da lista ligada, que no contexto do problema e um artigo
// Devolve o endereco do elemento criado em caso de sucesso, em caso de insucesso, devolve NULL
LLista criarElemento(int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa){
	LLista novaTarefa = (LLista) malloc(sizeof(struct idtarefa));
	if(novaTarefa!=NULL){
		(novaTarefa->identificacaoTarefa)=identificacaoTarefa;
		strcpy((novaTarefa->tarefa),tarefaAdicionar);
		(novaTarefa->pidTarefa) = pidTarefa;
		novaTarefa->prox = NULL;
	}
	return novaTarefa;
}

// Insere um artigo na lista.
// A lista esta ordenada por ordem crescente do numero da referencia do artigo
// Devolve, -1 se nao conseguir criar o artigo. 0 se o artigo ja exitir em stock(atualiza o stock). 1 se
//for criado um novo artigo.

int inserirElementoLista(int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa){
	
	LLista p = (tarefasExecucao), novo = criarElemento(identificacaoTarefa,tarefaAdicionar,pidTarefa);
	LLista c=tarefasExecucao;

	if(novo==NULL){
		;
	}
	if(p==NULL){
		p=novo;
		tarefasExecucao=novo;		
		return 1;
	}
	while(p->prox != NULL) 
		p=p->prox; 
	p->prox = novo;
	while(c!=NULL){
		c=c->prox;
	}
	return 1;

}

int removerTarefaListaExecucao(int pidTarefa){
	LLista p=tarefasExecucao, q=NULL;	

	if(tarefasExecucao == NULL)
		return 0;
	if((tarefasExecucao->pidTarefa) == pidTarefa){
		p = tarefasExecucao;
		tarefasExecucao = tarefasExecucao->prox;
		free(p);
		return 1;
	}
	q=(tarefasExecucao)->prox;
	while(q!=NULL && pidTarefa != (q->pidTarefa)){
		p=q;
		q=q->prox;
	}
	if(q!=NULL){
		p->prox=q->prox;
		free(q);
		return 1;
	}
	return 0;
}