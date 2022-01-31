/*
	Autor: Paulo Lopes do Nascimento
	Data: 31/01/2022
	Local: Contagem - MG
	Programa de introdu��o � comunica��o de redes TCP usando PuTTY e transmiss�o de dados Raw e SSH utilizando sockets.
	Execute o programa, abra o PuTTY e utilize seu IP (exemplo: 192.168.1.5) e a porta especificada no programa, neste caso � 54000, clique em Open, se n�o der certo, troque a op��o de SSH para Raw.
	Tudo dando certo, abrir� a janela do PuTTY onde voc� poder� mandar as mensagens e ir� recebe-las de volta.
	*/
// Incluindo biblioteca de entrada e saida de dados
#include<iostream>
// Incluindo biblioteca TCP/IP do Windows Sockets 2, que realiza as conex�es no Windows 10
#include<WS2tcpip.h>
// Indica para o montador (linker) para que adicione a biblioteca ws2_32 para a lista de depend�ncias
#pragma comment (lib, "ws2_32.lib")


void main() {
	// Inicializar um winsock
	/* A estrutura WSADATA cont�m informa��es sobre a implementa��o de soquetes de Windows.*/
	WSADATA wsData;
	/* Faz uma solicita��o para a vers�o 2,2 do Winsock no sistema e define a vers�o passada como a vers�o mais recente do suporte a soquetes de Windows que o chamador pode usar.*/
	/* MAKEWORD concatena os bits low e high de 8 bits passados em uma palavra de 16 bits que ser� utilizada para inicializa��o do socket*/
	WORD ver = MAKEWORD(2, 2);
	/* A fun��o WSAStartup deve ser a primeira fun��o dos sockets Windows a ser chamada por uma aplica��o ou DLL. 
	Ela permite que uma aplica��o ou DLL especifique a vers�o do Windows Sockets desejada e recebe detalhes da implementa��o espec�fica do Windows Sockets em quest�o. 
	A aplica��o ou DLL s� pode realizar chamadas subsequentes de fun��es do Windows Sockets depois de chamar, com sucesso a fun��o WSAStartup.*/
	int wsOk = WSAStartup(ver, &wsData);
	/* Se o retorno da WSAStartup for diferente de zero, significa que algum erro aconteceu */
	if (wsOk != 0) {
		std::cerr << "N�o foi poss�vel inicializar o winsock. Encerrando..." << std::endl;
		return;
	}
	// Criar o socket
	/* Cria o objeto de socket que ir� escutar, do tipo SOCKET que � um HANDLER, um index para tipos e estruturas que est�o no kernel */
	/* A fun��o socket() faz com que o descrevedor do socket e qualquer recurso relacionado seja alocado e vinculado a um provedor espec�fico de servi�o de transporte. */
	/* Significado dos par�metros : 
	AF_INET = Especificador da fam�lia de endere�os que o socket pode se comunicar, neste caso o AF_INET comum se comunica com ipv4, senod AF_INET6 a ver�o do Ipv6.
	SOCK_STREAM = Especificador de protocolo baseado em conex�o. A conex�o � estabelecida e ambas as partes conversam at� que a conex�o seja encerrada por um dos lados, ou por um erro.
	0 = Flag para especificar o protocolo que ser� usado (ICMP, TCP, UDP etc), passamos 0 pois n�o desejamos fixar o protocolo, e assim deixamos que seja decidido automaticamente.
	*/
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	/* Verifica se o retorno � inv�lido */
	if (listening == INVALID_SOCKET) {
		std::cerr << "N�o foi poss�vel criar o socket! Encerrando..." << std::endl;
		return;
	}

	// Conectar um endere�o IP e uma porta � um socket
	/* Estrutura que lida com endere�os de rede*/
	sockaddr_in hint;
	/* Vari�veis definidas:
	sin_family = Fam�lia dos endere�os para o endere�o de transporte. "Always AF_INET em socketaddr_in
	sin_port = N�mero da porta de transporte de protocolo
	sin_addr = Estrutura IN_ADDR que especifica endere�o de transporte ipv4.
		S_un = Uma uni�o que cont�m representa��es diferentes de um endere�o ipv4.
			S_addr = Aceita conex�o de todos os IPs dispon�veis.
	"*/
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Poderia usar inet_pton...
	/* Fun��o que associa um endere�o local � um socket, sendo os par�metros:
	listening = Socket livre.
	sockaddr* &hint = Ponteiro para uma estrutura sockaddr do endere�o local (&hint) que deve ser atribu�do ao socket livre.
	sizeof(hint) = O tamanho em bytes do valor que est� sendo apontado.
	*/
	bind(listening, (sockaddr*)&hint, sizeof(hint));
											
	// Dizer ao winsock que o socket est� ativo, escutando
	/* Fun��o que coloca o socket num estado no qual est� aguardando por uma conex�o, sendo os par�mtros:
	listening = Socket ligado � um endere�o, mas n�o em conex�o.
	SOMAXCONN = Maior n�mero de conex�es permitidas simultaneamente, o provedor do servi�o respons�vel pelo socket definir� o mairo valor.
	*/
	listen(listening, SOMAXCONN);
	// Aguardar pela conex�o do cliente
	/* Criando um segundo endere�o de rede, que ser� o do "cliente" */
	sockaddr_in client;
	/* Tamanho da estrutura sockaddr do cliente*/
	int clientsize = sizeof(client);
	/* Cria��o do socket do cliente
	Fun��o accept permite uma tentativa de conex�o no socket, com par�metros:
	listening = Socket que foi coloco em estado de aguardo pela fun��o listen(), a conex�o � feita com o socket retornado pela fun��o accept().
	sockaddr* &client = Ponteiro opcional que aponta para um buffer que recebe o endere�o da entidade que est� conectando.
	(Formato exato do par�metro � definido pela fam�lia do endere�o estabelecido quando o socket da sockaddr foi criado).
	&clientsize = Ponteiro opcional para um inteiro que cont�m o tamanho da estrutura apontado pelo par�metro anterior.
	*/	
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientsize);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "N�o foi poss�vel criar o socket de conex�o! Encerrando..." << std::endl;
		return;
	}
	/* Criando buffers de chars para:
	1� = Nome do cliente remoto. 
	2� = Service (i.e porta) que o client est� conectado.
	NI_MAXHOST = 
	*/
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	/* Preenche um bloco de mem�ria com zeros, sendo os par�metros:
	host, service = Ponteiros para o endere�o do in�cio do bloco de mem�ria que deve ser preenchido com zeros.
	NI_MAXHOST, NI_MAXSERV = Tamanho do bloco de mem�ria a ser preenchido com zeros, em bytes.
	*/
	SecureZeroMemory(host, NI_MAXHOST);
	SecureZeroMemory(service, NI_MAXSERV);
	/* Fun��o que buscar� resolu��o de um endere�o para um nome de host ANSI e um n�mero de porta para o nome do servi�o ANSI, com par�metros:
	(sockaddr*)&client = Ponteiro para o endere�o de uma estrutura de rede que cont�m o endere�o e porta do socket.
	sizeof(client) = O tamanho em bytes da estrutura apontada pelo par�metro acima.
	host = Ponteiro para uma string ANSI que ir� receber o nome do host.
	NI_MAXHOST = O tamanho em bytes do buffer apontado pelo par�metro anterior.
	service = Ponteiro para uma string ANSI que ir� receber o nome do servi�o.
	NI_MAXSERV = O tamanho em bytes do buffer apontado pelo par�metro anterior.
	0 = Valor usado para customizar o processamento da fun��o. (Ver doc.1 no fim do arquivo)
	*/
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		/* Se foi poss�vel, apresenta mensagem com os dados, notificando que a conex�o foi um sucesso*/
		std::cout << host << " conectou-se � porta " << service << std::endl;
	} else {
		/* Fun��o que converte um endere�o ipv4 ou ipv6 em uma string em formato padr�o "4bytes.4bytes.4bytes.4bytes"
		Vers�o ANSI.
		*/
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " conetou-se � porta " << ntohs(client.sin_port) << std::endl;
	}
	// Fechar o socket que est� escutando
	/* Fun��o que fecha um socket existente */
	closesocket(listening);
	// Loop com enquanto-> aceitar e dar um echo na mensagem de volta para o cliente
	/* Buffer que ser� usado nas mensagens*/
	char buf[4096];

	while (true) {
		/* Fun��o que preenche com zeros o bloco de mem�ria buf*/
		SecureZeroMemory(buf, 4096);
		// Aguardar o cliente mandar dados;
		/* Fun��o que recebe dados de um socket conectado ou um socket sem conex�o por�m com endere�o
		clientsocket = Socket conectado.
		buf = Ponteiro para o buffer que ir� receber os dados.
		4096 = Tamanho em bytes do buffer apontado pelo ponteiro anterior.
		0 = Conjunto de flags que influencia o comportamento desta fun��o. (Ver doc.2)
		*/
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		/* Verifica se o retorno de recv � um erro e apresenta mensagem*/
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Erro em recv(). Encerrando ..." << std::endl;
			break;
		}
		/* Verifica se o n�mero de bytes recebidos � 0, se for o cliente se desconecetou*/
		if (bytesReceived == 0) {
			std::cout << "Client disconnected " << std::endl;
			break;
		}
		// Mandar mensagem de volta para o cliente
		/* Fun��o que envia dados em um socket conectado, com par�metros:
		clientSocket = Socket conectado.
		buf = Ponteiro para o buffer contendo os dados que ser�o transmitidos.
		bytesReceived+1 = O tamanho em bytes dos dados no buffer apontados pelo par�metro anterior.
		(Quando recebemos uma mensagem, n�o recebemos o \0, da� aumentamos em 1 o tamanho).
		0 = Conjunto de flags que influencia o comportamento desta fun��o. (Ver doc.3)
		*/
		send(clientSocket, buf, bytesReceived+1,0);
	}
	// Fechar o socket
	closesocket(clientSocket);
	// Limpar o winsock
	WSACleanup();
}

/* Documenta��es utilizadas no c�digo:

doc.1 = https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getnameinfo
doc.2 = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
doc.3 = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
Adicionais:
V�deo base e passo a passo, cr�ditos a Sloan Kelly: https://www.youtube.com/watch?v=WDn-htpBlnU
sockaddr_in = https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in
bind = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind
listen = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen
accept = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
SecureZeroMemory = https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85)
closesocket = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket
inet_ntop= https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop
*/