/*
	Autor: Paulo Lopes do Nascimento
	Data: 31/01/2022
	Local: Contagem - MG
	Programa de introdução à comunicação de redes TCP usando PuTTY e transmissão de dados Raw e SSH utilizando sockets.
	Execute o programa, abra o PuTTY e utilize seu IP (exemplo: 192.168.1.5) e a porta especificada no programa, neste caso é 54000, clique em Open, se não der certo, troque a opção de SSH para Raw.
	Tudo dando certo, abrirá a janela do PuTTY onde você poderá mandar as mensagens e irá recebe-las de volta.
	*/
// Incluindo biblioteca de entrada e saida de dados
#include<iostream>
// Incluindo biblioteca TCP/IP do Windows Sockets 2, que realiza as conexões no Windows 10
#include<WS2tcpip.h>
// Indica para o montador (linker) para que adicione a biblioteca ws2_32 para a lista de dependências
#pragma comment (lib, "ws2_32.lib")


void main() {
	// Inicializar um winsock
	/* A estrutura WSADATA contém informações sobre a implementação de soquetes de Windows.*/
	WSADATA wsData;
	/* Faz uma solicitação para a versão 2,2 do Winsock no sistema e define a versão passada como a versão mais recente do suporte a soquetes de Windows que o chamador pode usar.*/
	/* MAKEWORD concatena os bits low e high de 8 bits passados em uma palavra de 16 bits que será utilizada para inicialização do socket*/
	WORD ver = MAKEWORD(2, 2);
	/* A função WSAStartup deve ser a primeira função dos sockets Windows a ser chamada por uma aplicação ou DLL. 
	Ela permite que uma aplicação ou DLL especifique a versão do Windows Sockets desejada e recebe detalhes da implementação específica do Windows Sockets em questão. 
	A aplicação ou DLL só pode realizar chamadas subsequentes de funções do Windows Sockets depois de chamar, com sucesso a função WSAStartup.*/
	int wsOk = WSAStartup(ver, &wsData);
	/* Se o retorno da WSAStartup for diferente de zero, significa que algum erro aconteceu */
	if (wsOk != 0) {
		std::cerr << "Não foi possível inicializar o winsock. Encerrando..." << std::endl;
		return;
	}
	// Criar o socket
	/* Cria o objeto de socket que irá escutar, do tipo SOCKET que é um HANDLER, um index para tipos e estruturas que estão no kernel */
	/* A função socket() faz com que o descrevedor do socket e qualquer recurso relacionado seja alocado e vinculado a um provedor específico de serviço de transporte. */
	/* Significado dos parâmetros : 
	AF_INET = Especificador da família de endereços que o socket pode se comunicar, neste caso o AF_INET comum se comunica com ipv4, senod AF_INET6 a verão do Ipv6.
	SOCK_STREAM = Especificador de protocolo baseado em conexão. A conexão é estabelecida e ambas as partes conversam até que a conexão seja encerrada por um dos lados, ou por um erro.
	0 = Flag para especificar o protocolo que será usado (ICMP, TCP, UDP etc), passamos 0 pois não desejamos fixar o protocolo, e assim deixamos que seja decidido automaticamente.
	*/
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	/* Verifica se o retorno é inválido */
	if (listening == INVALID_SOCKET) {
		std::cerr << "Não foi possível criar o socket! Encerrando..." << std::endl;
		return;
	}

	// Conectar um endereço IP e uma porta à um socket
	/* Estrutura que lida com endereços de rede*/
	sockaddr_in hint;
	/* Variáveis definidas:
	sin_family = Família dos endereços para o endereço de transporte. "Always AF_INET em socketaddr_in
	sin_port = Número da porta de transporte de protocolo
	sin_addr = Estrutura IN_ADDR que especifica endereço de transporte ipv4.
		S_un = Uma união que contém representações diferentes de um endereço ipv4.
			S_addr = Aceita conexão de todos os IPs disponíveis.
	"*/
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Poderia usar inet_pton...
	/* Função que associa um endereço local à um socket, sendo os parâmetros:
	listening = Socket livre.
	sockaddr* &hint = Ponteiro para uma estrutura sockaddr do endereço local (&hint) que deve ser atribuído ao socket livre.
	sizeof(hint) = O tamanho em bytes do valor que está sendo apontado.
	*/
	bind(listening, (sockaddr*)&hint, sizeof(hint));
											
	// Dizer ao winsock que o socket está ativo, escutando
	/* Função que coloca o socket num estado no qual está aguardando por uma conexão, sendo os parâmtros:
	listening = Socket ligado à um endereço, mas não em conexão.
	SOMAXCONN = Maior número de conexões permitidas simultaneamente, o provedor do serviço responsável pelo socket definirá o mairo valor.
	*/
	listen(listening, SOMAXCONN);
	// Aguardar pela conexão do cliente
	/* Criando um segundo endereço de rede, que será o do "cliente" */
	sockaddr_in client;
	/* Tamanho da estrutura sockaddr do cliente*/
	int clientsize = sizeof(client);
	/* Criação do socket do cliente
	Função accept permite uma tentativa de conexão no socket, com parâmetros:
	listening = Socket que foi coloco em estado de aguardo pela função listen(), a conexão é feita com o socket retornado pela função accept().
	sockaddr* &client = Ponteiro opcional que aponta para um buffer que recebe o endereço da entidade que está conectando.
	(Formato exato do parâmetro é definido pela família do endereço estabelecido quando o socket da sockaddr foi criado).
	&clientsize = Ponteiro opcional para um inteiro que contém o tamanho da estrutura apontado pelo parâmetro anterior.
	*/	
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientsize);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Não foi possível criar o socket de conexão! Encerrando..." << std::endl;
		return;
	}
	/* Criando buffers de chars para:
	1º = Nome do cliente remoto. 
	2º = Service (i.e porta) que o client está conectado.
	NI_MAXHOST = 
	*/
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	/* Preenche um bloco de memória com zeros, sendo os parâmetros:
	host, service = Ponteiros para o endereço do início do bloco de memória que deve ser preenchido com zeros.
	NI_MAXHOST, NI_MAXSERV = Tamanho do bloco de memória a ser preenchido com zeros, em bytes.
	*/
	SecureZeroMemory(host, NI_MAXHOST);
	SecureZeroMemory(service, NI_MAXSERV);
	/* Função que buscará resolução de um endereço para um nome de host ANSI e um número de porta para o nome do serviço ANSI, com parâmetros:
	(sockaddr*)&client = Ponteiro para o endereço de uma estrutura de rede que contém o endereço e porta do socket.
	sizeof(client) = O tamanho em bytes da estrutura apontada pelo parâmetro acima.
	host = Ponteiro para uma string ANSI que irá receber o nome do host.
	NI_MAXHOST = O tamanho em bytes do buffer apontado pelo parâmetro anterior.
	service = Ponteiro para uma string ANSI que irá receber o nome do serviço.
	NI_MAXSERV = O tamanho em bytes do buffer apontado pelo parâmetro anterior.
	0 = Valor usado para customizar o processamento da função. (Ver doc.1 no fim do arquivo)
	*/
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		/* Se foi possível, apresenta mensagem com os dados, notificando que a conexão foi um sucesso*/
		std::cout << host << " conectou-se à porta " << service << std::endl;
	} else {
		/* Função que converte um endereço ipv4 ou ipv6 em uma string em formato padrão "4bytes.4bytes.4bytes.4bytes"
		Versão ANSI.
		*/
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " conetou-se à porta " << ntohs(client.sin_port) << std::endl;
	}
	// Fechar o socket que está escutando
	/* Função que fecha um socket existente */
	closesocket(listening);
	// Loop com enquanto-> aceitar e dar um echo na mensagem de volta para o cliente
	/* Buffer que será usado nas mensagens*/
	char buf[4096];

	while (true) {
		/* Função que preenche com zeros o bloco de memória buf*/
		SecureZeroMemory(buf, 4096);
		// Aguardar o cliente mandar dados;
		/* Função que recebe dados de um socket conectado ou um socket sem conexão porém com endereço
		clientsocket = Socket conectado.
		buf = Ponteiro para o buffer que irá receber os dados.
		4096 = Tamanho em bytes do buffer apontado pelo ponteiro anterior.
		0 = Conjunto de flags que influencia o comportamento desta função. (Ver doc.2)
		*/
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		/* Verifica se o retorno de recv é um erro e apresenta mensagem*/
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Erro em recv(). Encerrando ..." << std::endl;
			break;
		}
		/* Verifica se o número de bytes recebidos é 0, se for o cliente se desconecetou*/
		if (bytesReceived == 0) {
			std::cout << "Client disconnected " << std::endl;
			break;
		}
		// Mandar mensagem de volta para o cliente
		/* Função que envia dados em um socket conectado, com parâmetros:
		clientSocket = Socket conectado.
		buf = Ponteiro para o buffer contendo os dados que serão transmitidos.
		bytesReceived+1 = O tamanho em bytes dos dados no buffer apontados pelo parâmetro anterior.
		(Quando recebemos uma mensagem, não recebemos o \0, daí aumentamos em 1 o tamanho).
		0 = Conjunto de flags que influencia o comportamento desta função. (Ver doc.3)
		*/
		send(clientSocket, buf, bytesReceived+1,0);
	}
	// Fechar o socket
	closesocket(clientSocket);
	// Limpar o winsock
	WSACleanup();
}

/* Documentações utilizadas no código:

doc.1 = https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getnameinfo
doc.2 = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
doc.3 = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
Adicionais:
Vídeo base e passo a passo, créditos a Sloan Kelly: https://www.youtube.com/watch?v=WDn-htpBlnU
sockaddr_in = https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in
bind = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind
listen = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen
accept = https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
SecureZeroMemory = https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85)
closesocket = https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket
inet_ntop= https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop
*/