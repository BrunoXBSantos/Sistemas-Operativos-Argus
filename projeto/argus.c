#include <unistd.h> // read
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readln(int fildes, char *buf, int nbyte);


int main(int argc, char *argv[]){
	char *myfifo = "fifo";  // FIFO file path
	int fd;

	char *sair="SAIR";
	char *ligar="LIGAR";
	char resposta[30], tarefa[30] = "";
	int flag = 0;
	char *prompt = "argus$ ";
	
	 
	 
	printf("***** Bem vindo ao programa Argus *****\n");
	printf("***** A conectar ao servidor .... *****\n");
	
	

	while(!flag){
		write(1,prompt,strlen(prompt)+1);
		readln(0,tarefa,30);
	}
	

	printf("\n%s\n",tarefa);

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