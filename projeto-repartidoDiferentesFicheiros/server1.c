#include <sys/types.h> 	// open
#include <sys/stat.h> 	// open
#include <fcntl.h>  	// open

#include <unistd.h> 	// write & read
#include <stdio.h>		// sprintf
#include <string.h>		// sprintf

#include <sys/wait.h>
#include <stdlib.h>

#include <signal.h> // sigaction(), sigsuspend(), sig*()

#include "prototipoFuncoesServer.h"
#include "definicaoFuncoesServer.h"
#include "definicaoFuncoesServer-Client.h"




char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";
int fd_log, fd_fifo1, fd_fifo2;
char buffer[80];
int numeracaoTarefas = 0;
int *pt_numeracaoTarefas = &numeracaoTarefas;
int pid; // para cada tarefa guardar o seu pid
int status;

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

	int i,n = 0;

	fd_tarefasConcluidas = open("lista-tarefas-concluidas.txt", O_RDWR | O_CREAT, 0644);
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
			//system("ps");			
		}
		
	}
}





void comunicacao(char comandosRecebidos[80]){
	
	char *primeiraPalavra;
	int n, numeroTarefaTerminar, pidTarefaTerminar;

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
	
		listarTeste(&tarefasExecucao);
	
		
	}
	else if((strcmp(primeiraPalavra,"terminar") == 0) || (strcmp(primeiraPalavra,"-t") == 0)){
		puts("SECÇAO Terminar tarefa");
		strcpy(buffer,comandosRecebidos);
		numeroTarefaTerminar = atoi(getSegundaPalavra(buffer));
		pidTarefaTerminar = getPidTarefaExecucao(&tarefasExecucao, numeroTarefaTerminar);
		printf("PID DA TAREFA A TERMINAR: %d\n",pidTarefaTerminar);
		kill(pidTarefaTerminar,SIGUSR1);
	}
	else if((strcmp(primeiraPalavra,"historico") == 0) || (strcmp(primeiraPalavra,"-r") == 0)){
		historico();
	}

	else if((strcmp(primeiraPalavra,"ajuda") == 0) || (strcmp(primeiraPalavra,"-h") == 0)){
		printf("ajuda\n");
	}

	// executar grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l


	
}




void executarTarefa(char *comandosRecebidos, int numeracaoTarefas){
	int n, i;
	char comandos[20][20];
	int n_tokens = 0;
	char *comandos2[20][20];
	int n_pipes = 0;
	int fd[10][2];
	int flag = 0,j;
	int flag_execucaoTarefa = 0;
	int status;

	contaPipesExecutados = 1;
	flag_tempo_execucao = 0;
	flag_tempo_inatividade = 0;
	flag_tarefa_terminada=0;
	filhoMorreu = 0;
	tempo = 0;


	// ativar o alarme com o tempo maximo de execucao de uma tarefa
	signal(SIGALRM, alarme);
	signal(SIGCHLD, filhoterminou);
	signal(SIGUSR1, handle_sigTerminarTarefa);
	
	// system("ps");

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

	for(i=0;i<n_pipes+1 && !flag_tempo_execucao && !flag_tempo_inatividade && !flag_tarefa_terminada && !flag_erroExecucaoTarefa ;i++){
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
	else if(flag_tarefa_terminada){
		// colocar no ficheiro de tarefas concluidas
		puts("Tarefa terminada a meio");
		sprintf(buffer,"#%d, terminada: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,15+strlen(tarefa));
		write(fd_tarefasConcluidas,"\n",1);
	}
	else if(flag_erroExecucaoTarefa){
		// colocar no ficheiro de tarefas concluidas
		puts("Erro na tarefa");
		sprintf(buffer,"#%d, Erro ao executar: %s",numeracaoTarefas,tarefa);
		write(fd_tarefasConcluidas,buffer,22+strlen(tarefa));
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





 

	