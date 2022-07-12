#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

// Includo le strutture definite da me
#include "utilities.h"

// Definisco PORT come 8080
#define PORT 8080

/*
 * Definisco le strutture per evitare di
 * scrivere ogni volta struct nomeStruct 
 * per inizializare le variabili
 */
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

int sockfd;

void doOperations(int sockfd);

void interruption_handler(int n);

int main(void) {
    
    SA_IN server_addr;

    signal(SIGINT, interruption_handler);

    // Creiamo la socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Creazione del socket fallita\n");
        exit(0);
    }

    // Assegnamo IP e PORTA
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(PORT);

    // Adesso ci colleghiamo con il server
    if(connect(sockfd, (SA *)&server_addr, sizeof(server_addr)) != 0) {
        printf("Connessione con il server fallita\n");
        exit(0);
    }

    printf("Connesso con il server!\n");

    // Faccio partire la funzione per prendere in input le operazioni da fare
    doOperations(sockfd);

    // Chiudo il socket
    close(sockfd);
}

void doOperations(int sockfd) {
    
    int inputs;
    Operation op;
    Response res;

    //TODO: Implementare controllo errori

    while(1) {
        printf("Inserisci l'operatione da fare: ");

        do {
            // Leggo in entrata l'operazione da fare
            inputs = scanf("%f %c %f", &op.firstValue, &op.operator, & op.secondValue);
            fflush()

        } while(inputs != 3);

        // Mando l'operazione da fare al server
        send(sockfd, &op, sizeof(Operation), 0);

        // Prendo la risposta dal server
        recv(sockfd, &res, sizeof(Response), 0);

        // Metto la risposta in out come risultato e tempo impiegato
        printf("Risultato: %f \t Tempo: %ld\n", res.result, res.invio - res.ricezione);
    }
}

void interruption_handler(int n) {
    write(STDOUT_FILENO, "\nChiudo la connesione\n", 22);
    close(sockfd);
    exit(0);
}