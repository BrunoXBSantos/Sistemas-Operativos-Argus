/* Fazer pedido para conectar ao server */
	fd = open(myfifo,O_WRONLY);
	write(fd, ligar, strlen(ligar)+1);
	close(fd);

	/* Receber pedido da conexao com o server */
	fd = open(myfifo,O_RDONLY);
	read(fd, resposta, 31);
	close(fd);

	if(strcmp(resposta,"ACEITE") == 0){
		printf("Ligado com sucesso ao server\n");
	}