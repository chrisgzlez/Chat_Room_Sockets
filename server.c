#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

#define CLIENT_BACKLOG 5    // Max numeros de conexiones en cola
#define MAX_WAIT_TIME 5     // Tiempo de espera maximo
#define MAX_MSSG_SIZE 1000  // Max Size of Mensaje

#define MENU_MAIN "Welcome to Chris' Chat Server\n" \
    "Type 0 at any point to exit program\n" \
    "1) Manage Chat Rooms\n" \
    "2) Join Chat Room\n" \
    "Option: "



typedef struct {
    char name[100];
    int socket_id;
} user_t;

typedef struct {
    char text[MAX_MSSG_SIZE];
    char user_name[100];
    // ...
} message;


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

int main(int argc, char** argv) {
    struct sockaddr_in  ip_server;
    socklen_t           size = sizeof(struct sockaddr_in);
    int                 id_server;

    // Check that a port was provided
    if (argc < 2) {
        handle_error("No Port provided\n", 0);
    }

    unsigned int port = strtoul(argv[1], NULL, 10);
    
    // Validate Port Number
    if (port <= IPPORT_USERRESERVED) {
        handle_error("Invalid Port\n", 1);
    }    

    // Initialize Values of Server socket
    ip_server.sin_family = AF_INET;
    ip_server.sin_addr.s_addr = htonl(INADDR_ANY);
    ip_server.sin_port = htons(port);
    
    // Creamos el socket 
    if ((id_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Could not create main server socket\n", 1);
    }

    // Bindeamos socket a la direccion Ip
    if ((bind(id_server, (struct sockaddr *)&ip_server, size)) < 0) {
        handle_error("Could not bind main server socket\n", 1);
    }

    // Mark server as to be listening
    if ((listen(id_server, CLIENT_BACKLOG)) < 0) {
        handle_error("Could not set main server to listen\n", 1);
    }

    // Handle Clients
    while(1) {
        struct sockaddr_in  ip_client;
        int                 id_client;

        fprintf(stdout, "----------------------------\n");
        fprintf(stdout, "Waiting to accept cliente\n");
        fprintf(stdout, "----------------------------\n");

        // Conectamos con el cliente
        if ((id_client = accept(id_server, (struct sockaddr *) &ip_client, &size)) < 0) {
            handle_error("Error in accepting connection.\n", 1);
        }

        /* PRINT DATA OF CLIENT */
        char ip_text[20], client_name[100];
        inet_ntop(ip_client.sin_family, &ip_client.sin_addr, ip_text, INET_ADDRSTRLEN);
        sprintf(client_name, "%s:%u", ip_text, ntohs(ip_client.sin_port));
        fprintf(stdout, "Connection to client: %s\n", client_name);
        
        // Make a function and paralelyze
        int     bytes_recv = 0, bytes_sent = 0;
        char    mssg[MAX_MSSG_SIZE];
        while(1) {
             
            bytes_recv = 0, bytes_sent = 0;
            mssg[0] = '\0';

            if ((bytes_recv = recv(id_client, mssg, MAX_MSSG_SIZE, 0)) < 0) {
                handle_error("Error receiving message\n", 1);
            }
            
            fprintf(stdout, "[%s] >> %s", client_name, mssg);
            fprintf(stdout, "Me >> ");

            fgets(mssg, MAX_MSSG_SIZE, stdin);
            if ((bytes_sent = send(id_client, mssg, MAX_MSSG_SIZE, 0)) < 0) {
                handle_error("Error Sending Message", 1);
            }

        }
        // int     bytes_recv = 0, bytes_sent = 0;
        // char    mssg[MAX_WAIT_TIME];
        //
        // if ((bytes_recv = recv(id_client, mssg, MAX_MSSG_SIZE, 0)) < 0) {
        //     handle_error("Error receiving message\n", 1);
        // }
        // 
        // fprintf(stdout, "[%s] >> %s\n", client_name, mssg);
        close(id_client);
    }

}
