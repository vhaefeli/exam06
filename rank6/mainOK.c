#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>




void exit_fatal()
{
  write(2, "Fatal error\n", 12);
      exit(1);
}

typedef struct s_client
{
int id;
char buf[10000];
} t_client;

t_client clients[1024];
char readbuf[424242], writebuf[424242];
fd_set readfds, writefds, activefds;
int max_fd;

int servfd;

void sendAll(int fd_sender)
{
	if (!writebuf[0])
		return ;
	else
	{
		for (int i = 0; i <= max_fd; i++)
		{
			if (!FD_ISSET(i, &writefds) || i == servfd || i == fd_sender)
				continue;
			send (i, writebuf, strlen(writebuf), 0);
		}
	}
	memset(writebuf, 0, 424242);
}


int main(int argc, char *argv[]) {

	int nbr_clients = 0;

	int clientfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli; 

  if (argc != 2)
    {
      write(2, "Wrong number of arguments\n", 26);
      return (1);
    }

	memset(clients, 0, sizeof(t_client) * 1024);
	memset(readbuf, 0, 424242);
	memset(writebuf, 0, 424242);
	FD_ZERO(&activefds);
	readfds = activefds;
	writefds = activefds;

	// socket create and verification 
	servfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (servfd == -1) { 
		exit_fatal();
	} 
	else
		printf("Socket successfully created..\n"); 
	memset(&servaddr, 0, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(servfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		exit_fatal();
	} 
	if (listen(servfd, 10) != 0) {
		exit_fatal();
	}
	max_fd = servfd;
	FD_SET(servfd, &activefds);
	
	while(1)
	{
		readfds = activefds;
		writefds = activefds;

		if (select(max_fd + 1, &readfds, &writefds, NULL, NULL) < 0)
			continue;
		for (int fd = 0; fd <= max_fd; fd++)
		{
			if (FD_ISSET(fd, &readfds))
			{
				if (fd == servfd)
				{
					len = sizeof(cli);
					clientfd = accept(servfd, (struct sockaddr *)&cli, &len);
					if (clientfd < 0) { 
						continue;
					} 
					FD_SET(clientfd, &activefds);
					if (clientfd > max_fd)
						max_fd = clientfd;
					clients[clientfd].id = nbr_clients;
					nbr_clients++;
					sprintf(writebuf, "server: client %d just arrived\n", clients[clientfd].id);
					printf("%s\n" ,writebuf);
					sendAll(clientfd);
					break;
				}
				else
				{
					int n_bytes = read(fd, &readbuf, 424242);
					if( n_bytes <= 0)
					{
						sprintf(writebuf, "server: client %d just left\n", clients[fd].id);
						printf("%s\n" ,writebuf);
						sendAll(fd);
						FD_CLR(fd, &activefds);
						close(fd);
						break;
					}
					else
					{
						for( int i = 0, j = 0 ; i < n_bytes; i++, j++)
						{
							clients[fd].buf[j] = readbuf[i];
							if (readbuf[i] == '\n' || readbuf[i] == 0)
							{
								clients[fd].buf[j] = 0;
								sprintf(writebuf, "client %d: %s\n", clients[fd].id, clients[fd].buf);
								printf("%s\n" ,writebuf);
								sendAll(fd);
								memset(clients[fd].buf, 0, j);
								j= -1;
							}
						}
						break;
					}
				}
			}
		}
	}		
}
