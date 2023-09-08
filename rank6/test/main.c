#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

void exitFatal()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

typedef struct s_client
{
	int id;
	char buf[10000];
} t_client;

int fdserv, fdmax;
	t_client clients[1024];
fd_set readfds, writefds, activfds;
char writebuf[424242], readbuf[424242];

void sendAll(int fd_sender)
{
	if (!writebuf[0])
		return ;
	else
	{
		for (int i = 0; i <= fdmax; i++)
		{
			if (!FD_ISSET(i, &writefds) || i == fdserv || i == fd_sender)
				continue;
			send (i, writebuf, strlen(writebuf), 0);
		}
	}
	memset(writebuf, 0, 424242);
}


int main(int argc, char *argv[]) {

	int clientfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli; 
	int nbclients = 0;

	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	
	memset(clients, 0, sizeof(t_client)*1024);
	memset(readbuf, 0, 424242);
	memset(writebuf, 0, 424242);
	FD_ZERO(&activfds);
	

	// socket create and verification 
	fdserv = socket(AF_INET, SOCK_STREAM, 0); 
	if (fdserv == -1) { 
		exitFatal();
	} 
	else
		printf("Socket successfully created..\n"); 
	memset(&servaddr,0, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(fdserv, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		exitFatal();
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(fdserv, 10) != 0) {
		exitFatal();
	}


	FD_SET(fdserv, &activfds);
	fdmax = fdserv;
		
	while(1)
	{
		writefds = readfds = activfds;

		if (select(fdmax + 1, &readfds, &writefds, NULL, NULL) < 0)
			continue;
		for(int fd = 0; fd <= fdmax; fd ++)
		{
			if (FD_ISSET(fd, &activfds))
			{
				printf("fd%d\n" ,fd);
				if (fd == fdserv)
				{
					len = sizeof(cli);
					clientfd = accept(fdserv, (struct sockaddr *)&cli, &len);
					if (clientfd < 0) { 
						continue;
					} 
					FD_SET(clientfd, &activfds);
					clients[clientfd].id = nbclients;
					sprintf(writebuf, "server: client %d just arrived\n", nbclients);
					printf("%s\n" ,writebuf);
					sendAll(clientfd);
					nbclients++;
					if (clientfd > fdmax)
						fdmax = clientfd;
					break;

				}
				else
				{
					int nB = read(fd, &readbuf, 424242);
					if (nB <= 0)
					{
						sprintf(writebuf, "server: client %d just left\n", clients[fd].id);
						printf("%s\n" ,writebuf);
						sendAll(fd);
						FD_CLR(fd, &activfds);
						close(fd);
						break;
					}
					else
					{
						for (int i = 0, j = 0; i < nB; i++, j++)
						{
							clients[fd].buf[j] = readbuf[i];
							if (readbuf[i] == '\n' || readbuf[i] == 0)
							{
								clients[fd].buf[j] = 0;
								sprintf(writebuf, "client %d: %s\n", clients[fd].id, clients[fd].buf);
								printf("%s\n" ,writebuf);
								sendAll(fd);
								memset(clients[fd].buf, 0, j);
								j = -1;
							}
						}
						memset(readbuf, 0, 424242);
						break;
					}

				}
			}
		}


	}
	
	
}