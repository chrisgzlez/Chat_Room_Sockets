#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <signal.h>

#define CLIENT_BACKLOG 5    // Max numeros de conexiones en cola
#define MAX_WAIT_TIME 5     // Tiempo de espera maximo
#define MAX_MSSG_SIZE 1000  // Max Size of Mensaje

#define MENU_MAIN "Welcome to Chris' Chat Server\n" \
    "Type 'ctrl + c' at any point to exit program\n" \
    "1) Manage Chat Rooms\n" \
    "2) Join Chat Room\n" \
    "Option: "


typedef struct {
    char text[MAX_MSSG_SIZE];
    char user_name[100];
    // ...
} message;


int id_client;

/**
 * Funcion para manejar los errores
 */
void handle_error(char *msg, int flag) {
    if(flag)
        perror(msg);
    else
        fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}


void handle_signal(int signal);

// Deals with receiving messages and displaying them
void *chat_display(void *args);

// Deals with writing messages
void *write_messages(void *args);


int main(int argc, char** argv) {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    // Bind signal handler
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        handle_error("Error binding the signal handler\n", 1);
    }


    struct sockaddr_in  ip_server;
    socklen_t           size = sizeof(struct sockaddr_in);

    // Check that a port and ip were provided 
    if (argc < 3) {
        handle_error("Not enough arguments were provided\n", 0);
    }

    // Initialize values of client socket
    unsigned int port = strtoul(argv[2], NULL, 10);

    if (port <= IPPORT_USERRESERVED) {
        handle_error("Invalid Port\n", 0);
    }


    ip_server.sin_port = htons(port);
    ip_server.sin_family = AF_INET;
    if (!inet_pton(AF_INET, argv[1], &ip_server.sin_addr.s_addr)) {
        handle_error("Error converting ip to network format\n", 1);
    }


    // Create Client socket
    if ((id_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Could not create client socket", 1);
    }

    // Bind Client socket to ip 
    if (connect(id_client, (struct sockaddr *)&ip_server, size) < 0) {
        handle_error("Could not connect to server", 1);
    }
    
    char ip_text[20], server_name[100];
    inet_ntop(ip_server.sin_family, &ip_server.sin_addr, ip_text, INET_ADDRSTRLEN);
    sprintf(server_name, "%s:%u", ip_text, ntohs(ip_server.sin_port));

    char mssg[MAX_MSSG_SIZE];
    pthread_t write_thread, read_thread;
    while(1) {

        if(pthread_create(&write_thread, NULL, write_messages, (void*)&id_client)) {
            handle_error("Could not create writing thread\n", 1);
        }

        if(pthread_create(&read_thread, NULL, chat_display, (void*)&id_client)) {
            handle_error("Could not create reading thread\n", 1);
        }

        mssg[0] = '\0';


        if(recv(id_client, mssg, MAX_MSSG_SIZE, 0) < 0) {
            handle_error("Could not receive message\n", 1);
        }

        fprintf(stdout, "%s >> %s", server_name, mssg);

    }

    close(id_client);
}

void *write_messages(void *args) {
    int socket_id = *(int*)args;
    message mssg;
    while(1) {

        mssg.text[0] = '\0';

        fprintf(stdout, ">> ");
        fgets(mssg.text, MAX_MSSG_SIZE, stdin);

        if(send(id_client, &mssg, MAX_MSSG_SIZE, 0) < 0) {
            handle_error("Could not send message\n", 1);
        }
    }

}

void *chat_display(void *args) {
    int socket_id = *(int*)args;
    message mssg;
    while(1) {

        mssg.text[0] = '\0';

        if(recv(id_client, &mssg, MAX_MSSG_SIZE, 0) < 0) {
            handle_error("Could not receive message\n", 1);
        }

        fprintf(stdout, "%s >> %s", mssg.user_name, mssg.text);
    }
}


void handle_signal(int signal) {
    switch (signal) {
        case SIGINT:
            printf("\r\n Cerrando Cliente...");
            close(id_client);
            exit(EXIT_SUCCESS);

        case SIGUSR1:
            break;
    }
}




