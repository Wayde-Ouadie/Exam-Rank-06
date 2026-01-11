#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <sys/select.h>

typedef struct	s_client{
	int		id;
	char	msg[424242];
}	t_client;

t_client	clients[1024];
int			max = 0, next_id = 0;
char		buffRead[424242], buffWrite[424242];
fd_set		Readfds, Writefds, curr; 

void	erro(char *str)
{
	write(2,str,strlen(str));
	exit(1);
}

void	msgsender(int sendfd)
{
	for (int fd = 0; fd <= max ; fd++) {
		if (FD_ISSET(fd, &Writefds) && fd != sendfd)
			write(fd, buffWrite, strlen(buffWrite));
	}
}

int	main(int ac,char** av)
{
	if (ac != 2) erro("Wrong number of arguments\n");
	int					sockfd;
	struct sockaddr_in	servaddr;
	sockfd = max = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) erro("Fatal error\n");
	FD_ZERO(&curr);
	FD_SET(sockfd, &curr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));

	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		erro("Fatal error\n");
	if (listen(sockfd, 10) != 0) erro("Fatal error\n");
	while(1)
	{
		Readfds = Writefds = curr;
		if (select(max + 1, &Readfds, &Writefds, NULL, NULL) < 0) continue;
		for (int fd = 0; fd <= max; fd++)
		{
			if (FD_ISSET(sockfd, &Readfds)) {
				int clientSocket = accept(sockfd, NULL, NULL);
				if (clientSocket > max)
					max = clientSocket;
				clients[clientSocket].id = next_id++;
				bzero(clients[clientSocket].msg, strlen(clients[clientSocket].msg));
				FD_SET(clientSocket, &curr);
				sprintf(buffWrite, "server: client %d just arrived\n", clients[clientSocket].id);
				msgsender(clientSocket);
				break;
			}
			if (FD_ISSET(fd, &Readfds) && fd != sockfd)
			{
				int read = recv(fd, buffRead, sizeof(buffRead), 0);
				if(read <= 0)
				{
					sprintf(buffWrite, "server: client %d just left\n", clients[fd].id);
					msgsender(fd);
					FD_CLR(fd, &curr);
					close(fd);
					break;
				}
				else
				{
					for(int i = 0, j = strlen(clients[fd].msg); i < read; i++, j++)
					{
						clients[fd].msg[j] = buffRead[i];
						if(clients[fd].msg[j] == '\n')
						{
							clients[fd].msg[j] = '\0';
							sprintf(buffWrite, "client %d: %s\n", clients[fd].id, clients[fd].msg);
							msgsender(fd);
							bzero(clients[fd].msg, strlen(clients[fd].msg));
							j = -1;
						}
					}
					break;
				}
			}
		}
	}
}