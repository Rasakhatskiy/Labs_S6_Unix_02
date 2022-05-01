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

#define PORT "4747"
#define ADDRESS "127.0.0.1"
#define MESSAGE_SIZE 4

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    printf("main()\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    } else {
        printf("Socket opened.\n");
    }

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int n = getaddrinfo(ADDRESS, PORT, &hints, &servinfo);

    if(n != 0) {
        error("Can't get server address info\n");
    } else {
        printf("Successfuly get address info.\n");
    }

    n = connect(sockfd, servinfo->ai_addr, (socklen_t) servinfo->ai_addrlen);
    if (n != 0) {
        error("Can't connect\n");
    } else {
        printf("Successfuly connected to %s:%s.\n", ADDRESS, PORT);
    }

    char buffer[MESSAGE_SIZE];

    printf("Please enter the message: ");
    bzero(buffer, MESSAGE_SIZE);
    fgets(buffer, MESSAGE_SIZE - 1, stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    n = write(sockfd, buffer, MESSAGE_SIZE);
    if (n < 0) {
        error("ERROR writing to socket");
    } else {
        printf("Successfully written to socket: '%s'\n", buffer);
    }

    n = read(sockfd, &buffer, MESSAGE_SIZE);
    if (n < 0) {
        error("ERROR reading from socket");
    } else {
        printf("Successfully read from socket: '%s'\n", buffer);
    }

    close(sockfd);
    printf("Socket closed.\n");
    return 0;


    return 0;
}
