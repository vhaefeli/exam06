/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vhaefeli <vhaefeli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/12 08:13:30 by trossel           #+#    #+#             */
/*   Updated: 2023/04/06 08:05:40 by vhaefeli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

typedef struct s_client {
	int id;
	char msgBuffer[10000];
} t_client;

t_client			clients[1024];
fd_set				active_fds;
fd_set				read_fds;
fd_set				write_fds;
int					max_fd;
int					socket_fd;
int					next_id = 0;
int					port;
struct sockaddr_in	my_addr;
socklen_t			my_size;
char 				write_buf[424242];
char				read_buf[424242];

void exitError(char *msg)
{
	if (msg)
		write(2, msg, strlen(msg));
	exit(1);
}

void	sendMsgToAllOthers(int sender_fd, const char *msg)
{
	if (!msg)
		return;
	for(int i = 0; i < max_fd; i++)
	{
		if (!FD_ISSET(i, &write_fds) || i == socket_fd || i == sender_fd)
			continue;
		send(i, msg, strlen(msg), 0);
	}
}


int main(int ac, char *av[])
{

	if (ac < 2)
		exitError("Wrong number of arguments\n");

	FD_ZERO(&active_fds);
	bzero(clients, 1024 * sizeof(t_client));

	port = atoi(av[1]);
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = (1 << 24) + 127; // in network byte order
	my_addr.sin_port = atoi(av[1]);

	int my_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (my_socket < 0
		|| bind(my_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0
		|| listen(my_socket, 128) < 0)
		exitError("Fatal error\n");

	max_fd = my_socket;
	FD_SET(my_socket, &active_fds);

	while(1)
	{
		write_fds = active_fds;
		read_fds = active_fds;

		int n = select(max_fd + 1, &read_fds, &write_fds, NULL, NULL);
		if (n < 0)
			continue;
		for(unsigned int fd = 0; fd < max_fd; fd++)
		{
			if (!FD_ISSET(fd, &read_fds))
				continue;
			if (fd == my_socket)
			{
				int client_fd = accept(fd, NULL, NULL);
				if (client_fd < 0)
					continue;
				FD_SET(client_fd, &active_fds);
				if (client_fd > max_fd)
					max_fd = client_fd;
				clients[client_fd].id = next_id++;
				sprintf(write_buf, "server: client %d just arrived\n", clients[client_fd].id);
				sendMsgToAllOthers(client_fd, write_buf);
				break;
			}
			else
			{
				int read_buff[424242];
				int n_bytes = read(fd, &read_buff, 424242);
				if (n_bytes <= 0)
				{
					sprintf(write_buf, "server: client %d just left\n", clients[fd].id);
					sendMsgToAllOthers(fd, write_buf);
					FD_CLR(fd, &active_fds);
					close(fd);
					break;
				}
				else
				{
					for(int i = 0, j = 0; i < n_bytes; i++, j++)
					{
						clients[fd].msgBuffer[j] = read_buff[i];
						if (read_buff[i] == '\n')
						{
							clients[fd].msgBuffer[j] = '\0';
							sprintf(write_buf, "client %d: %s", fd, clients[fd].msgBuffer);
							sendMsgToAllOthers(fd, write_buf);
							bzero(clients[fd].msgBuffer, j);
							j = -1;
						}
					}
					break;
				}
			}

		}
	}
}


// 1. Initialize server my_socket
//		- socket
//		- bind
//		- listen
// 2. Initialize client list
// 3. Boucle while:
//		1. select
//		2. iterate on 0..max_fd
//			if !is_set -> continue
//			if fd == fd_server -> new client
//				- accept
//			else
//				- read
//				if error -> client deconnexion
//				else -> client msg
