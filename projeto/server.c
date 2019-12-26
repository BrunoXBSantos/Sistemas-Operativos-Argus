#include <sys/types.h> 	// open
#include <sys/stat.h> 	// open
#include <fcntl.h>  	// open

#include <unistd.h> 	// write & read
#include <stdio.h>		// sprintf
#include <string.h>		// sprintf






int main(int argc, char *argv[]){

	// Declaracao de variaveis 
	char *fifo = "fifo";  // FIFO file path
	int fd_log, fd_fifo, n;
	char buffer[80];
	char *file_log = "log.txt"; // ficheiro que guarda a comunicacao com o cliente
	char *aceite = "ACEITE";

	/* Abrir || Criar o ficheiro log.txt  */
	if((fd_log = open(file_log,O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1){
		sprintf(buffer,"Abrir ou criar ficheiro log.txt");
		perror(buffer);
		_exit(1);
	}

	/* Criar o ficheiro especial - FIFO FILE  creating the named file (FIFO) */
	if(mkfifo(fifo,0644) == -1){
		sprintf(buffer,"Criar o fifo file");
		perror(buffer);
		_exit(2);
	}

	/* Abrir a extremidade do canal unidirecional FIFO - Apenas para leitura*/
	if((fd_fifo = open(fifo,O_RDONLY)) == -1){
		sprintf(buffer, "Abrir o fifo-file para leitura - server");
		perror(buffer);
		_exit(3);
	}

	read(fd_fifo,buffer,30);
	if(strcmp(buffer,"LIGAR") == 0){
		printf("Pedido de conexao");
		close(fd_fifo);
		fd_fifo =  open(fifo,O_WRONLY);
		write(fd_fifo,aceite,strlen(aceite)+1);
	}

}