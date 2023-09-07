#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>




void exit_fatal()
{
  write(2, "Fatal error\n", 12);
      exit(1);
}



int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				exit_fatal();
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		exit_fatal();
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
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
		for (int i; i <= max_fd; i++)
		{
			if (!FD_ISSET(i, &writefds) ||  )
		}
	}
	memset(writebuf, 0, 424242);
}


int main(int argc, char *argv[]) {

	int nbr_clients = 0;

	int clientfd, len;
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
					else
					{
						sprintf(writebuf, "server: client %d just arrived\n", clientfd);
						printf(writebuf);
						sendAll(clientfd);
					}
				}
				}
			}
		}


			
	}
