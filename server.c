#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<signal.h>
#include<fcntl.h>
#include<math.h>
#include<pthread.h>

#include "threadpool.c"
#include "components.c"

#define MAXCONNEC 1000
#define BYTES 1024

char PORT[6];
char *ROOT;
int listenfd, clients[MAXCONNEC];
void error(char *);
void startServer(char *);
void respond(int);

void returnComponent(int slot){
	printf("return component\n");
	sleep(5);
	respond(slot);
	//exit(0);
}


int main(int argc, char* argv[])
{
	//printf("%s", (char *)argv[1]);
	//printf("%s", (char *)argv[2]);
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;    
	
	//PORT = argv[1];
	ROOT = getenv("PWD");
	strcpy(PORT,argv[1]); //PORT 10000

	int slot=0;
	
	// Starts de pool
	//printf("vai passar\n");
	threadpool myThreadPool = create_threadpool(2);
	//printf("passou\n");
	//dispatch(myThreadPool, (dispatch_fn)_getComponent, &c);
	//void (*_returnComponent)(int) = &returnComponent;
	//dispatch(myThreadPool, (dispatch_fn)_returnComponent, &slot);
	
	printf("Server works! Port: %s%s%s | Root Directory %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	// Setting all elements to -1: signifies there is no client connected
	int i; 
	for (i=0; i<MAXCONNEC; i++)
		clients[i]=-1;
	startServer(PORT);

	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

		if (clients[slot]<0)
			error ("accept() error");
		else
		{ // TODO: A thread main that switch among a pool of threads available to the client.
			
			//int (*_getComponent)(int) = &getComponent;
			//dispatch(myThreadPool, (dispatch_fn)_getComponent, &c);
				void (*_returnComponent)(int) = &returnComponent;
				dispatch(myThreadPool, (dispatch_fn)_returnComponent, (void*)slot);
	
			//if ( fork()==0 )
			//{
			
				//respond(slot);	
				//exit(0);
			//}
		}

		while (clients[slot]!=-1) slot = (slot+1)%MAXCONNEC;
	}

	return 0;
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}


void respond(int n)
{
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients[n], mesg, 99999, 0);
	printf("oi\n rcvd=%d", rcvd);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected upexpectedly.\n");
	else    // message received
	{
		printf("%s", mesg);
		reqline[0] = strtok (mesg, " \t\n");
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
				if ( strncmp(reqline[1], "/\0", 2)==0 )
					reqline[1] = "/client_application/index.html";        //DEFAULT

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], reqline[1]);
				printf("file: %s\n", path);

				if ( (fd=open(path, O_RDONLY))!=-1 ) 
				{
					send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
					while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
						write (clients[n], data_to_send, bytes_read);
				}
				else    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
			}
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         
	close(clients[n]);
	clients[n]=-1;
}


