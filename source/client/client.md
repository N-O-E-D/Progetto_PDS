# Client
### Istruzioni pe l'uso
* La cartella contiene solo il CMakeList.txt, quindi prima di cominciare ricarare il cmake (tasto destro -> Reload Cmake Project).
* Nel main ho scritto due test per controllare che Boost e OpenSSL funzionino.

### Todo
* Due thread: uno per il FileWatcher e uno per comunicazione con server.
* Modifica del FileWatcher con inserimento dei seguenti stati:
    * FileCreated
    * FileErased
    * FileModified
    * DirCreated
    * DirModified
* Per ogni modifica rilevata inserisco in una coda path e status e risveglio il thread di comunicazione
* Thread di comunicazione apre un socket e legge la coda: in base allo status chiama la corrispettiva funzione per inviare un messaggio al server.
* Provare ad implementare gli status FileRenamed e DirRenamed 
