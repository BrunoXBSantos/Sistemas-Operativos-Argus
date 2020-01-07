

typedef struct idtarefa{
	int identificacaoTarefa;
	char tarefa[80];
	int pidTarefa;
	struct idtarefa *prox;
} *LLista;

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
int getPidTarefaExecucao(LLista *l, int numeroTarefaTerminar);