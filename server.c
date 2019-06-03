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
#include <time.h>

#include "threadpool.c"
#include "components.c"

#define MAXCONNEC 1000
#define BYTES 1024

// for a random delay
int returnRandom(int lower, int upper) {  
	int num = (rand() % 
	(upper - lower + 1)) + lower; 
	return num; 
}

char PORT[6];
char *ROOT;
int listenfd, clients[MAXCONNEC];
char NUMTHREADS[2];
char SLEEPTIME[5];
void error(char *);
void startServer(char *);
void respond(int);

void returnComponent(int slot){
	srand(time(0));
	int secs = returnRandom(2,10);
	if(SLEEPTIME == "delay") sleep(5);
	respond(slot);
}


int main(int argc, char* argv[])
{

	strcpy(NUMTHREADS, argv[2]); //get number of threads
	if(argv[3] == "delay") strcpy(SLEEPTIME, argv[3]); //return components with a 5sec delay 
	
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;    
	
	//PORT = argv[1];
	ROOT = getenv("PWD");
	strcpy(PORT,argv[1]); //PORT 10000

	int slot=0;
	
	// Starts de pool
	threadpool myThreadPool = create_threadpool(atoi(NUMTHREADS));
	
	printf("Server works with %s%s%s threads! Port: %s%s%s | Root Directory %s%s%s\n","\033[92m",NUMTHREADS,"\033[0m","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
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
		{ // A thread main that switch among a pool of threads available to the client.
			
				void (*_returnComponent)(int) = &returnComponent;
				dispatch(myThreadPool, (dispatch_fn)_returnComponent, (int*)slot);
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
	char mesg[99999], *request_line[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients[n], mesg, 99999, 0);
	//printf("\n rcvd=%d", rcvd);

	if (rcvd<0)    // receive error
		fprintf(stderr,("Error in recv()\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected upexpectedly.\n");
	else    // message received
	{
		printf("%s", mesg);
		request_line[0] = strtok (mesg, " \t\n");
		if ( strncmp(request_line[0], "GET\0", 4)==0 )
		{
			request_line[1] = strtok (NULL, " \t");
			request_line[2] = strtok (NULL, " \t\n");
			if ( strncmp( request_line[2], "HTTP/1.0", 8)!=0 && strncmp( request_line[2], "HTTP/1.1", 8)!=0 )
			{
				write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
				//Route Default
				if ( strncmp(request_line[1], "/\0", 2)==0 )
					request_line[1] = "/client_application/index.html";        //DEFAULT

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], request_line[1]);
				printf("file in response: %s\n", path);

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


