#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "prototipoFuncoesClient.h"
#include "definicaoFuncoesClient.h"



int fd1, fd2;
const char *prompt = "argus$ ";
char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";  // FIFO file path


int main(int argc, char *argv[]){
	int n,i;
	char fraseEnviar[80] = "";
	printf("DEBUGGG\n");
	char buffer[1024];
	
	fd1 = open(fifo1,O_WRONLY);
	fd2 = open(fifo2,O_RDONLY);

	if(argc == 1){ // estabelece comunicacao
		printf("INICIA inicia Interpretador\n");
		iniciaInterpretador(); 
	}
	else{
		printf("INICIA Executar UMA VEZ\n");
		for(i=1;i<argc;i++){
			strcat(fraseEnviar,argv[i]);
			strcat(fraseEnviar," ");
		}
		
		printf("fraseEnviar: %s - %d\n",fraseEnviar, strlen(fraseEnviar));	

		enviarComando(fraseEnviar);
	}

	close(fd1); close(fd2);

	return 0;

}

void enviarComando(char fraseEnviar[80]){
	int n;
	char buffer[1024]="";
	char primeiraPalavra[30]="";
	strcpy(buffer,fraseEnviar);	
	getPrimeiraPalavra(buffer,primeiraPalavra);

	printf("primeira palavra: %s -- %d\n",primeiraPalavra,strlen(primeiraPalavra));

	if(strcmp(primeiraPalavra,"-i") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
	}
	else if(strcmp(primeiraPalavra,"-m") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
	}
	else if(strcmp(primeiraPalavra,"-e") == 0){
		printf("Frase enviada: %s -- %d\n",fraseEnviar,strlen(fraseEnviar));
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		strcpy(fraseEnviar,"");
		n=read(fd2,fraseEnviar,80);
		write(1,fraseEnviar,n);	
	}
	else if(strcmp(primeiraPalavra,"-l") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		strcpy(fraseEnviar,"");
		listar();
	}
	else if(strcmp(primeiraPalavra,"-t") == 0){
		printf("terminar\n");
	}
	else if(strcmp(primeiraPalavra,"-r") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		strcpy(fraseEnviar,"");
		historico();
	}

	else if(strcmp(primeiraPalavra,"ajuda") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		strcpy(fraseEnviar,"");
		ajuda();
	}


}

void iniciaInterpretador(){
	char *sair="SAIR\n";
	char primeiraPalavra[30]="";
	char fraseLida[80] = "", buffer[80] = "";
	int n=0;
	int flag = 0;
	fd1 = open(fifo1,O_WRONLY);
	fd2 = open(fifo2,O_RDONLY);

	do{
		strcpy(fraseLida,"");
		strcpy(buffer,"");
		strcpy(primeiraPalavra,"");
		write(1,prompt,strlen(prompt)+1);
		n=read(0,fraseLida,80);
		if(strstr(fraseLida,sair)!=NULL){
			flag = 1;
		}
		else{

			strcpy(buffer,fraseLida);	
			getPrimeiraPalavra(buffer,primeiraPalavra);

			printf("primeira palavra: %s -- %d\n",primeiraPalavra,strlen(primeiraPalavra));
			fraseLida[n]='\0';
			printf("frase escrita: %s  --  %d\n",fraseLida, n);

			if(strcmp(primeiraPalavra,"tempo-inatividade") == 0){
				write(fd1,fraseLida,n);
			}
			else if(strcmp(primeiraPalavra,"tempo-execucao") == 0){
				write(fd1,fraseLida,n);
			}
			else if(strcmp(primeiraPalavra,"executar") == 0){
				printf("Frase enviada: %s -- %d\n",fraseLida,n);
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");
				n=read(fd2,fraseLida,80);
				write(1,fraseLida,n);	
			}
			else if(strcmp(primeiraPalavra,"listar") == 0){
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");
				listar();
			}
			else if(strcmp(primeiraPalavra,"terminar") == 0){
				printf("terminar tarefa\n");
				write(fd1,fraseLida,n);
			}
			else if(strcmp(primeiraPalavra,"historico") == 0){
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");
				historico();
			}

			else if(strcmp(primeiraPalavra,"ajuda") == 0){
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");
				ajuda();
			}
	
		}

	}
	while(!flag);
	printf("Muito obrigado por utilizado o nosso servico\n");
}


