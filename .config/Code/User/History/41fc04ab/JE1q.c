#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Definisco TRUE e FALSE per una maggiore facilità nel
 * creare espressioni booleane e per un maggiore leggibilità
 */
#define TRUE 1
#define FALSE 0

/*
 * Definisco MAX e MIN sempre per un maggiore leggibilità
 * del codice. Potrei anche scrivere direttamente due funzioni
 * per svolgere questo compito ma credo che svilupparlo così
 * lo renda più leggibile e compatto
 */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// Funzione che controlla se un stringa inizia con un determinato carattere
int starts_with_char(char *string, char character);

// Funzione per verificare la correttezza dei parametri
int validation(int argc, char *argv[], char *args[]);

// Funzione per calcolare
int count_rows(FILE *file);

// Bufferizzo le N righe oppure le righe fino all'EOF
void buffer_n_rows(char *rows[], int max_lines, FILE *file);

void free_buffers(char *buffer[], int size);

void exec_subroutine(char *args[], FILE *infile1, FILE *infile2, char *namefile1, char *namefile2, int max, FILE *outfile);

/*
 * Funzione che quando F1 e F2 sono diversi in almeno una riga
 * restituisce la stringa ”F1 ed F2 sono differenti"
 */
int subroutine_g(char *string1, char *string2, char *namefile1, char *namefile2, FILE *outfile);

/*
 * Funzione che quando F1 e F2 sono uguali in tutte le righe
 * restituisce la stringa “F1 ed F2 sono uguali”
 */
int subroutine_s(char *string1, char *string2);

// Funzione che restituisce i numeri delle righe differenti
void subroutine_d(char *string1, char *string2,char *namefile1, char *namefile2, int line, int specified, FILE *outfile);

// Funzione che restituisce i numeri delle righe uguali
void subroutine_u(char *string1, char *string2,char *namefile1, char *namefile2, int line, int specified, FILE *outfile);

int main(int argc, char *argv[]) {
    FILE *infile1, *infile2, *outfile = NULL;

    /*
     * Array che include gli argomenti nelle seguenti posizioni
     * 0: -g; 1: -s; 2: -d; 3: -u; 4: -v; 5: -o; 6: Nome del file in uscita
     */
    char *args[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    // Check sugli argomenti passati
    if(validation(argc, argv, args) == FALSE) {
        printf("Non buono \n");
        return 0;
    }

    // Apertura in lettura dei due file
    infile1 = fopen(argv[1], "r");
    infile2 = fopen(argv[2], "r");

    // Trovo il file più corto
    int lenfile1, lenfile2, maxLines;
    lenfile1 = count_rows(infile1);
    lenfile2 = count_rows(infile2);

    maxLines = MIN(lenfile1, lenfile2);

    // Riporto il cursore del file all'inizio
    rewind(infile1);
    rewind(infile2);

    exec_subroutine(args, infile1, infile2, argv[1], argv[2], maxLines, outfile);

    //Chiusura di entrambi i file
    fclose(infile1);
    fclose(infile2);

    return 0;
}

// Funzione che controlla se un stringa inizia con un determinato carattere
int starts_with_char(char *string, char character) {

    // Controllo che gli argomenti siano stati passati
    if(!string || !character) {
        printf("Gli argomenti passati sono NULL\n");
        return FALSE;
    }

    /*
     * Controllo che il primo carattere della stringa
     * si uguale al carattere passato come argomento
     */
    if(string[0] == character) {
        printf("La stringa incomincia con %c\n", character);
        return TRUE;
    }

    return FALSE;
}

// Funzione per verificare la correttezza dei parametri
int validation(int argc, char *argv[], char *args[]) {

    /*
     * Controllo argomenti obligatori da passare
     * Primo argomento: chiamata al programma
     * Secondo argomento: primo file
     * Terzo argomento: secondo file
     * Quarto argomento: un'operando qualsiasi
     */
    if(argc < 3)
        return FALSE;

    // Serie di check sulla sintassi del comando

    // Check delle funzioni passate
    int d = FALSE;
    int u = FALSE;
    for(int i=3; i<argc; i++) {

        if(strcmp(argv[i], "-g") == 0) {
            args[0] = "-g";
        }

        if(strcmp(argv[i], "-s") == 0) {
            args[1] = "-s";
        }

        // Controllo per vedere se -d può essere presente
        if(strcmp(argv[i], "-d") == 0) {

            // Controllo se -u è già presente
            if(u == TRUE) {
                printf("-u è già presente\n");
                return FALSE;
            }

            // Se non è presente -u allora -d è possibile eseguirlo
            d = TRUE;

            args[2] = "-d";
        }

        // Controllo per vedere se -u può essere presente
        if(strcmp(argv[i], "-u") == 0) {

            // Controllo se -d è già presente
            if(d == TRUE) {
                printf("-d è già presente\n");
                return FALSE;
            }

            // Se non è presente -d allora -u è possibile eseguirlo
            u = TRUE;

            args[3] = "-u";
        }

        if(strcmp(argv[i], "-v") == 0) {
            args[4] = "-v";
        }

        // Controllo per vedere se -o è stato scritto correttamente
        if(strcmp(argv[i], "-o") == 0) {

            // Controllo che posso effettivamente entrare in argv[i+1]
            if(i+1 < argc) {

                // Controllo che dopo -o ci sia il file su cui scrivere l'output
                if(starts_with_char(argv[i+1], '-') == TRUE) {
                    printf("Manca il nome del file\n");
                    return FALSE;
                }

                args[5] = "-o";
                args[6] = argv[i+1];

                // Sorpasso sopra il nome del file di output
                i = i + 1;

            } else {

                // FALSE perchè manca il nome del file
                printf("Manca il nome del file\n");
                return FALSE;
            }

        }

    }

    /*
     * Tutti i controlli sono stati eseguiti e passati
     * quindi il programma è pronto per essere eseguito
     */
    return TRUE;
}

// Funzione per calcolare il numero di righe di un file
int count_rows(FILE *file) {
    char check;
    int rows = 1;

    // Conta le righe fino alla fine del file
    while(!feof(file)) {

        // Legge il prossimo carattere
        check = fgetc(file);

        // Per ogni carattere \n fa un incremento unitario
        if(check == '\n') {
            rows++;
        }
    }

    return rows;
}

// Bufferizzo le N righe oppure le righe fino all'EOF
void buffer_n_rows(char *rows[], int max_lines, FILE *file) {
    int lines = 0;

    char *buffer;
    size_t len = 0;

    do {

        // Metto buffer che punta a NULL
        buffer = NULL;

        // Metto la stringa dentro al buffer, le stringhe devono avere meno di 200 caratteri
        getline(&buffer, &len, file);

        // Alloco la dimensione esatta della stringa più uno per evitare sprechi
        rows[lines] = (char *) calloc(strlen(buffer) + 1, sizeof(char));

        // Copio la stringa dentro al buffer alla posizione i-esima (lines) nell'array
        strcpy(rows[lines], buffer);

        // Incremento il numero delle righe
        lines++;

        // Dealloco lo spazio da buffer per riallocarlo in seguito
        free(buffer);

    } while(!feof(file) && lines < max_lines);
}

/*
 * Funzione che quando F1 e F2 sono diversi in almeno una riga
 * restituisce la stringa ”F1 ed F2 sono differenti"
 */
int subroutine_g(char *string1, char *string2, char *namefile1, char *namefile2, FILE *outfile) {
    if(strcmp(string1, string2) != 0) {
        printf("%s e %s sono differenti\n", namefile1, namefile2);
        return FALSE;
    }
    return TRUE;
}

/*
 * Funzione che quando F1 e F2 sono uguali in tutte le righe
 * restituisce la stringa “F1 ed F2 sono uguali”-
 */
int subroutine_s(char *string1, char *string2) {
    if(strcmp(string1, string2) != 0) {
        return FALSE;
    }
    return TRUE;
}

// Funzione che restituisce i numeri delle righe differenti
void subroutine_d(char *string1, char *string2, char *namefile1, char *namefile2, int line, int specified, FILE *outfile) {
    if(strcmp(string1, string2) != 0) {
        if(specified == TRUE) {
            if(outfile != NULL) {
                fprintf(outfile, "%dRiga, FILENAME %s: %s\n", line, namefile1, string1);
                fprintf(outfile, "%dRiga, FILENAME %s: %s\n", line, namefile2, string2);
                return;
            }
            printf("Riga %d, FILENAME %s: %s\n", line, namefile1, string1);
            printf("Riga %d, FILENAME %s: %s\n", line, namefile2, string2);
            return;
        }
        if(outfile != NULL) {
            fprintf(outfile, "%d\n", line);
            return;
        }
        printf("%d\n", line);
        return;
    }
}

// Funzione che restituisce i numeri delle righe uguali
void subroutine_u(char *string1, char *string2, char *namefile1, char *namefile2, int line, int specified, FILE *outfile) {
    if(strcmp(string1, string2) == 0) {
        if(specified == TRUE) {
            if(outfile != NULL) {
                fprintf(outfile, "%dRiga, FILENAME %s: %s\n", line, namefile1, string1);
                fprintf(outfile, "%dRiga, FILENAME %s: %s\n", line, namefile2, string2);
                return;
            }
            printf("Riga %d, FILENAME %s: %s\n", line, namefile1, string1);
            printf("Riga %d, FILENAME %s: %s\n", line, namefile2, string2);
            return;
        }
        if(outfile != NULL) {
            fprintf(outfile, "%d\n", line);
            return;
        }
        printf("%d\n", line);
        return;
    }
}

void exec_subroutine(char *args[], FILE *infile1, FILE *infile2, char *namefile1, char *namefile2, int max, FILE *outfile) {
    // Controllare se esiste il parametro -o e creare il file
    if(args[5] != NULL) {
        outfile = fopen(args[6], "wb");
    }

    // Subroutine da eseguire
    int sg = FALSE, ss = FALSE, sd = FALSE, su = FALSE, sv = FALSE;

    /*
     * Imposto le subroutine da svolgere con i valori pseudo-booleani
     * ricordandoci che l'array include gli argomenti nelle seguenti posizioni
     * 0: -g; 1: -s; 2: -d; 3: -u; 4: -v; 5: -o; 6: Nome del file in uscita
     */
    if (args[0] != NULL) {
        sg = TRUE;
    }

    if (args[1] != NULL) {
        ss = TRUE;
    }

    if (args[2] != NULL) {
        sd = TRUE;
    }

    if (args[3] != NULL) {
        su = TRUE;
    }

    if (args[4] != NULL) {
        sv = TRUE;
    }

    // Buffer che memorizzano solo 50 righe alla volta
    char *buffer1[50], *buffer2[50];

    /*
     * Lines serve per tenere il conto delle righe totali lette
     * iter serve per capire quante righe sono state lette e quante ne vanno iterate
     */
    int lines = max, iter;

    do {
        /*
         * Linee da leggere se non si arriva la massimo (50)
         * se ne legge il numero massimo che il file contiene
         */
        if(lines > 50) {
            buffer_n_rows(buffer1, 50, infile1);
            buffer_n_rows(buffer2, 50, infile2);

            iter = 50;
            lines -= iter;
        } else {
            buffer_n_rows(buffer1, lines, infile1);
            buffer_n_rows(buffer2, lines, infile2);

            iter = lines;
            lines -= iter;
        }

        /*
         * Itera per il numero di righe lette in precedenza
         * Le subroutine vengono utilizzate solo se sono settate a TRUE e nel mentre della lettura
         * posso anche fermarsi se hanno raggiunto la fine prima del termine per evitare sprechi di
         * cicli. Unica eccezione viene fatta sulla -s dove ss indica anche lo stato del file (se uguale o diverso)
         */
        for(int i=0; i<iter; i++) {
            if(sg == TRUE) {
                sg = subroutine_g(buffer1[i], buffer2[i], namefile1, namefile2, outfile);
            }

            if(ss == TRUE) {
                ss = subroutine_s(buffer1[i], buffer2[i]);
            }

            if(sd == TRUE) {
                subroutine_d(buffer1[i], buffer2[i], namefile1, namefile2, (max - (lines + iter) + i + 1), sv, outfile);
            }

            if(su == TRUE) {
                subroutine_u(buffer1[i], buffer2[i], namefile1, namefile2, (max - (lines + iter) + i + 1), sv, outfile);
            }
        }

    } while(lines > 0);

    if(ss == TRUE) {
        if(outfile != NULL) {
            fprintf(outfile, "%s e %s sono uguali\n", namefile1, namefile2);
            return;
        }
        printf("%s e %s sono uguali\n", namefile1, namefile2);
    }
}