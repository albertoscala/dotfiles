#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

// Includo le strutture definite da me
#include "utilities.h"

/*
 * Definisco la PORT come 8080 
 * E definisco la BACKLOG come 100 prima di rifiutare altri client
 */
#define PORT 8080
#define SERVER_BACKLOG 100

/*
 * Definisco le strutture per evitare di
 * scrivere ogni volta struct nomeStruct 
 * per inizializare le variabili
 */
typedef struct timespec TS;
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

FILE *log_file;
SA_IN client_addr;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Funzione che esegue le operazioni richieste dai 
 * client adattata per funzionare con i thread
 */
void *execOperations(void *pclient);

void compileLog(Operation op, Response res, int ip);

void interruptionHandler(int n);

int main(void) {

    SA_IN server_addr;
    int server_socket, client_socket, addr_size;

    signal(SIGINT, interruptionHandler);

    // socket create and verification
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }

    // Assegno IP e PORTA
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    // Binding newly created socket to given IP and verification
    if((bind(server_socket, (SA *)&server_addr, sizeof(server_addr))) != 0) {
        printf("Abbinamento al socket fallito\n");
        exit(0);
    }

    // Adesso metto il server in ascolto
    if((listen(server_socket, SERVER_BACKLOG)) != 0) {
        printf("Ascolto fallito\n");
        exit(0);
    }

    log_file = fopen("log", "a+");

    // Loop infinito per accettare nuovi client
    while(1) {   

        printf("Server in ascolto\n");

        addr_size = sizeof(SA_IN);

        // Accetto il pacchetto mandato dal client
        client_socket = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
        if(client_socket < 0) {
            printf("Il server non è riuscito ad accettare\n");
            exit(0);
        }

        printf("Client accettato!\n");

        pthread_t thread;
        
        /**
         * Creo un puntatore a int per passare l'argomento alla funzione thread
         * questo perchè le pthread vuole che la funzione prenda in input un puntatore
         * di tipo void che poi casteremo a puntatore int per ottenere il valore
         * passato in argomento che sarebbe la client socket
         */
        int *client = malloc(2 * sizeof(int));
        *client = client_socket;

        // Avviamo il thread
        pthread_create(&thread, NULL, execOperations, client);
    }
    
    // Chiudo la connesione
    close(server_socket);

    return 0;
}

void *execOperations(void *pclient) {

    /**
     * Castiamo il puntatore void a int per ottenere
     * la socket del client e liberiamo la memoria
     * usata dal puntatore
     */
    int client = *((int *) pclient);
    free(pclient);

    TS spec;
    Operation op;
    Response res;

    while(1) {

        // Riceviamo l'operazione da fare
        if(recv(client, &op, sizeof(Operation), 0) <= 0) {
            printf("Il client ha chiuso la connesione\n");
            close(client);
            // pthread_exit(NULL);
            return NULL;
        }

        // Prendiamo il tempo alla ricezione
        clock_gettime(CLOCK_REALTIME, &spec);
        res.ricezione = round(spec.tv_nsec / 1.0e6);

        // Controlliamo che operazione dobbiamo fare
        switch(op.operator) {
            case '+':
                res.result = op.firstValue + op.secondValue;
                break;
            case '-':
                res.result = op.firstValue - op.secondValue;
                break;
            case '*':
                res.result = op.firstValue * op.secondValue;
                break;
            case '/':
                res.result = op.firstValue / op.secondValue;
                break;
            default:
                break;
        }

        // Prendiamo il tempo all'invio
        clock_gettime(CLOCK_REALTIME, &spec);
        res.invio = round(spec.tv_nsec / 1.0e6);

        // Inviamo la risposta al client
        send(client, &res, sizeof(Response), 0);
    
        pthread_mutex_lock(&lock);
        
        compileLog(op, res, 0);
    
        pthread_mutex_unlock(&lock);
    }
}

void compileLog(Operation op, Response res, int ip) {

    // Converto per ottenere l'IP
    char *addr;
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, INET_ADDRSTRLEN);

    // Scrivo il log per il calcolo appena svolto
    fprintf(log_file, "IP: %s \nOperazione: %f %c %f \nRisultato: %f \nTempo inizio: %ld \tTempo fine: %ld \n-------------------------------\n", addr, op.firstValue, op.operator, op.secondValue, res.result, res.ricezione, res.invio);

}

void interruptionHandler(int n) {
    fclose(log_file);
    exit(0);
}