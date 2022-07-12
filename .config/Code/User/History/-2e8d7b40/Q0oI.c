#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

// File di log
FILE *log_file;

// Struttura dati del client
SA_IN client_addr;

// Mutex per scrittura su file log
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Socket server
int server_socket;

/*
 * Funzione che esegue le operazioni richieste dai 
 * client adattata per funzionare con i thread
 */
void *exec_operations(void *pclient);

/*
 * Funzione che gestisce la scrittura su file
 * di log per le operazioni compiute 
 */
void fill_log(Operation op, Response res, int ip);

// Funzione per la gestione del segnale di interrupt
void interruption_handler(int n);

int main(void) {

    SA_IN server_addr;
    int client_socket, addr_size;

    // Signal handler per SIGINT
    signal(SIGINT, interruption_handler);

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

    // Apertura file di log in append
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

        // Avvio il thread
        pthread_create(&thread, NULL, exec_operations, client);
    }

    return 0;
}
// Funzione che svolge tutte le operazioni
void *exec_operations(void *pclient) {

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
        res.ricezione = spec.tv_nsec;

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
				// Gestione operazioni non definite
                break;
        }

        // Prendiamo il tempo all'invio
        clock_gettime(CLOCK_REALTIME, &spec);
        res.invio = spec.tv_nsec;

        // Inviamo la risposta al client
        send(client, &res, sizeof(Response), 0);
    
        // Inizio zona critica
        pthread_mutex_lock(&lock);
        
		// Funzione per scrivere il file di log
        fill_log(op, res, 0);
    
        // Fine zona critica
        pthread_mutex_unlock(&lock);
    }
}

// Funzione per scrivere il file di log
void fill_log(Operation op, Response res, int ip) {

    // Converto per ottenere l'IP
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, INET_ADDRSTRLEN);
    unsigned short port = ntohs(client_addr.sin_port);

    // Scrivo il log per il calcolo appena svolto
    fprintf(log_file, "IP: %s:%d \nOperazione: %f %c %f \nRisultato: %f \nTempo inizio: %ld \tTempo fine: %ld \n/--------------------------------------------/\n", addr, port, op.firstValue, op.operator, op.secondValue, res.result, res.ricezione, res.invio);

}

// Funzione per la gestione del segnale di interrupt
void interruption_handler(int n) {
    
    // Chiudo il file di log
    fclose(log_file);

    // Chiudo la connesione
    close(server_socket);

    printf("\n");
    exit(0);
}
