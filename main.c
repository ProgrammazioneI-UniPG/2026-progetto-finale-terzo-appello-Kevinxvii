#include "gamelib.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h> 

// Questo file contiene solo la definizione della funzione main
// con il menu principale 1-2-3-4

int main() {
    // Dichiarazione delle variabili
    int scelta;
    int continua = 1; 
    
    // Inizializza il generatore di numeri casuali
    srand(time(NULL));

    do {
        printf("\n--- Menu Principale ---\n");
        printf("1. Imposta Gioco\n");
        printf("2. Gioca\n");
        printf("3. Termina Gioco\n");
        printf("4. Visualizza Crediti\n");
        printf("Scelta: ");

        // Validazione input
        if (scanf("%d", &scelta) != 1) {
            printf("Input non valido. Inserisci un numero tra 1 e 4.\n");
            while (getchar() != '\n'); // Svuota il buffer
            continue;
        }

        switch (scelta) {
            case 1:
                imposta_gioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                termina_gioco(); // Pulisce la memoria e saluta
                continua = 0;    // Imposta il flag per uscire dal while
                break;
            case 4:
                crediti();
                break;
            default:
                printf("Scelta non valida. Inserisci un numero tra 1 e 4.\n");
                break;
        }
    } while (continua);

    return 0;
}