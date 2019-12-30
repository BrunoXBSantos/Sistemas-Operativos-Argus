#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readln(int fildes, char *buf, int nbyte);
char * concatenaString(char *argv[], char *buffer, int total);
void comunicacao();

int fd1, fd2;
const char *prompt = "argus$ ";
char *fifo1 = "/tmp/fifo1";  // FIFO file path
char *fifo2 = "/tmp/fifo2";  // FIFO file path


int main(int argc, char *argv[]){

	
	
	int n;

	
	char resposta[30], tarefa[30] = "";
	int flag = 0;
	
	char *iniciarInterpretador="1";  // 1 -> inicia interpretador de comandos
	char buffer[20]="";

	 
	
	 
	if(argc == 1){
		comunicacao();
	}
	else{
		//buffer = concatenaString(&argv[1],&buffer,argc-2);
		printf("%s\n",buffer);
		write(fd1,iniciarInterpretador,strlen(iniciarInterpretador)+1);
	}

	return 0;

}

void comunicacao(){
	char *sair="SAIR\n";
	char buffer[80] = "";
	int n;
	int flag = 0;
	fd1 = open(fifo1,O_WRONLY);
	fd2 = open(fifo2,O_RDONLY);
	do{
		write(1,prompt,strlen(prompt)+1);
		n=read(0,buffer,80);
		if(strstr(buffer,sair)!=NULL){
			flag = 1;
		}
		else{
			printf("numero lido: %d\n",n);
			puts(buffer);
			
			write(fd1,buffer,n);

			strcpy(buffer,"");
			n=read(fd2,buffer,80);
			write(1,buffer,n);
		}

	}
	while(!flag);
	printf("Muito obrigado por utilizado o nosso servico\n");
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