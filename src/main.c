/* #2
 * Спільна умова для варіантів 1 – 21.
 * Програми (процеси, потоки – згідно варіанту задачі), що реалізують функції f(x) і g(x),
 * займаються тільки обчисленням значення над вхідним аргументом,
 * вони не обробляють ніяких інших запитів (у тому числі – про завершення обчислень) і не взаємодіють з
 * іншими процесами та потоками ні в який інший спосіб,
 * окрім викликів обчислень f(x) і g(x) (тобто запуску функції на обчислення) та
 * повернення результату (коли обчислення результату завершено)
 */

/* #17
 * Взаємодія процесів. Паралелізм. Обмін повідомленнями через порт.
 * Обчислити f(x) && g(x), використовуючи 2 допоміжні процеси: один обчислює f(x), а інший – g(x).
 * Основна програма виконує ввод-вивід та операцію &&.
 * Використати обмін повідомленнями між процесами через порт (Socket).
 * Реалізувати варіант неблокуючих операцій обміну повідомленнями, тобто з “перериванням” обчислень і
 * їх продовженням (відновленням) після отримання повідомлень з результатами ініційованих допоміжних обчислень.
 * Функції f(x) та g(x) “нічого не знають друг про друга” і не можуть комунікувати між собою.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 4747
#define ADDRESS "127.0.0.1"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    printf("main()");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    } else {
        printf("Socket opened\n");
    }

    struct hostent *server = gethostbyname(ADDRESS);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    } else {
        printf("Host found on %s\n", ADDRESS);
    }
    
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    } else {
         char* strAddr = inet_ntoa(serv_addr.sin_addr);
        printf("Connected to %s:%d\n", strAddr, PORT);
    }

    // int n;
    // char buffer[256];


    // 

    // printf("Please enter the message: ");
    // bzero(buffer,256);
    // fgets(buffer,255,stdin);
    // n = write(sockfd,buffer,1);
    // if (n < 0)
    //     error("ERROR writing to socket");
    // bzero(buffer,256);
    // n = read(sockfd,buffer,256);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // printf("%s\n",buffer);


    close(sockfd);
    printf("Socket closed\n");
    return 0;


    return 0;
}
