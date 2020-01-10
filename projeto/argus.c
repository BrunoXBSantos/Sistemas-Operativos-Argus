#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>


#define MAX_CARACTERES_COMANDO 500


// PROTOTIPO DE FUNCOES
void iniciaInterpretador();
void enviarComando(char fraseEnviar[MAX_CARACTERES_COMANDO]);
void historico();
void listar();
void ajuda();
int readln(int fildes, char *buf, int nbyte);
void getPrimeiraPalavra();


int fd1, fd2;
const char *prompt = "argus$ ";
char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";  // FIFO file path


int main(int argc, char *argv[]){
	int i;
	char fraseEnviar[MAX_CARACTERES_COMANDO];
	
	if((fd1 = open(fifo1,O_WRONLY))==-1){
		perror("Abertura do fifo1");
		_exit(1);
	}

	if((fd2 = open(fifo2,O_RDONLY))==-1){
		perror("Abertura do fifo2");
		_exit(2);
	}

	if(argc == 1){ 					// Inicia o interpretador de comandos
		iniciaInterpretador();    
	}	
	else{	
		memset(fraseEnviar,0,sizeof(MAX_CARACTERES_COMANDO));							
		for(i=1;i<argc;i++){
			strcat(fraseEnviar,argv[i]);
			strcat(fraseEnviar," ");
		}
		enviarComando(fraseEnviar);
	}

	close(fd1); close(fd2);    // Fecha os descritores

	return 0;

}


// Envia o comando colocado pelo utilizador inserido na linha de comnados
// diretamente  
void enviarComando(char fraseEnviar[MAX_CARACTERES_COMANDO]){
	int n;
	char buffer[MAX_CARACTERES_COMANDO];
	char primeiraPalavra[30];
	strcpy(buffer,fraseEnviar);	
	getPrimeiraPalavra(buffer,primeiraPalavra);

	if(strcmp(primeiraPalavra,"-i") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
	}
	else if(strcmp(primeiraPalavra,"-m") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
	}
	else if(strcmp(primeiraPalavra,"-e") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		memset(fraseEnviar,0,MAX_CARACTERES_COMANDO);
		n=read(fd2,fraseEnviar,80);
		write(1,fraseEnviar,n);	
	}
	else if(strcmp(primeiraPalavra,"-l") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		memset(fraseEnviar,0,MAX_CARACTERES_COMANDO);
		listar();
	}
	else if(strcmp(primeiraPalavra,"-t") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
	}
	else if(strcmp(primeiraPalavra,"-r") == 0){
		write(fd1,fraseEnviar,strlen(fraseEnviar));
		memset(fraseEnviar,0,MAX_CARACTERES_COMANDO);
		historico();
	}

	else if(strcmp(primeiraPalavra,"-h") == 0){
		write(1,"\n",2);
		ajuda();
		write(1,"\n",2);
	}


}

void iniciaInterpretador(){
	char *sair="SAIR\n";
	char primeiraPalavra[30];
	char fraseLida[MAX_CARACTERES_COMANDO], buffer[MAX_CARACTERES_COMANDO];
	int n=0;
	int flag = 0;
	fd1 = open(fifo1,O_WRONLY);
	fd2 = open(fifo2,O_RDONLY);

	do{
		memset(fraseLida,0,sizeof(fraseLida));
		memset(buffer,0,sizeof(buffer));
		memset(primeiraPalavra,0,sizeof(primeiraPalavra));
		write(1,prompt,strlen(prompt)+1);
		n=read(0,fraseLida,MAX_CARACTERES_COMANDO);
		fraseLida[n]='\0';
		if(strstr(fraseLida,sair)!=NULL){
			flag = 1;
		}
		else{

			strcpy(buffer,fraseLida);	
			getPrimeiraPalavra(buffer,primeiraPalavra);

			if(strcmp(primeiraPalavra,"tempo-inatividade") == 0){
				write(fd1,fraseLida,n);
				write(1,"\n",2);
			}
			else if(strcmp(primeiraPalavra,"tempo-execucao") == 0){
				write(fd1,fraseLida,n);
				write(1,"\n",2);
			}
			else if(strcmp(primeiraPalavra,"executar") == 0){
				write(fd1,fraseLida,n);
				memset(fraseLida,0,sizeof(fraseLida));
				n=read(fd2,fraseLida,80);
				write(1,fraseLida,n);
				write(1,"\n",2);	
			}
			else if(strcmp(primeiraPalavra,"listar") == 0){
				write(fd1,fraseLida,n);
				memset(fraseLida,0,sizeof(fraseLida));
				listar();
			}
			else if(strcmp(primeiraPalavra,"terminar") == 0){
				write(fd1,fraseLida,n);
				write(1,"\n",2);
			}
			else if(strcmp(primeiraPalavra,"historico") == 0){
				write(fd1,fraseLida,n);
				memset(fraseLida,0,sizeof(fraseLida));
				historico();
			}

			else if(strcmp(primeiraPalavra,"ajuda") == 0){
				write(1,"\n",2);
				ajuda();
				write(1,"\n",2);
			}
	
		}

	}
	while(!flag);
	write(1,"\nMuito obrigado por utilizado o nosso servico\n",46);
}

// ver lista ajudas
void ajuda(){
	int n, fd;
	char temp[MAX_CARACTERES_COMANDO];
	fd=open("ajuda.txt",O_RDWR);
	while((n=readln(fd,temp,MAX_CARACTERES_COMANDO)) > 0){
		write(1,temp,n);
	}
	close(fd);
}

// ver lista de tarefas executadas
void historico(){
	char temp[1024];
	char buffer[1024];
	memset(temp,0,sizeof(temp));
	memset(buffer,0,sizeof(buffer));
	int n, flag_historico = 0;
	write(1,"\n",2);
	while(!flag_historico){
		n=read(fd2,temp,1024);
		if(strstr(temp,"FIM") != NULL){
			write(1,temp,n);
			flag_historico = 1;
		}
		else{
			write(1,temp,n);
		}
	}
	write(1,"\n",2);
}
/*

*/

void listar(){
	int cont = 0;
	char temp[PIPE_BUF];
	char buffer[PIPE_BUF];
	memset(temp,0,sizeof(temp));
	memset(buffer,0,sizeof(buffer));
	int n, flag_listar=0;
	write(1,"\n",2);
	while(!flag_listar){
		if(cont!=0){
			if(strstr(temp,"FIM ") != NULL){
				flag_listar=1;
			}
		}
		memset(temp,0,sizeof(temp));
		n=read(fd2,temp,PIPE_BUF);
		if(strstr(temp,"FIM ") != NULL){
			write(1,temp,n);
			flag_listar=1;
		}
		else{
			write(1,temp,n);
		}
		cont++;
	}
	write(1,"\n",2);
}

void getPrimeiraPalavra(char q[MAX_CARACTERES_COMANDO], char primeiraPalavra[30]){
	int i,j, flag = 0;
	for(i=j=0;i<strlen(q) && !flag;i++){
		if(q[i]==' ' || q[i]=='\n'){
			flag=1;
		}
		else{
			primeiraPalavra[j++]=q[i];
		}
	}
	primeiraPalavra[j]='\0';

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