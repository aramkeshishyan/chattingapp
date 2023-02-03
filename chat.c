#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <pthread.h>

#include <net/if.h>
#include <sys/ioctl.h>


struct AcceptedSocket
{
	int acceptedSocketFD;
	struct sockaddr_in address;
	int error;
	bool acceptedSuccessfully;
};

struct AcceptedSocket* clientSockets[10];
int connectionsCount = 0;

int CreateSocket();
struct sockaddr_in* createAddress(char *ip, int port);
struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);
void * receiveAndPrintIncomingMessages(void* voidSocketFD);
void receiveIncomingMessagesOnSeparateThread(struct AcceptedSocket *pSocket);
void * acceptNewConnection(void * voidserverSocketFD);

void help();
void list();
void connectTo(char *ip, char *port);
void sendMessage(int id, char *message);
void terminateConnection(int id);
char * getMyIp();
void termianteAllConnections();


int main(int argc, char * argv[])
{
	for(int i = 0; i < 10; i++)
	{
		clientSockets[i] = NULL;
		//printf("clientSockets[%d] points to %p\n", i, clientSockets[i]);
	}

	if(argc < 2)
	{
		printf("Input should be in this format: <filename.c> <portnumber>");
		exit(1);
	}

	char command[100];
	
	int portNum = atoi(argv[1]);;
	int server_socketfd;

	server_socketfd = CreateSocket();
	if(server_socketfd < 0)
	{
		printf("Error when opening the socket");
		exit(1);
	}

	struct sockaddr_in *server_addr = createAddress("", portNum);

	//bind the socket to address and port number
	if(bind(server_socketfd, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0)
	{
		printf("ERROR, binding wasn't successful\n");
		exit(1);
	}

	int listenResult = listen(server_socketfd, 10);


	pthread_t id;
	int *pserver_socketfd = malloc(sizeof(int));
	*pserver_socketfd = server_socketfd;
	pthread_create(&id, NULL, acceptNewConnection, (void *)pserver_socketfd);


	char *token;
	char *temp;
	char *ip;

	do
	{
		token = NULL;
		temp = NULL;
		//printf("Please enter a command.\n");
		scanf("%[^\n]s", command);
		token = strtok(command, " ");


		while ((getchar()) != '\n');

		if(!strcmp(token, "help"))
		{
			help();
		}
		else if(!strcmp(token, "myport"))
		{
			printf("The program runs on port# %d\n", portNum);
		}
		else if(!strcmp(token, "myip"))
		{
			ip = getMyIp();
			printf("The IP address is: %s\n", ip);
		}
		else if(!strcmp(token, "exit"))
		{
			termianteAllConnections();
		}
		else if(!strcmp(token, "connect")) 
		{
			token = strtok(NULL, " ");
			temp = token;
			token = strtok(NULL, " ");


			if(!strcmp(getMyIp(), temp) && !strcmp(token, argv[1]))
				printf("Error, attempt of self connection\n");
			else
				connectTo(temp, token);
		}
		else if(!strcmp(token, "list"))
		{
			list();

		}
		else if(!strcmp(token, "terminate"))
		{
			token = strtok(NULL, " ");
			//int term;
			//printf("Enter id");
			//scanf("%d", &term);
			//while ((getchar()) != '\n');
			terminateConnection(atoi(token));
		}
		else if(!strcmp(token, "send"))
		{
			token = strtok(NULL, " ");
			temp = token;
			token = strtok(NULL, "\n");
		
			//while ((getchar()) != '\n');
			sendMessage(atoi(temp), token);
		}
		else
		{
			printf("Not a valid command.\n");
		}

	}while(strcmp(token, "exit") != 0);

	close(server_socketfd);
	return 0;
}

void termianteAllConnections()
{
	for(int i = 0; i < connectionsCount; i++)
	{
		terminateConnection(i);
	}

}




void connectTo(char *ip, char *port)
{
	char buf[INET_ADDRSTRLEN];

	//check for duplicate connections
	for(int i = 0; i < connectionsCount;i++)
	{
		inet_ntop(AF_INET,&(clientSockets[i]->address.sin_addr.s_addr), buf, sizeof(buf));
		if(!strcmp(buf, ip) && atoi(port) == (ntohs(clientSockets[i]->address.sin_port)))
		{
			printf("The connection already exists\n");
			return;
		}

		//bzero(buf);
	}


	//while ((getchar()) != '\n');
	int portNum = atoi(port);
	int server_socketfd = CreateSocket();

	struct sockaddr_in *server_addr = createAddress(ip, portNum);


	if(connect(server_socketfd, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0)
	{
		printf("ERROR, connection to the server failed!\n");
		return;
	}
	else
	{
		printf("The connection to peer %s on Port# %d is successfully established\n", ip, portNum);
	}


	struct AcceptedSocket * serverSocket = malloc(sizeof(struct AcceptedSocket));
	serverSocket->address = *server_addr;
	serverSocket->acceptedSocketFD = server_socketfd;
	serverSocket->acceptedSuccessfully = server_socketfd>0;

	if(serverSocket->acceptedSuccessfully)
		serverSocket->error = server_socketfd;


	clientSockets[connectionsCount] = serverSocket;
	connectionsCount++;

	receiveIncomingMessagesOnSeparateThread(serverSocket);

}


void terminateConnection(int id)
{
	if(clientSockets[id] == NULL)
	{
		printf("The id doesn't exist.\n");
		return;
	}

	shutdown(clientSockets[id]->acceptedSocketFD, SHUT_RDWR);
	close(clientSockets[id]->acceptedSocketFD);
	free(clientSockets[id]);
	clientSockets[id] = NULL;
	connectionsCount--;

	if(connectionsCount > 0)
	{
		for(int i = 0; i < 10; i++)
		{
			if(i+1 != 10)
				clientSockets[i] = clientSockets[i+1];
		}
	}

}


void help()
{
	printf("The availabe commands are:\n");
	printf("myip -- Display IP address\n");
	printf("myport -- Display this process' listening port\n");
	printf("connect <destination> <port no> -- Establishes connection to the <destination> IP at the specified <port no>.\n");
	printf("list -- Display a list of all the connections this process is part of.\n");
	printf("terminate <connection id.> -- This command will terminate a connection using its id:\n");
	printf("send <connection id.> <message> -- send the messages to peers.\n");
	printf("exit -- Close all connections and terminate this process\n");
}


int CreateSocket()
{
	return socket(AF_INET, SOCK_STREAM,0);
}

struct sockaddr_in* createAddress(char *ip, int port)
{
	struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
	address->sin_family = AF_INET;
	address->sin_port = htons(port);


	if(strlen(ip) == 0)
		address->sin_addr.s_addr = INADDR_ANY;
	else
		inet_pton(AF_INET, ip, &address->sin_addr.s_addr);


	return address;
}

void sendMessage(int id, char *message)
{

	ssize_t amountWasSent = send(clientSockets[id]->acceptedSocketFD, message, strlen(message), 0);
	if(amountWasSent == -1)
	{
		printf("Couldn't send the message.\n");
	}
	else
		printf("Message sent to %d\n", id);

	message = NULL;

}



struct AcceptedSocket * acceptIncomingConnection(int server_socketfd)
{
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);

	//the requestor
	int clientSocketFD = accept(server_socketfd, (struct sockaddr *) &clientAddress, &clientAddressSize);

	struct AcceptedSocket * acceptedSocket = malloc(sizeof(struct AcceptedSocket));
	acceptedSocket->address = clientAddress;
	acceptedSocket->acceptedSocketFD = clientSocketFD;
	acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

	if(acceptedSocket->acceptedSuccessfully)
		acceptedSocket->error = clientSocketFD;

	return acceptedSocket;
}


void * acceptNewConnection(void * voidserverSocketFD)//(int serverSocketFD)
{
	int serverSocketFD = *((int*)voidserverSocketFD);
	free(voidserverSocketFD);
	struct AcceptedSocket* clientSocket;
	char buf[INET_ADDRSTRLEN];
	
	while(1)
	{
		//printf("before accepting\n");
		clientSocket = acceptIncomingConnection(serverSocketFD);          ////////////////LISTEN AND ACCEPT AS A SERVER, GET THE REQUESTORS INFO
		//printf("after accepting a client\n");
		clientSockets[connectionsCount] = clientSocket;

		inet_ntop(AF_INET,&(clientSockets[connectionsCount]->address.sin_addr.s_addr), buf, sizeof(buf)); 
		printf("The connection to peer %s on Port# %d is successfully established\n", buf, ntohs(clientSockets[connectionsCount]->address.sin_port));


		connectionsCount++;

		receiveIncomingMessagesOnSeparateThread(clientSocket);	///////////USES REQUESTOR INFO TO RECEIVE FROM THEM
	}	
}


void receiveIncomingMessagesOnSeparateThread(struct AcceptedSocket *pSocket)
{
	pthread_t id;
	pthread_create(&id, NULL, receiveAndPrintIncomingMessages, (void *)pSocket);
}


void * receiveAndPrintIncomingMessages(void * voidAcceptedSocket)//int socketFD
{
	struct AcceptedSocket socketStruct = *((struct AcceptedSocket*)voidAcceptedSocket);
	//free(voidSocketFD);

	char buf[INET_ADDRSTRLEN];
	char buffer[1024];
	inet_ntop(AF_INET,&(socketStruct.address.sin_addr.s_addr), buf, sizeof(buf)); 

	while(1)
	{
		int message = recv(socketStruct.acceptedSocketFD, buffer, 1024, 0);
		if(message <= 0)
		{	
			printf("Peer %s on Port# %d connection terminated\n", buf, ntohs(socketStruct.address.sin_port));

			for(int i = 0; i < connectionsCount; i++)
			{
				if(clientSockets[i]->acceptedSocketFD == socketStruct.acceptedSocketFD)
				{
					terminateConnection(i);
					break;
				}
			}

			break;
		}
		else
		{
			buffer[message] = 0;
			printf("Message received from %s\n", buf);
			printf("Sender’s Port: %d\n", ntohs(socketStruct.address.sin_port));
			printf("Message: %s\n", buffer);
		}
	}	
	

}

void list()
{
	char buf[INET_ADDRSTRLEN];
	printf("id: IP address                Port No.\n");
	int i = 0;

	while(i < connectionsCount)
	{
		if(inet_ntop(AF_INET,&(clientSockets[i]->address.sin_addr.s_addr), buf, sizeof(buf)) != NULL);
		{
			if(clientSockets[i] != NULL)
			printf(" %d: %s                 %d\n", i, buf, ntohs(clientSockets[i]->address.sin_port));
		}

		i++;
	}
	
}


char * getMyIp()
{
	int n;
    struct ifreq ifr;
    char array[] = "eth0";
    char * ip;
 
    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);

    ip = inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);

    return ip;

}



