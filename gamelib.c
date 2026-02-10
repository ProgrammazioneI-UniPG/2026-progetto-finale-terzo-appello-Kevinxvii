#include "gamelib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// -------------------------------------------------------------------------
// VARIABILI GLOBALI STATICHE (Visibili solo in questo file)
// -------------------------------------------------------------------------

static struct Giocatore* giocatori[4]; // Array di puntatori ai giocatori
static int n_giocatori = 0;
static int gioco_impostato = 0;        // Flag per verificare se il gioco è pronto

// Puntatori alle teste delle due mappe
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

static int mappa_chiusa = 0; // 1 se la creazione mappa è terminata
static int undici_scelto = 0; // Flag per verificare se Undici è già stata scelta

// Array statico per i nomi degli ultimi 3 vincitori
static char vincitori[3][50] = {"Nessuno", "Nessuno", "Nessuno"};

// -------------------------------------------------------------------------
// PROTOTIPI DELLE FUNZIONI STATICHE (Interne)
// -------------------------------------------------------------------------

// Funzioni di utilità
static int lancia_dado(int facce);
static void svuota_buffer();
static const char* nome_zona(Tipo_zona t);
static const char* nome_nemico(Tipo_nemico t);
static const char* nome_oggetto(Tipo_oggetto t);
static void aggiungi_vincitore(char* nome);

// Funzioni per la mappa
static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void stampa_zona_dettaglio(); // Stampa zona specifica (menu imposta)
static void chiudi_mappa();
static void dealloca_mappa();

// Funzioni per il gioco (Azioni)
static void avanza(struct Giocatore* g);
static void indietreggia(struct Giocatore* g);
static void cambia_mondo(struct Giocatore* g);
static void combatti(struct Giocatore* g);
static void raccogli_oggetto(struct Giocatore* g);
static void utilizza_oggetto(struct Giocatore* g);
static void stampa_giocatore(struct Giocatore* g);
static void stampa_zona_corrente(struct Giocatore* g);

// -------------------------------------------------------------------------
// IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE (Quelle in gamelib.h)
// -------------------------------------------------------------------------

void imposta_gioco() {
    if (gioco_impostato) {
        printf("Resetto il gioco precedente...\n");
        termina_gioco(); // Pulisce memoria vecchia
    }
    
    // Reset variabili
    mappa_chiusa = 0;
    undici_scelto = 0;
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    printf("\n=== IMPOSTAZIONE GIOCO ===\n");
    
    // 1. Inserimento numero giocatori
    do {
        printf("Inserisci numero giocatori (1-4): ");
        if (scanf("%d", &n_giocatori) != 1) svuota_buffer();
    } while (n_giocatori < 1 || n_giocatori > 4);
    svuota_buffer();

    // 2. Creazione Giocatori
    for (int i = 0; i < n_giocatori; i++) {
        giocatori[i] = (struct Giocatore*)malloc(sizeof(struct Giocatore));
        
        printf("\n-- Giocatore %d --\n", i + 1);
        printf("Nome: ");
        scanf("%49s", giocatori[i]->nome);
        svuota_buffer();

        // Generazione Stats
        giocatori[i]->attacco_pischico = lancia_dado(20);
        giocatori[i]->difesa_pischica = lancia_dado(20);
        giocatori[i]->fortuna = lancia_dado(20);
        giocatori[i]->mondo = 0; // Parte nel mondo reale
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;
        
        // Inizializza zaino vuoto
        for(int k=0; k<3; k++) giocatori[i]->zaino[k] = nessun_oggetto;

        printf("Stats: Attacco %d, Difesa %d, Fortuna %d\n", 
               giocatori[i]->attacco_pischico, giocatori[i]->difesa_pischica, giocatori[i]->fortuna);

        // Menu scelta classe/modifiche
      // Menu scelta classe/modifiche
        int scelta_classe = 0;
        printf("Scegli classe:\n");
        printf("1. Normale (Nessuna modifica)\n");
        printf("2. Aggressivo (+3 Attacco, -3 Difesa)\n");
        printf("3. Difensivo (-3 Attacco, +3 Difesa)\n");
        if (!undici_scelto) {
            printf("4. Undici VirgolaCinque (+4 Att, +4 Dif, -7 Fortuna) [UNICA]\n");
        }
        
        // --- INIZIO MODIFICA ROBUSTEZZA INPUT ---
        int input_valido = 0;
        int max_opzioni = undici_scelto ? 3 : 4; // Se Undici è presa, max è 3, altrimenti 4

        do {
            printf("Scelta: ");
            
            // 1. Verifica se scanf è riuscita a leggere un intero
            if (scanf("%d", &scelta_classe) != 1) {
                printf("Errore: Inserimento non valido! Devi inserire un numero.\n");
                
                // PULIZIA BUFFER: Rimuove il carattere errato (es. il punto) e tutto il resto della riga
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            } 
            // 2. Verifica se il numero è nel range corretto
            else if (scelta_classe < 1 || scelta_classe > max_opzioni) {
                printf("Errore: Numero non valido. Scegli tra 1 e %d.\n", max_opzioni);
            } 
            // 3. Tutto ok
            else {
                input_valido = 1;
            }
        } while (!input_valido);
        // --- FINE MODIFICA ROBUSTEZZA INPUT ---

        switch (scelta_classe) {
            case 2:
                giocatori[i]->attacco_pischico += 3;
                if(giocatori[i]->difesa_pischica > 3) 
                    giocatori[i]->difesa_pischica -= 3; 
                else 
                    giocatori[i]->difesa_pischica = 0;
                break;
            case 3:
                if(giocatori[i]->attacco_pischico > 3) 
                    giocatori[i]->attacco_pischico -= 3; 
                else 
                    giocatori[i]->attacco_pischico = 0;
                giocatori[i]->difesa_pischica += 3;
                break;
            case 4:
                giocatori[i]->attacco_pischico += 4;
                giocatori[i]->difesa_pischica += 4;
                if(giocatori[i]->fortuna > 7) 
                    giocatori[i]->fortuna -= 7; 
                else 
                    giocatori[i]->fortuna = 0;
                
                // Aggiunge suffisso al nome
                strcat(giocatori[i]->nome, " (11.5)"); 
                undici_scelto = 1;
                break;
        }
    }

    // 3. Creazione Mappa (Menu)
// 3. Creazione Mappa (Menu)
    int scelta_mappa = 0;
    do {
        printf("\n--- MENU CREAZIONE MAPPA ---\n");
        printf("1. Genera Mappa Casuale (15 zone)\n");
        printf("2. Inserisci Zona Manuale\n");
        printf("3. Cancella Zona\n");
        printf("4. Stampa Mappa Completa\n");
        printf("5. Stampa Zona Specifica\n");
        printf("6. Chiudi Mappa e Termina Impostazione\n");
        printf("Scelta: ");
        
        // --- INIZIO CONTROLLO ANTI-LOOP ---
        int risultato_scanf = scanf("%d", &scelta_mappa);

        if (risultato_scanf == 0) {
            // Se scanf ritorna 0, vuol dire che hai inserito un carattere (es. un puntino)
            printf("Errore: Input non valido! Devi inserire un numero.\n");
            
            // PULIZIA BUFFER: Mangia tutto fino all'invio
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            
            scelta_mappa = 0; // Resetta scelta per non entrare in un case a caso
            continue; // Torna all'inizio del loop (ristampa menu)
        }
        // --- FINE CONTROLLO ---

        switch (scelta_mappa) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa(); break;
            case 5: stampa_zona_dettaglio(); break;
            case 6: chiudi_mappa(); break;
            default: 
                printf("Scelta non valida. Inserisci un numero da 1 a 6.\n");
                break;
        }
    } while (!mappa_chiusa);

    gioco_impostato = 1;
    printf("Gioco impostato correttamente!\n");
}

void gioca() {
    if (!gioco_impostato) {
        printf("Errore: Devi prima impostare il gioco!\n");
        return;
    }

    // Posiziona tutti i giocatori all'inizio del Mondo Reale
    for (int i = 0; i < n_giocatori; i++) {
        giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
        giocatori[i]->pos_soprasotto = prima_zona_soprasotto; // Solo come riferimento, sono nel mondo 0
        giocatori[i]->mondo = 0;
    }

    int gioco_finito = 0;
    
    while (!gioco_finito) {
        // Mischia l'ordine dei giocatori (semplificato: turno circolare randomizzato all'inizio del round?)
        // La traccia dice: "scegliere a caso chi muove, poi tutti giocano, poi si rimescola"
        
        int ordine[4];
        for(int i=0; i<n_giocatori; i++) ordine[i] = i;
        
        // Shuffle semplice
        for(int i=0; i<n_giocatori; i++) {
            int r = rand() % n_giocatori;
            int temp = ordine[i];
            ordine[i] = ordine[r];
            ordine[r] = temp;
        }

        printf("\n=== NUOVO ROUND ===\n");
        
        for (int k = 0; k < n_giocatori; k++) {
            struct Giocatore* g = giocatori[ordine[k]];
            
            // Se il giocatore è morto (es. attacco_pischico a 0 o flag morto? La traccia non dice HP, ma "può morire")
            // Usiamo difesa_pischica come vita? O aggiungiamo un campo vita? 
            // La traccia non specifica "punti vita", ma dice "può morire".
            // Assumeremo: Se Difesa Pischica <= 0, sei morto.
            if (g->difesa_pischica <= 0) {
                printf("Il giocatore %s è MORTO e salta il turno.\n", g->nome);
                continue;
            }

            int turno_finito = 0;
            int ha_avanzato = 0; // Per vincolo cambia_mondo

            while (!turno_finito && !gioco_finito) {
                printf("\nTurno di %s (Mondo: %s)\n", g->nome, g->mondo == 0 ? "Reale" : "SOPRASOTTO");
                printf("1. Avanza\n");
                printf("2. Indietreggia\n");
                printf("3. Cambia Mondo\n");
                printf("4. Combatti\n");
                printf("5. Raccogli Oggetto\n");
                printf("6. Utilizza Oggetto\n");
                printf("7. Stampa Giocatore\n");
                printf("8. Stampa Zona\n");
                printf("9. Passa Turno\n");
                printf("Scelta: ");
                
                int azione;
                if (scanf("%d", &azione) != 1) { svuota_buffer(); continue; }

                switch (azione) {
                    case 1: 
                        if (!ha_avanzato) { avanza(g); ha_avanzato = 1; }
                        else printf("Hai già mosso in questo turno!\n");
                        break;
                    case 2: 
                        if (!ha_avanzato) { indietreggia(g); ha_avanzato = 1; }
                        else printf("Hai già mosso in questo turno!\n");
                        break;
                    case 3: 
                        if (!ha_avanzato) { cambia_mondo(g); ha_avanzato = 1; }
                        else printf("Non puoi cambiare mondo dopo aver mosso!\n");
                        break;
                    case 4: combatti(g); break;
                    case 5: raccogli_oggetto(g); break;
                    case 6: utilizza_oggetto(g); break;
                    case 7: stampa_giocatore(g); break;
                    case 8: stampa_zona_corrente(g); break;
                    case 9: turno_finito = 1; break;
                }

                // Controllo vittoria (Demotorzone sconfitto)
                // Nel mio combatti, se vinci contro Demotorzone, imposto un flag globale o ritorno valore?
                // Controllo semplice: se il gioco è finito, esci dai loop
                // (Implementato dentro combatti che chiama termina_gioco e exit o simili? Meglio flag)
            }
        }
        
        // Controllo se tutti morti
        int tutti_morti = 1;
        for(int i=0; i<n_giocatori; i++) if (giocatori[i]->difesa_pischica > 0) tutti_morti = 0;
        
        if (tutti_morti) {
            printf("Tutti i giocatori sono morti! GAME OVER.\n");
            gioco_finito = 1;
            termina_gioco();
        }
    }
}

void termina_gioco() {
    printf("\nPulizia memoria in corso...\n");
    dealloca_mappa();
    
    for (int i = 0; i < n_giocatori; i++) {
        if (giocatori[i]) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    n_giocatori = 0;
    gioco_impostato = 0;
    mappa_chiusa = 0;
    printf("Gioco terminato.\n");
}

void crediti() {
    printf("\n--- CREDITI ---\n");
    printf("Ultimi vincitori:\n");
    for(int i=0; i<3; i++) printf("- %s\n", vincitori[i]);
}

// -------------------------------------------------------------------------
// IMPLEMENTAZIONE FUNZIONI STATICHE (MAPPA)
// -------------------------------------------------------------------------

static void genera_mappa() {
    dealloca_mappa(); // Pulisce se c'era già
    printf("Generazione mappa casuale in corso...\n");

    struct Zona_mondoreale* prec_mr = NULL;
    struct Zona_soprasotto* prec_ss = NULL;
    int demotorzone_piazzato = 0;

    for (int i = 0; i < 15; i++) {
        // Allocazione nodi
        struct Zona_mondoreale* nuovo_mr = malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuovo_ss = malloc(sizeof(struct Zona_soprasotto));

        // Tipo zona (uguale per entrambi)
        Tipo_zona tipo = rand() % 10;
        nuovo_mr->tipo = tipo;
        nuovo_ss->tipo = tipo;

        // Nemici e Oggetti Mondo Reale
        nuovo_mr->nemico = (rand() % 100 < 30) ? (rand() % 2 + 1) : nessun_nemico; // 30% nemico (billi o democane)
        nuovo_mr->oggetto = (rand() % 100 < 40) ? (rand() % 4 + 1) : nessun_oggetto;

        // Nemici Soprasotto
        // Demotorzone solo qui. Se siamo all'ultima stanza e non c'è ancora, mettiamolo.
        if (i == 14 && !demotorzone_piazzato) {
            nuovo_ss->nemico = demotorzone;
            demotorzone_piazzato = 1;
        } else {
            int roll = rand() % 100;
            if (roll < 5 && !demotorzone_piazzato) { // 5% probabilità demotorzone
                nuovo_ss->nemico = demotorzone;
                demotorzone_piazzato = 1;
            } else if (roll < 40) {
                nuovo_ss->nemico = democane; // Solo democane o demotorzone qui
            } else {
                nuovo_ss->nemico = nessun_nemico;
            }
        }

        // Link verticali (Dimensionali)
        nuovo_mr->link_soprasotto = nuovo_ss;
        nuovo_ss->link_mondoreale = nuovo_mr;

        // Link orizzontali (Lista)
        nuovo_mr->avanti = NULL;
        nuovo_mr->indietro = prec_mr;
        nuovo_ss->avanti = NULL;
        nuovo_ss->indietro = prec_ss;

        if (prec_mr) {
            prec_mr->avanti = nuovo_mr;
            prec_ss->avanti = nuovo_ss;
        } else {
            prima_zona_mondoreale = nuovo_mr;
            prima_zona_soprasotto = nuovo_ss;
        }

        prec_mr = nuovo_mr;
        prec_ss = nuovo_ss;
    }
    printf("Mappa generata (15 zone). Demotorzone %s.\n", demotorzone_piazzato ? "presente" : "ASSENTE (Errore gen)");
}

static void inserisci_zona() {
    struct Zona_mondoreale* curr_mr = prima_zona_mondoreale;
    int count = 0;
    
    // Contiamo quante zone ci sono
    while (curr_mr) { count++; curr_mr = curr_mr->avanti; }

    int posizione;
    printf("In quale posizione vuoi inserire la zona? (0 - %d): ", count);
    scanf("%d", &posizione);
    svuota_buffer();

    if (posizione < 0 || posizione > count) {
        printf("Posizione non valida.\n");
        return;
    }

    // 1. Creazione dei nuovi nodi
    struct Zona_mondoreale* nuovo_mr = malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuovo_ss = malloc(sizeof(struct Zona_soprasotto));

    // 2. Impostazione manuale dei dati (Richiesto dal PDF)
    int scelta_tipo, scelta_nemico, scelta_oggetto;
    
    printf("Scegli Tipo Zona (0-9): ");
    scanf("%d", &scelta_tipo);
    nuovo_mr->tipo = (Tipo_zona)scelta_tipo;
    nuovo_ss->tipo = (Tipo_zona)scelta_tipo;

    printf("Scegli Nemico (0=Nessuno, 1=Billi, 2=Democane, 3=Demotorzone): ");
    scanf("%d", &scelta_nemico);
    // Nota: Il PDF dice di inserire nemico/oggetto manualmente
    nuovo_mr->nemico = (Tipo_nemico)scelta_nemico;
    nuovo_ss->nemico = (Tipo_nemico)scelta_nemico; // Copiamo lo stesso per semplicità o chiediamo due volte? 
    // Il PDF dice "generare anche nemico... chiedendo all'utente". 
    // Mettiamo lo stesso tipo base, ma gestiamo eccezioni (Billi solo MR, Demotor solo SS) automaticamente se necessario.

    printf("Scegli Oggetto (0-4): ");
    scanf("%d", &scelta_oggetto);
    nuovo_mr->oggetto = (Tipo_oggetto)scelta_oggetto;
    // Il Soprasotto NON ha oggetti [cite: 35]

    // 3. Collegamento Verticale
    nuovo_mr->link_soprasotto = nuovo_ss;
    nuovo_ss->link_mondoreale = nuovo_mr;

    // 4. Inserimento nella Lista (Logica dei puntatori)
    if (posizione == 0) {
        // Inserimento in TESTA
        nuovo_mr->avanti = prima_zona_mondoreale;
        nuovo_mr->indietro = NULL;
        nuovo_ss->avanti = prima_zona_soprasotto;
        nuovo_ss->indietro = NULL;

        if (prima_zona_mondoreale) {
            prima_zona_mondoreale->indietro = nuovo_mr;
            prima_zona_soprasotto->indietro = nuovo_ss;
        }
        prima_zona_mondoreale = nuovo_mr;
        prima_zona_soprasotto = nuovo_ss;
    } else {
        // Inserimento in MEZZO o CODA
        struct Zona_mondoreale* temp_mr = prima_zona_mondoreale;
        struct Zona_soprasotto* temp_ss = prima_zona_soprasotto;

        // Scorro fino a quello PRIMA della posizione desiderata
        for (int i = 0; i < posizione - 1; i++) {
            temp_mr = temp_mr->avanti;
            temp_ss = temp_ss->avanti;
        }

        // Collego il nuovo al successivo del corrente
        nuovo_mr->avanti = temp_mr->avanti;
        nuovo_ss->avanti = temp_ss->avanti;

        // Se c'era un successivo, il suo 'indietro' ora punta al nuovo
        if (temp_mr->avanti) {
            temp_mr->avanti->indietro = nuovo_mr;
            temp_ss->avanti->indietro = nuovo_ss;
        }

        // Collego il precedente al nuovo
        temp_mr->avanti = nuovo_mr;
        temp_ss->avanti = nuovo_ss;
        
        // Collego il nuovo al precedente
        nuovo_mr->indietro = temp_mr;
        nuovo_ss->indietro = temp_ss;
    }
    printf("Zona inserita con successo!\n");
}

static void cancella_zona() {
    if (prima_zona_mondoreale == NULL) {
        printf("Mappa vuota.\n");
        return;
    }

    struct Zona_mondoreale* curr_mr = prima_zona_mondoreale;
    int count = 0;
    while (curr_mr) { count++; curr_mr = curr_mr->avanti; }

    int posizione;
    printf("Quale zona cancellare? (0 - %d): ", count - 1);
    scanf("%d", &posizione);
    svuota_buffer();

    if (posizione < 0 || posizione >= count) {
        printf("Posizione non valida.\n");
        return;
    }

    struct Zona_mondoreale* da_canc_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* da_canc_ss = prima_zona_soprasotto;

    // Raggiungiamo il nodo da cancellare
    for (int i = 0; i < posizione; i++) {
        da_canc_mr = da_canc_mr->avanti;
        da_canc_ss = da_canc_ss->avanti;
    }

    // Caso 1: Cancelliamo la TESTA (la prima zona)
    if (da_canc_mr == prima_zona_mondoreale) {
        prima_zona_mondoreale = da_canc_mr->avanti;
        prima_zona_soprasotto = da_canc_ss->avanti;
        
        if (prima_zona_mondoreale) {
            prima_zona_mondoreale->indietro = NULL;
            prima_zona_soprasotto->indietro = NULL;
        }
    } 
    else {
        // Caso 2: Cancelliamo in MEZZO o CODA
        // Il nodo PRECEDENTE deve puntare al SUCCESSIVO del nodo da cancellare
        da_canc_mr->indietro->avanti = da_canc_mr->avanti;
        da_canc_ss->indietro->avanti = da_canc_ss->avanti;

        // Se esiste un SUCCESSIVO, il suo 'indietro' deve puntare al PRECEDENTE
        if (da_canc_mr->avanti) {
            da_canc_mr->avanti->indietro = da_canc_mr->indietro;
            da_canc_ss->avanti->indietro = da_canc_ss->indietro;
        }
    }

    // Importante: Liberare la memoria [cite: 82, 141]
    free(da_canc_mr);
    free(da_canc_ss);

    printf("Zona cancellata con successo.\n");
}

static void stampa_mappa() {
    struct Zona_mondoreale* curr = prima_zona_mondoreale;
    int i = 0;
    while (curr) {
        printf("Zona %d: %s | Nemico MR: %s | Nemico SS: %s\n", 
            i++, nome_zona(curr->tipo), nome_nemico(curr->nemico), nome_nemico(curr->link_soprasotto->nemico));
        curr = curr->avanti;
    }
}

static void stampa_zona_dettaglio() {
    printf("Stampa dettaglio prima zona:\n");
    if (prima_zona_mondoreale) {
        printf("Tipo: %s\n", nome_zona(prima_zona_mondoreale->tipo));
    }
}

static void chiudi_mappa() {
    // Verifica vincoli: 15 zone e 1 demotorzone
    int count = 0;
    int demo = 0;
    struct Zona_soprasotto* curr = prima_zona_soprasotto;
    while (curr) {
        count++;
        if (curr->nemico == demotorzone) demo++;
        curr = curr->avanti;
    }

    if (count >= 15 && demo == 1) {
        mappa_chiusa = 1;
        printf("Mappa chiusa correttamente.\n");
    } else {
        printf("Errore: La mappa deve avere almeno 15 zone e ESATTAMENTE 1 Demotorzone (trovati: %d zone, %d demo).\n", count, demo);
        mappa_chiusa = 0;
    }
}

static void dealloca_mappa() {
    struct Zona_mondoreale* curr_mr = prima_zona_mondoreale;
    while (curr_mr) {
        struct Zona_mondoreale* temp = curr_mr;
        curr_mr = curr_mr->avanti;
        // Libera anche il soprasotto collegato
        free(temp->link_soprasotto); 
        free(temp);
    }
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
}

// -------------------------------------------------------------------------
// IMPLEMENTAZIONE AZIONI DI GIOCO
// -------------------------------------------------------------------------

static void avanza(struct Giocatore* g) {
    // Controllo nemici
    Tipo_nemico nemico_qui;
    if (g->mondo == 0) nemico_qui = g->pos_mondoreale->nemico;
    else nemico_qui = g->pos_soprasotto->nemico;

    if (nemico_qui != nessun_nemico) {
        printf("Non puoi avanzare! C'è un %s che ti blocca. Devi combattere!\n", nome_nemico(nemico_qui));
        return;
    }

    // Movimento
    if (g->mondo == 0) {
        if (g->pos_mondoreale->avanti) {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            g->pos_soprasotto = g->pos_mondoreale->link_soprasotto; // Allinea puntatore ombra
            printf("%s avanza nel Mondo Reale.\n", g->nome);
        } else printf("Sei all'ultima zona, non puoi avanzare!\n");
    } else {
        if (g->pos_soprasotto->avanti) {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
            printf("%s avanza nel SOPRASOTTO.\n", g->nome);
        } else printf("Sei all'ultima zona, non puoi avanzare!\n");
    }
}

static void indietreggia(struct Giocatore* g) {
    // Stessa logica di avanza ma con 'indietro'
    Tipo_nemico nemico_qui;
    if (g->mondo == 0) nemico_qui = g->pos_mondoreale->nemico;
    else nemico_qui = g->pos_soprasotto->nemico;

    if (nemico_qui != nessun_nemico) {
        printf("Non puoi scappare! C'è un %s. Devi combattere!\n", nome_nemico(nemico_qui));
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro) {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            g->pos_soprasotto = g->pos_mondoreale->link_soprasotto;
            printf("Sei tornato indietro.\n");
        } else printf("Sei all'inizio!\n");
    } else {
        if (g->pos_soprasotto->indietro) {
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
            printf("Sei tornato indietro (Soprasotto).\n");
        } else printf("Sei all'inizio!\n");
    }
}

static void cambia_mondo(struct Giocatore* g) {
    if (g->mondo == 0) {
        // Da Reale a Soprasotto: Controllo nemici
        if (g->pos_mondoreale->nemico != nessun_nemico) {
            printf("Non puoi cambiare mondo con un nemico presente!\n");
            return;
        }
        g->mondo = 1;
        printf("Whoosh! Sei finito nel SOPRASOTTO!\n");
    } else {
        // Da Soprasotto a Reale: Check Fortuna
        int tiro = lancia_dado(20);
        printf("Tiro Fortuna: %d (La tua Fortuna: %d)\n", tiro, g->fortuna);
        if (tiro < g->fortuna) {
            g->mondo = 0;
            printf("Sei riuscito a scappare nel Mondo Reale!\n");
        } else {
            printf("Fallito! Sei bloccato nel Soprasotto!\n");
        }
    }
}

static void combatti(struct Giocatore* g) {
    Tipo_nemico* nemico_ptr; // Puntatore al campo nemico nella struct zona
    
    if (g->mondo == 0) nemico_ptr = &(g->pos_mondoreale->nemico);
    else nemico_ptr = &(g->pos_soprasotto->nemico);

    if (*nemico_ptr == nessun_nemico) {
        printf("Non c'è nessuno da combattere qui.\n");
        return;
    }

    printf("COMBATTIMENTO contro %s!\n", nome_nemico(*nemico_ptr));
    
    // Logica semplificata: Tiro Dado + Attacco vs Tiro Dado Nemico
    // Se vinci, nemico muore (o scappa). Se perdi, perdi Difesa.
    
    int forza_nemico = 0;
    if (*nemico_ptr == billi) forza_nemico = 5;
    if (*nemico_ptr == democane) forza_nemico = 10;
    if (*nemico_ptr == demotorzone) forza_nemico = 15;

    int danno_g = lancia_dado(6) + g->attacco_pischico;
    int danno_n = lancia_dado(6) + forza_nemico;

    printf("Tuo attacco: %d | Nemico: %d\n", danno_g, danno_n);

    if (danno_g >= danno_n) {
        printf("Hai vinto lo scontro!\n");
        if (*nemico_ptr == demotorzone) {
            printf("\n!!! HAI SCONFITTO IL DEMOTORZONE !!!\n");
            printf("!!! %s HA VINTO LA PARTITA !!!\n", g->nome);
            aggiungi_vincitore(g->nome);
            // Termina gioco brutale per ora
            termina_gioco();
            exit(0);
        }
        
        // 50% probabilità che il nemico scompaia
        if (lancia_dado(100) < 50) {
            printf("Il nemico è fuggito/morto definitivamente!\n");
            *nemico_ptr = nessun_nemico;
        } else {
            printf("Il nemico è a terra ma non è morto (rimane nella zona).\n");
        }
    } else {
        int ferita = danno_n - danno_g;
        // Riduce difesa pischica (usata come vita)
        if (g->difesa_pischica > ferita) g->difesa_pischica -= ferita;
        else g->difesa_pischica = 0;
        
        printf("Sei stato ferito! Difesa residua: %d\n", g->difesa_pischica);
    }
}

static void raccogli_oggetto(struct Giocatore* g) {
    if (g->mondo == 1) {
        printf("Non ci sono oggetti nel Soprasotto.\n");
        return;
    }
    if (g->pos_mondoreale->oggetto == nessun_oggetto) {
        printf("Qui non c'è niente.\n");
        return;
    }
    
    // Cerca spazio zaino
    for(int i=0; i<3; i++) {
        if (g->zaino[i] == nessun_oggetto) {
            g->zaino[i] = g->pos_mondoreale->oggetto;
            printf("Hai raccolto: %s\n", nome_oggetto(g->zaino[i]));
            g->pos_mondoreale->oggetto = nessun_oggetto;
            return;
        }
    }
    printf("Zaino pieno!\n");
}

static void utilizza_oggetto(struct Giocatore* g) {
    printf("Zaino:\n");
    for(int i=0; i<3; i++) printf("%d. %s\n", i+1, nome_oggetto(g->zaino[i]));
    
    int scelta;
    printf("Quale usare? (0 annulla): ");
    scanf("%d", &scelta);
    if (scelta < 1 || scelta > 3 || g->zaino[scelta-1] == nessun_oggetto) return;

    Tipo_oggetto obj = g->zaino[scelta-1];
    printf("Usi %s...\n", nome_oggetto(obj));
    
    // Effetti semplici
    switch(obj) {
        case maglietta_fuocoinferno: 
            if(g->difesa_pischica < 20) g->difesa_pischica++; 
            printf("Ti senti protetto (+1 Difesa)\n");
            break;
        case bussola:
            printf("La bussola indica che il Demotorzone è... da qualche parte nel Soprasotto.\n");
            break;
        case schitarrata_metallica:
            printf("SUONI UN ASSOLO EPICO! (Attacco +2 temporaneo o cura? Facciamo cura)\n");
            g->difesa_pischica += 2;
            break;
        default: printf("Non succede nulla di speciale.\n");
    }
    
    g->zaino[scelta-1] = nessun_oggetto; // Consumato
}

static void stampa_giocatore(struct Giocatore* g) {
    printf("Giocatore: %s | HP(Dif): %d | Atk: %d | Fort: %d\n", 
           g->nome, g->difesa_pischica, g->attacco_pischico, g->fortuna);
}

static void stampa_zona_corrente(struct Giocatore* g) {
    if (g->mondo == 0) {
        printf("Mondo Reale: %s. Nemico: %s. Oggetto: %s\n", 
               nome_zona(g->pos_mondoreale->tipo), 
               nome_nemico(g->pos_mondoreale->nemico),
               nome_oggetto(g->pos_mondoreale->oggetto));
    } else {
        printf("SOPRASOTTO: %s. Nemico: %s.\n", 
               nome_zona(g->pos_soprasotto->tipo), 
               nome_nemico(g->pos_soprasotto->nemico));
    }
}

// -------------------------------------------------------------------------
// UTILITÀ
// -------------------------------------------------------------------------
static int lancia_dado(int facce) { return rand() % facce + 1; }
static void svuota_buffer() { while (getchar() != '\n'); }
static const char* nome_zona(Tipo_zona t) {
    const char* nomi[] = {"Bosco", "Scuola", "Laboratorio", "Caverna", "Strada", "Giardino", "Supermercato", "Centrale", "Deposito", "Polizia"};
    return nomi[t];
}
static const char* nome_nemico(Tipo_nemico t) {
    if (t==billi) return "Billi";
    if (t==democane) return "Democane";
    if (t==demotorzone) return "DEMOTORZONE";
    return "Nessuno";
}
static const char* nome_oggetto(Tipo_oggetto t) {
    if (t==bicicletta) return "Bicicletta";
    if (t==maglietta_fuocoinferno) return "Maglietta Hellfire";
    if (t==bussola) return "Bussola";
    if (t==schitarrata_metallica) return "Chitarra";
    return "Vuoto";
}
static void aggiungi_vincitore(char* nome) {
    strcpy(vincitori[2], vincitori[1]);
    strcpy(vincitori[1], vincitori[0]);
    strcpy(vincitori[0], nome);
}