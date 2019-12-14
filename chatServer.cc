#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <set>
#include <algorithm>
#include <cerrno>

#include "chat.h"

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    std::set<int> clients;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Couldn't create socket\n");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Couldn't bind socket to port %d\n", SERVER_PORT);
        exit(1);
    }

    if ((listen(sockfd, 5)) != 0)
    {
        printf("Couldn't listen on socket\n");
        exit(1);
    }

    char buf[MAX_BUF_SIZE];

    fd_set master;
    FD_ZERO(&master);
    FD_SET(sockfd, &master);
    int largestfd = sockfd;

    for (;;)
    {
        fd_set copy = master;
        int selectCount = select(largestfd + 1, &copy, nullptr, nullptr, nullptr);

        for (int i = 0; selectCount > 0; i++)
        {
            if (!FD_ISSET(i, &copy))
                continue;

            selectCount--;

            if (i == sockfd)
            { // Accept a new connection
                printf("Got a new connection\n");
                int connfd;
                if ((connfd = accept(sockfd, nullptr, nullptr)) < 0)
                {
                    printf("Couldn't accept inbound connection\n");
                    continue;
                }
                FD_SET(connfd, &master);
                largestfd = std::max(largestfd, connfd);
                clients.insert(connfd);
            }
            else
            { // Recieved a message from this client
                int n = read(i, buf, MAX_BUF_SIZE);
                int byteswritten;
                // printf("Got message %.*s\n", n, buf);
                for (int client : clients)
                {
                    if (client != i)
                    {
                        byteswritten = write(client, buf, n);
                        if (byteswritten == -1 && errno != EBADF)
                        { // Socket was closed
                            clients.erase(client);
                            if (client == largestfd)
                                largestfd = clients.empty() ? sockfd : std::max(sockfd, *prev(clients.end()));
                        }
                        else if (byteswritten != n)
                        {
                            printf("Only wrote %d of %d bytes to client fd %d\n", byteswritten, n, client);
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
}
