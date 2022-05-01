#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4747
#define MESSAGE_SIZE 4

void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[]) {
    printf("f()\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    } else {
        printf("Socket opened\n");
    }

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    } else {
        printf("Socket bind on 127.0.0.1:%d\n", PORT);
    }


    listen(sockfd, 5);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        error("ERROR on accept");
    } else {
        char* strAddr = inet_ntoa(cli_addr.sin_addr);
        printf("Accepted connection from %s\n", strAddr);
    }

    char buffer[MESSAGE_SIZE];
    bzero(buffer, MESSAGE_SIZE);
    int n = read(newsockfd, &buffer, MESSAGE_SIZE);
    if (n < 0) {
        error("ERROR reading from socket");
    } else {
        printf("Successfully read from socket: '%s'\n", buffer);
    }

    buffer[0] += 5;

    n = write(newsockfd, buffer, MESSAGE_SIZE);

    if (n < 0) {
        error("ERROR writing to socket");
    } else {
        printf("Successfully written to socket: '%s'\n", buffer);
    }

    close(newsockfd);
    close(sockfd);
    printf("Socket closed\n");
    return 0;
}
