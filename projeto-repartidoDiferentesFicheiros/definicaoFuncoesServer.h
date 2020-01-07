




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

int getPidTarefaExecucao(LLista *l, int numeroTarefaTerminar){
	LLista p = (*l);
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
		printf("TAREFA REMOVIDA COM SUCESSO\n");
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

void filhoPrincipalTerminou(int sig){
	printf("Funcao filhoPrincipalTerminou\n");
	int j;
	j=wait(NULL);
	printf("PID do filho que terminou %d\n",j);
	removerTarefaListaExecucao(&tarefasExecucao,j);
	//system("ps");
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
	int pid = wait(&status); 
	contaPipesExecutados++;
	if(WEXITSTATUS(status) == -1 || !WIFEXITED(status)){
		flag_erroExecucaoTarefa=1;
	}

	filhoMorreu=1;
}

void handle_sigTerminarTarefa(int sig){
	kill(pid, SIGKILL);
	flag_tarefa_terminada = 1;
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