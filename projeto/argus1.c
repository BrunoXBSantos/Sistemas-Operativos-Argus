#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readln(int fildes, char *buf, int nbyte);
char * concatenaString(char *argv[], char *buffer, int total);
void comunicacao();
void getPrimeiraPalavra(char q[80], char primeiraPalavra[15]);
void historico();
void ajuda();

int fd1, fd2;
const char *prompt = "argus$ ";
char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";  // FIFO file path


int main(int argc, char *argv[]){
	int n;

	if(argc == 1){ // estabelece comunicacao
		comunicacao(); 
	}
	else{
		//buffer = concatenaString(&argv[1],&buffer,argc-2);
	}

	return 0;

}

void comunicacao(){
	char *sair="SAIR\n";
	char primeiraPalavra[15]="";
	char fraseLida[80] = "", buffer[80] = "";
	int n;
	int flag = 0;

	// abre os dois FIFOS
	fd1 = open(fifo1,O_WRONLY);
	fd2 = open(fifo2,O_RDONLY);


	do{

		write(1,prompt,strlen(prompt)+1);
		n=read(0,fraseLida,80);
		if(strstr(fraseLida,sair)!=NULL){
			flag = 1;
		}
		else{

			strcpy(buffer,fraseLida);	
			getPrimeiraPalavra(buffer,primeiraPalavra);


			printf("primeira palavra: %s -- %d\n",primeiraPalavra,strlen(primeiraPalavra));
			printf("frase escrita: %s  --  %d\n",fraseLida, n);

			if(strcmp(primeiraPalavra,"tempo-inactividade") == 0){
				printf("tempo-inactividade\n");
			}
			else if(strcmp(primeiraPalavra,"tempo-execucao") == 0){
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");;
			}
			else if(strcmp(primeiraPalavra,"executar") == 0){
				puts(fraseLida);
				write(fd1,fraseLida,n);
				strcpy(fraseLida,"");
				n=read(fd2,fraseLida,80);
				write(1,fraseLida,n);	
			}
			else if(strcmp(primeiraPalavra,"listar") == 0){
				printf("listar\n");
			}
			else if(strcmp(primeiraPalavra,"terminar") == 0){
				printf("terminar\n");
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
		strcpy(buffer,"");
		strcpy(primeiraPalavra,"");
		strcpy(fraseLida,"");

	}
	while(!flag);
	printf("Muito obrigado por utilizado o nosso servico\n");
}

// ver lista ajudas
void ajuda(){
	int n, fd;
	char temp[50];
	fd=open("ajuda.txt",O_RDWR);
	while((n=readln(fd,temp,50)) > 0){
		write(1,temp,n);
	}
	write(1,temp,n);
}

// ver lista de tarefas executadas
void historico(){
	char temp[80];
	int n, flag_historico = 0;
	while(!flag_historico){
		n=read(fd2,temp,80);
		if(strcmp(temp,"FIM TRANSMISSAO") == 0){
			printf("recebido do historico %s\n",temp);
			flag_historico = 1;
		}
		else{
			write(1,temp,n);
		}
	}
}

char * concatenaString(char *argv[], char *buffer, int total){
	int i;
	char *s = buffer;  // colocar em s o endereco de buffer 
	strcpy(buffer,argv[0]);
	for(i=1;i<total;i++){
		strcat(buffer," ");
		strcat(buffer,argv[i]);
	}
	strcat(buffer," \0");
	return s;
	
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

void getPrimeiraPalavra(char q[80], char primeiraPalavra[15]){
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