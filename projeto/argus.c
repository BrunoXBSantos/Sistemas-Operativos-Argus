#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readln(int fildes, char *buf, int nbyte);
char * concatenaString(char *argv[], char *buffer, int total);
void comunicacao();

int fd;
const char *prompt = "argus$ ";
char *fifo = "/tmp/fifo";  // FIFO file path


int main(int argc, char *argv[]){

	
	
	int n;

	
	char resposta[30], tarefa[30] = "";
	int flag = 0;
	
	char *iniciarInterpretador="1";  // 1 -> inicia interpretador de comandos
	char buffer[20]="";

	 
	
	 
	if(argc == 1){
		fd = open(fifo,O_WRONLY);
		write(fd,iniciarInterpretador,strlen(iniciarInterpretador)+1);
		close(fd);
		fd=open(fifo,O_RDONLY);
		read(fd, buffer, 80);
		if(strcmp(buffer, iniciarInterpretador) == 0){
			close(fd);
			comunicacao();
		}
	}
	else{
		//buffer = concatenaString(&argv[1],&buffer,argc-2);
		printf("%s\n",buffer);
		write(fd,iniciarInterpretador,strlen(iniciarInterpretador)+1);
	}

	

}

void comunicacao(){
	char *sair="SAIR";
	char buffer[80] = "";
	int n;
	do{
		write(1,prompt,strlen(prompt)+1);
		n=read(0,buffer,80);
		puts(buffer);
		printf("%d\n",n);
		fd = open(fifo,O_WRONLY);
		write(fd,buffer,n);
		close(fd);

		
		

	}
	while(strcmp(buffer,sair)!=0);
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