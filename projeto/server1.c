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
void historico();
int readln(int fildes, char *buf, int nbyte);
void filhoterminou(int sig);
char *getSegundaPalavra(char q[80]);
void filhoPrincipalTerminou(int sig);
void listarTeste();

char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";
int fd_log, fd_fifo1, fd_fifo2;
char buffer[80];
int numeracaoTarefas = 0;
int *pt_numeracaoTarefas = &numeracaoTarefas;

int tempo_execucao = 100; // tempo maximo de execucao de uma tarefa
int tempo_inatividade = 100;

int flag_tempo_execucao = 0;  // se 0 o tempo de execucao não chegou ao fim. Se 1 chegou ao fim
int flag_tempo_inatividade = 0; // se 0 o tempo de execucao não chegou ao fim. Se 1 chegou ao fim
int fd_tarefasConcluidas;
int contaPipesExecutados = 1;
int tempo = 0;
int filhoMorreu = 0;


typedef struct idtarefa{
	int identificacaoTarefa;
	char tarefa[80];
	int pidTarefa;
	struct idtarefa *prox;
} *LLista;

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

// Procura se existe um artigo pela sua referencia na lista
//A funcao rece a cabeca da lista e a referencia a procurar
// Devolve o endereco do artigo se encontrar a sua referencia. Devolve NULL caso a referencia nao existe na
//lista
LLista procurar(LLista l, int refe){
	while(l!=NULL && (l->identificacaoTarefa) != refe)
		l=l->prox;
	return l;
}

// EXIBIR conteudo
void listar(LLista l){
	char temp[105];
	
	while(l!=NULL && (l->prox) != NULL){
		sprintf(temp, "#%2d, concluida: %s",l->identificacaoTarefa,l->tarefa);
		write(fd_fifo2,temp,17+strlen(l->tarefa));
		printf("%d %s\n",l->identificacaoTarefa, l->tarefa);
		l=l->prox;
	}
}

void listarTeste(LLista *l){
	char temp[100]="";
	LLista p = (*l);
	while((p)!=NULL){
		sprintf(temp,"#%d: %s\n",(p)->identificacaoTarefa,(p)->tarefa);
		write(fd_fifo2,temp,5+strlen(temp));
		printf("ENVIADA: %s - %d\n",temp,strlen(temp));
		printf("#%d - %s - %d\n",(p->identificacaoTarefa),(p->tarefa), (p->pidTarefa));
		(p)=(p)->prox;
	}
	write(1,"FIM TRANSMISSAO\n",17);
	write(fd_fifo2,"FIM TRANSMISSAO",16);
}

// Insere um artigo na lista.
// A lista esta ordenada por ordem crescente do numero da referencia do artigo
// Devolve, -1 se nao conseguir criar o artigo. 0 se o artigo ja exitir em stock(atualiza o stock). 1 se
//for criado um novo artigo.

int inserirElementoLista(LLista *l,int identificacaoTarefa, char tarefaAdicionar[80], int pidTarefa){
	LLista p = NULL, novo = criarElemento(identificacaoTarefa,tarefaAdicionar,pidTarefa);
	printf("ELEMENTO CRIADO\n");
	printf("#%d - %s - %d\n",novo->identificacaoTarefa,novo->tarefa, novo->pidTarefa);

	if(novo==NULL){
		;
	}
	if((*l)==NULL){
		(*l)=novo;
		return 1;
	}
	while((*l)->prox != NULL) ;
	(*l)->prox = novo;
	return 1;

}

int removerTarefaListaExecucao(LLista *l, int pidTarefa){
	printf("REMOVER TAREFA %d\n",pidTarefa);
	LLista p=(*l), q=NULL;
		printf("Primeira TAREFa:  %d - %s - %d\n",p->identificacaoTarefa,p->tarefa,p->pidTarefa);

	if(*l == NULL)
		return 0;
	if(((*l)->pidTarefa) == pidTarefa){
		printf("Tarefa para remover encontrada: %d - %s - %d\n",p->identificacaoTarefa,p->tarefa,p->pidTarefa);
		(*l)=(*l)->prox;
		free(p);
		return 1;
	}
	q=(*l)->prox;
	while(q!=NULL && pidTarefa != (q->pidTarefa)){
		p=q;
		q=q->prox;
	}
	if(q!=NULL){
		printf("Tarefa para remover encontrada: %d - %s - %d\n",q->identificacaoTarefa,q->tarefa,q->pidTarefa);
		p->prox=q->prox;
		free(q);
		return 1;
	}
	return 0;
}

// Inicializacao das listas ligadas
LLista tarefasExecucao = NULL;

int main(int argc, char *argv[]){

	int pid[10];
	int i = 0;

	fd_tarefasConcluidas = open("lista-tarefas-concluidas.txt", O_RDWR | O_CREAT, 0644);
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
			comandosRecebidos[n]='\0';
			printf("recebido: %s -- %d\n",comandosRecebidos,n);
			puts("FUNCAO PRINCIPAL");
			comunicacao(comandosRecebidos);
			strcpy(comandosRecebidos,"");
			printf("tempo-execucao %d\n",tempo_execucao);
			printf("tempo_inatividade definitivo: %d\n",tempo_inatividade);
			puts("DEBUGGGGGGGG\n\n\n\n");					
		}
		
	}
}

char *getSegundaPalavra(char q[80]){
	const char s[2] = " ";
	char *token;
	/* get the first token */
	token = strtok(q, s);
	/* get the second token*/
	token = strtok(NULL, s);
	return token;
}

void filhoPrincipalTerminou(int sig){
	printf("Funcao filhoPrincipalTerminou\n");
	int j;
	j=wait(NULL);
	printf("PID do filho que terminou %d\n",j);
	removerTarefaListaExecucao(&tarefasExecucao,j);
}

void comunicacao(char comandosRecebidos[80]){
	
	char *primeiraPalavra;
	int n;

	signal(SIGCHLD, filhoPrincipalTerminou);

	puts("FUNCAO COMUNICACAO ");
	printf("%d\n",strlen(comandosRecebidos));
	comandosRecebidos[strlen(comandosRecebidos)-1] = '\0';
	printf("Comandos recebidos %s\nEsta linha nao faz parte\n",comandosRecebidos);

	strcpy(buffer,comandosRecebidos);	
	
	primeiraPalavra = getPrimeiraPalavra(buffer);
	
	printf("%d\n",strlen(comandosRecebidos));
	printf("primeira palavra: %s\n",primeiraPalavra);
	printf("tamanho primeira palavra: %d\n",strlen(primeiraPalavra));

	if((strcmp(primeiraPalavra,"tempo-inatividade") == 0) || (strcmp(primeiraPalavra,"-i") == 0)){
		puts("SECÇAO TEMPO-INATIVIDADE");
		strcpy(buffer,comandosRecebidos);
		tempo_inatividade = atoi(getSegundaPalavra(buffer));
		printf("tempo_inatividade definitivo: %d\n",tempo_inatividade);
	}
	else if((strcmp(primeiraPalavra,"tempo-execucao") == 0) || (strcmp(primeiraPalavra,"-m") == 0)){
		puts("SECÇAO TEMPO-EXECUCAO");
		strcpy(buffer,comandosRecebidos);
		tempo_execucao = atoi(getSegundaPalavra(buffer));
		printf("tempo_execucao defenitivo: %d\n",tempo_execucao);
		
	}
	else if((strcmp(primeiraPalavra,"executar") == 0) || (strcmp(primeiraPalavra,"-e") == 0)){
		char temp[17];
		numeracaoTarefas++;
		printf("Numero da tarefa a ser executada: %d\n",numeracaoTarefas);
		
		/*	Envia para o argus o numero da tarefa a ser executada */
		sprintf(temp, "nova tarefa #%2d\n",numeracaoTarefas);
		write(fd_fifo2,temp,17);
		
		printf("%s\n",&(comandosRecebidos[strlen(primeiraPalavra)+1]));
		int pid = fork();

		if(pid == 0){
			printf("ESTE é o meu PID: %d\n\n",getpid());
			executarTarefa(&(comandosRecebidos[strlen(primeiraPalavra)+1]),numeracaoTarefas);
		}
		
		/* Coloca na lista de tarefas a serem executadas */
		inserirElementoLista(&tarefasExecucao,numeracaoTarefas,&(comandosRecebidos[strlen(primeiraPalavra)+1]), pid);

	}
	else if((strcmp(primeiraPalavra,"listar") == 0) || (strcmp(primeiraPalavra,"-l") == 0)){
		printf("LISTAR EM EXECUCAO\n");
		
		int cpid = fork();
		if(cpid==0){
			listarTeste(&tarefasExecucao);
		}
		wait(NULL);
		
	}
	else if((strcmp(primeiraPalavra,"terminar") == 0) || (strcmp(primeiraPalavra,"-t") == 0)){
		printf("terminar\n");
	}
	else if((strcmp(primeiraPalavra,"historico") == 0) || (strcmp(primeiraPalavra,"-r") == 0)){
		historico();
	}

	else if((strcmp(primeiraPalavra,"ajuda") == 0) || (strcmp(primeiraPalavra,"-h") == 0)){
		printf("ajuda\n");
	}

	// executar grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l


	
}

void historico(){
	char temp[80]="";
	int n, fd = open("lista-tarefas-concluidas.txt", O_RDONLY);
	while((n=readln(fd,temp,80)) > 0){
		write(1,temp,n);
		write(fd_fifo2,temp,n);
		puts(temp);
	}
	write(fd_fifo2,"FIM TRANSMISSAO",16);
	close(fd);
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
		printf("tempo: %d flag_execucaoTarefa: %d flag_tempo_inatividade: %d\n",tempo,flag_tempo_execucao,flag_tempo_inatividade);
		alarm(1);
	} 	

}

void filhoterminou(int sig){
	int pid; 
	contaPipesExecutados++;
	filhoMorreu=1;
}

void executarTarefa(char *comandosRecebidos, int numeracaoTarefas){
	int n, i;
	char comandos[20][20];
	int n_tokens = 0;
	char *comandos2[20][20];
	int n_pipes = 0;
	int fd[10][2], pid[10];
	int flag = 0,j;
	int flag_execucaoTarefa = 0;
	int status;

	contaPipesExecutados = 1;
	flag_tempo_execucao = 0;
	flag_tempo_inatividade = 0;
	filhoMorreu = 0;
	tempo = 0;


	// ativar o alarme com o tempo maximo de execucao de uma tarefa
	signal(SIGALRM, alarme);
	signal(SIGCHLD, filhoterminou);
	


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

	puts("FUNCAO EXECUTARTAREFA ");
	printf("tempo-execucao %d\n",tempo_execucao);
	printf("tempo-inatividade %d\n",tempo_inatividade);
	printf("contaPipesExecutados %d\n",contaPipesExecutados);
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

	
	alarm(1); // ativa o contador

	for(i=0;i<n_pipes+1 && !flag_tempo_execucao && !flag_tempo_inatividade;i++){
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
					_exit(-1);
				}
				close(fd[i][1]);
				printf("\nDebug %d\n",i);
				execvp(comandos2[i][0],comandos2[i]);
				sprintf(buffer,"Erro no execvp %d",i+1);
				perror(buffer);
				_exit(-1);
			}
			else {
				if(i==n_pipes){
					if(dup2(fd[i-1][0],0)==-1){
						sprintf(buffer,"Erro no dup2 %d",i-1);
						perror(buffer);
						_exit(-1);
					}
					close(fd[i-1][0]);
					//sleep(5);
					execvp(comandos2[i][0],comandos2[i]);
					sprintf(buffer,"Erro no execvp %d",i+1);
					perror(buffer);
					_exit(-1);
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
					sprintf(buffer,"Erro no execvp %d",i+1);
					perror(buffer);
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

			char tt[30];
			sprintf(tt,"flag_tempo_execucao %d\n",flag_tempo_execucao);
			write(1,tt,22);
			sprintf(tt,"flag_tempo_inatividade %d\n",flag_tempo_inatividade);
			write(1,tt,25);
			sprintf(tt,"contaPipesExecutados %d\n",contaPipesExecutados);
			write(1,tt,23);
			if(flag_tempo_execucao==1){
				kill(pid[i],SIGKILL);
				alarm(0);
			}
			else if(flag_tempo_inatividade == 1){
				kill(pid[i],SIGKILL);
				alarm(0);
			}
			/*
			waitpid(pid[i],&status,0);
			if(WEXITSTATUS(status) == -1 || !WIFEXITED(status)){
				flag_execucaoTarefa=1;
			}
			*/
		}
	}	
	
	alarm(0);
	puts(tarefa);
	
	// colocar no ficheiro de tarefas concluidas
	printf("flag_tempo_execucao %d\n", flag_tempo_execucao);
	if(flag_tempo_execucao){ // nao foi executada com exito
		// colocar no ficheiro de tarefas concluidas
		puts("NAO EXECUTADA devi ao tempo-execucao");
		sprintf(buffer,"#%d, max. execucao: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,19+strlen(tarefa));
		write(fd_tarefasConcluidas,"\n",1);
	}
	else if(flag_tempo_inatividade){
		// colocar no ficheiro de tarefas concluidas
		puts("NAO EXECUTADA devido ao tempo de execucao de um pipe");
		sprintf(buffer,"#%d, max. inactividade: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,23+strlen(tarefa));
		write(fd_tarefasConcluidas,"\n",1);
	}
	else{		// executada com exito
		sprintf(buffer,"#%d, concluida: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,15+strlen(tarefa));
		write(fd_tarefasConcluidas,"\n",1);
	}

	puts("COLOCADO NO FICHEIRO A TAREFA CONCLUIDA");

	write(1,"TAREFA TERMINADA COM SUCESSO\n",30);
	_exit(0);
	
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