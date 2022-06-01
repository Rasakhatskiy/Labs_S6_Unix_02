#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>

#define PORTG "4741"
#define PORTF "4742"
#define MESSAGE_SIZE 4

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(int functType) {
    // Listening socket descriptor
    int listener;

    // For setsockopt() SO_REUSEADDR, below
    int yes = 1;
    int rv;
    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const char* PORT = functType ? PORTF : PORTG;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {

    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}


int main(void) {
    printf("Хто я? f/g: ");
    char funcTypeC;
    scanf("%c",&funcTypeC);
    if (funcTypeC != 'f' && funcTypeC != 'g' && funcTypeC != 'F' && funcTypeC != 'G') {
        printf("Не знаємо таких\n");
        exit(1);
    }
    int funcType;
    if (funcTypeC == 'f' || funcTypeC == 'F') {
        funcType = 1;
    } else {
        funcType = 0;
    }



    // Listening socket descriptor
    int listener;

    // Newly accept()ed socket descriptor
    int newFD;

    // Client address
    struct sockaddr_storage remoteAddr;
    socklen_t addrLen;

    // Buffer for client data
    char buf[2];

    char remoteIP[INET6_ADDRSTRLEN];

    // Start off with room for 5 connections
    // (We'll realloc as necessary)
    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // Set up and get a listening socket
    listener = get_listener_socket(funcType);

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Add the listener to set
    pfds[0].fd = listener;

    // Report ready to read on incoming connection
    pfds[0].events = POLLIN;

    // For the listener
    fd_count = 1;

    // Main loop
    for (;;) {
        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for (int i = 0; i < fd_count; i++) {

            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN) { // We got one!!

                if (pfds[i].fd == listener) {
                    // If listener is ready to read, handle new connection

                    addrLen = sizeof remoteAddr;
                    newFD = accept(listener,
                                   (struct sockaddr *) &remoteAddr,
                                   &addrLen);

                    if (newFD == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newFD, &fd_count, &fd_size);

                        printf("pollserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteAddr.ss_family,
                                         get_in_addr((struct sockaddr *) &remoteAddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newFD);

                    }
                } else {
                    // If not the listener, we're just a regular client
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0) {
                        // Got error or connection closed by client
                        if (nbytes == 0) {
                            // Connection closed
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd); // Bye!

                        del_from_pfds(pfds, i, &fd_count);

                    } else {
                        // We got some good data from a client

                        printf("Reseived: %s\n", buf[0] ? "1" : "0");

                        if (funcType) {
                            buf[0] += 1;
                            if (buf[0]) {
                                buf[0] = 0;
                            }
                        } else {
                            if (buf[0]) {
                                buf[0] = 0;
                            } else {
                                buf[0] = 1;
                            }
                        }

                        printf("Processing data for 5 s\n");
                        sleep(5);

                        if (sender_fd != listener) {
                            if (send(sender_fd, buf, nbytes, 0) == -1) {
                                perror("send");
                            } else {
                                printf("Sent '%d'\n", buf[0] ? 1 : 0);
                            }
                        }
                    }
                } // END handle data from client
            } // END got ready-to-read from poll()
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}








//int main(int argc, char *argv[]) {
//    printf("f()\n");
//    int listener = socket(AF_INET, SOCK_STREAM, 0);
//
//    if (listener < 0) {
//        error("ERROR opening socket");
//    } else {
//        printf("Socket opened\n");
//    }
//
//    struct sockaddr_in serv_addr;
//    bzero((char *) &serv_addr, sizeof(serv_addr));
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = INADDR_ANY;
//    serv_addr.sin_port = htons(PORT);
//
//    if (bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
//        error("ERROR on binding");
//    } else {
//        printf("Socket bind on 127.0.0.1:%d\n", PORT);
//    }
//
//
//    listen(listener, 5);
//
//    struct sockaddr_in cli_addr;
//    socklen_t clilen = sizeof(cli_addr);
//
//
//    char buffer[MESSAGE_SIZE];
//
//    struct pollfd pfds[1];
//
//    for (;;) {
//        int newsockfd = accept(listener, (struct sockaddr *) &cli_addr, &clilen);
//
//        if (newsockfd < 0) {
//            error("ERROR on accept");
//        } else {
//            char *strAddr = inet_ntoa(cli_addr.sin_addr);
//            printf("Accepted connection from %s\n", strAddr);
//        }
//
//        bzero(buffer, MESSAGE_SIZE);
//        int n = read(newsockfd, &buffer, MESSAGE_SIZE);
//        if (n < 0) {
//            error("ERROR reading from socket");
//        } else {
//            printf("Successfully read from socket: '%s'\n", buffer);
//        }
//
//        buffer[0] += 5;
//
//        n = write(newsockfd, buffer, MESSAGE_SIZE);
//
//        if (n < 0) {
//            error("ERROR writing to socket");
//        } else {
//            printf("Successfully written to socket: '%s'\n", buffer);
//        }
//
//        close(newsockfd);
//    }
//
//
////    close(newsockfd);
//    close(listener);
//    printf("Socket closed\n");
//    return 0;
//}
