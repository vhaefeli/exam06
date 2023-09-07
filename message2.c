#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct client
{
	int		id;
	char	msg[100000];
} t_client;

t_client	clients[1024];
int		fd_max = 0;
int		nb_cli = 0;
fd_set	fdset, readfds, writefds;
char	buf_read[424242], buf_write[424242];

void	errorminator(char *error)
{
	if (error == NULL)
		error = "fatal error\n";
	write(2, error, strlen(error));
	exit(1);
}

void	send_to_all(int fd)
{
	for (int i = 0; i <= fd_max; i++)
	{
		if (FD_ISSET(i, &writefds) && i != fd)
			send(i, buf_write, strlen(buf_write), 0);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2)
		errorminator("Wrong args...\n");
	int	port = atoi(argv[1]);
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	socklen_t len;

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		errorminator(NULL);
	} 

	bzero(clients, sizeof(clients));
	FD_ZERO(&fdset);
	fd_max = sockfd;
	FD_SET(sockfd, &fdset);

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		close(sockfd);
		errorminator(NULL);
	} 

	if (listen(sockfd, 128) != 0) {
		close(sockfd);
		errorminator(NULL);
	}

	while (1)
	{
		readfds = writefds = fdset;
		if (select(fd_max + 1, &readfds, &writefds, NULL, NULL) < 0)
			continue;

		for (int fd = 0; fd <= fd_max; fd++)
		{
			if (FD_ISSET(fd, &readfds) && fd == sockfd)
			{
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0) {
					continue;
				}

				if (fd_max < connfd)
					fd_max = connfd;

				clients[connfd].id = nb_cli++;
				FD_SET(connfd, &fdset);
				sprintf(buf_write, "server: client %d just arrived\n", clients[connfd].id);
				send_to_all(connfd);
				break;
			}

			if (FD_ISSET(fd, &readfds) && fd != sockfd)
			{
				int	read = recv(fd, buf_read, 424242, 0);
				if (read <= 0)
				{
					sprintf(buf_write, "server: client %d just left\n", clients[fd].id);
					send_to_all(fd);
					FD_CLR(fd, &fdset);
					close(fd);
					break;
				}
				else
				{
					for (int i = 0, j = strlen(clients[fd].msg); i < read; i++, j++)
					{
						clients[fd].msg[j] = buf_read[i];
						if (clients[fd].msg[j] == '\n')
						{
							clients[fd].msg[j] = '\0';
							sprintf(buf_write, "client %d: %s\n", clients[fd].id, clients[fd].msg);
							send_to_all(fd);
							bzero(&clients[fd].msg, strlen(clients[fd].msg));
							j = -1;
						}
					}
					break;
				}
			}
		}
	}
}
