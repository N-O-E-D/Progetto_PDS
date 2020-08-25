# Client
### Istruzioni per l'uso
* La cartella contiene solo il CMakeList.txt, quindi prima di cominciare ricarare il cmake (tasto destro -> Reload Cmake Project).
* Nel main ho scritto due test per controllare che Boost e OpenSSL funzionino.   

#### PRODUCER
* Esegue File System Watcher che aggiunge le modifiche all'interno della coda;
* All'inizializzazione, contatta il server per sapere quali file sono sincronizzati;
* Di default non sincronizzati => quando riesco a contattare il server inserisco nella codae l'aggiunta/modifica dei file non ancora sincronizzati

#### CONSUMER
* Crea una comunicazione con il server e appena la apre manda il comando di sincronizzazione;
* Processa le entry nella coda con la modifica corrispondente.
* La entry va eliminata dalla coda solo in caso di successo.

#### IMPLEMENTAZIONE
* Classe con parametri:
    * std::string path;
    * Status pathStatus;
* Coda thread safe che possa specializzarsi in questa classe;
* Aggiunta dello stato di sincronizzazione all'interno della mappa del FSW;

Coda thread safe => boost::lockfree::spsc_queue (grandezza limitata);

Classe => std::pair<std::string, Status>;

#####MAIN:
1. Avviare File System Watcher (producer);
2. Sincornizzazione;
5. Consumer;
6. Gestione terminazione?

Mappa thread safe per salvare i dati della sincronizzazione.

### class PathStatusMap
Thread Safe class wrapping a map that contains path as key and sync status as value.
Constructor receive the root path and populates the map with all the sub paths, with the default value NotSynced

### TODO
* autenticazione -> riprovare/fermare il programma in caso di fallimento
* gestione degli errori
* sistemazione del codice 