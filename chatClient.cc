#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>
#include <iostream>
#include <string>
#include "chat.h"

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Couldn't create a socket\n");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Couldn't connect to the server\n");
        exit(1);
    }

    printf("Input username: ");
    std::string username;
    std::cin >> username;
    if (username.size() > MAX_BUF_SIZE - 4)
        username.erase(username.begin() + (MAX_BUF_SIZE - 4), username.end());
    username.insert(0, 1, '[');
    username.append("]: ");

    fd_set master;
    FD_ZERO(&master);
    FD_SET(sockfd, &master);
    FD_SET(STDIN_FILENO, &master);
    int largestfd = std::max(sockfd, STDIN_FILENO);

    char buf[MAX_BUF_SIZE];
    int n;
    for (;;)
    {
        fd_set copy = master;
        select(largestfd + 1, &copy, nullptr, nullptr, nullptr);

        if (FD_ISSET(sockfd, &copy))
        {
            n = read(sockfd, buf, MAX_BUF_SIZE);
            printf("%.*s", n, buf);
        }

        if (FD_ISSET(STDIN_FILENO, &copy))
        {
            n = username.size();
            strncpy(buf, username.c_str(), username.size());
            n += read(STDIN_FILENO, buf + username.size(), MAX_BUF_SIZE - username.size());
            if ((strncmp(buf, "/quit", 5)) == 0)
                break;
            write(sockfd, buf, n);
        }
    }

    close(sockfd);
}
