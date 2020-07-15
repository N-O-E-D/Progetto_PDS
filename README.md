# Progetto PDS

Il progetto mira a costruire un sistema client-server che esegua un backup incrementale
del contenuto di una cartella (e tutte le sue sottocartelle) sul computer locale su un server remoto. Una volta lanciato, il sistema funzionerà come un servizio in background mantenendo il contenuto della cartella configurata sincronizzata con la copia remota: ogni volta che il contenuto monitorato cambia (vengono aggiunti nuovi file, quelli esistenti vengono modificati e / o rimossi), il comando adatto verrà inviato attraverso la rete per replicare le modifiche sull'altro lato. In caso di errori temporanei (porzionamento della rete o guasto del server remoto), il sistema continuerà a monitorare l'evoluzione della cartella locale e tenterà di sincronizzare nuovamente finchè la condizione di errore transitorio scompare).


### Background e working environment richiesti

Conoscenza delle astrazioni generali di C++ 17 e della libreria STL C++.
Conoscenza di concorrenza, sincronizzazione ed elaborazione in background. Il sistema sarà sviluppato utilizzando librerie di terze parti (ad es. Boost.asio) al fine di
supportare l'implementazione su più piattaforme.


## Definizione del problema

### Architettura d'insieme

Il sistema è costituito da due diversi moduli (il client e il server) che interagiscono tramite una connessione con socket TCP.
Il dialogo si svolge lungo questa connessione permettendo alle due parti di capirsi.
L'insieme dei possibili messaggi, la loro codifica e il loro significato rappresenta il così
chiamato "protocollo a livello di applicazione" che dovrebbe essere definito per primo, quando ci si avvicina a questo tipo di problemi. Il suo contenuto deriva dai requisiti generali del sistema e dovrebbe consentire una adeguata rappresentazione per le entità previste che possono far parte della conversazione (tra gli altri, file - che comprendono il loro nome, il loro contenuto e i loro metadati, cartelle, comandi, risposte, indicazioni di errore, ...). L'insieme di possibili richieste e le risposte corrispondenti dovrebbero essere definite (e probabilmente, perfezionate lungo il progetto, non appena emergono ulteriori dettagli).
Inoltre, ogni messaggio scambiato dovrebbe anche essere correlato alle implicazioni che ha
la parte ricevente, che dovrebbe essere codificata assumendo tale implicazione come requisito obbligatorio.
Per rimanere sincronizzati, sia il client che il server devono avere un'immagine di file monitorati mentre vengono archiviati nel rispettivo file system. Il protocollo dell'applicazione dovrebbe offrire una sorta di "comando sonda" che consente al client di verificare se il server ha già una copia di un determinato file o cartella. Dal momento che il nome e la dimensione non sono abbastanza per provare che il server ha la stessa copia del client, un (robusto) checksum o l'hash dovrebbe essere calcolato per ogni file e inviato al server. Dal momento che potrebbe essere necessario del tempo per il server per calcolare la risposta, il protocollo dovrebbe probabilmente funzionare in modo asincrono cioè senza richiedere una risposta immediata del server prima di emetterne una nuova richiesta ad esso.

### Lato Client

Il lato client è responsabile del monitoraggio continuo di una cartella specifica che può essere specificata in qualsiasi modo ragionevole (parametro della riga di comando, file di configurazione, variabile d'ambiente ...) e verificare che tutti i contenuti siano sincronizzati con il lato sever. Per eseguire questa operazione, si può fare affidamento sulla classe del filesystem fornita con la libreria C ++ 17 o Boost one (https://www.boost.org/doc/libs/1_73_0/libs/filesystem/doc/index.htm). Ogni volta che viene rilevata una discrepanza, l'entry locale corrispondente deve essere contrassegnata come non valida ed è necessario prendere alcune disposizioni per trasferire il file (aggiornato) sul server. Alcuni indicazioni su come creare un watcher del file system sono disponibili qui
(Https://solarianprogrammer.com/2019/01/13/cpp-17-filesystem-write-file-watchermonitor/)

### Lato Server

Il lato server è responsabile dell'ascolto sulla connessione socket e accetta la connessione
richieste dai client. Il server deve essere progettato per gestirne più di un client e deve avere cartelle di backup separate per ciascuno di essi. Quando il canale di comunicazione è impostato, verrà eseguito un controllo dell'identità del client (scambiando un nome e qualche forma di prova di identità) al fine di associare la connessione ai propri parametri di configurazione. Per ogni connessione esistente, i messaggi in arrivo saranno valutati nell'ordine in cui arrivano e verrà generata una risposta adeguata contenente le informazioni richieste o un codice di errore. In caso di successo, qualsiasi conseguenza del comando sarà garantita. In caso di errore, non si verificherà alcuna modifica sul lato server. La comunicazione tra il client e il server può essere basata sulla libreria Boost ASIO (https://www.boost.org/doc/libs/1_73_0/doc/html/boost_asio.html) o qualsiasi altra libreria adatta.




## LINK PROGETTO:
- documento: https://docs.google.com/document/d/1_EYECKDJnXkgcdtO51mPQqZi-ZIg2k6e2CJkWicUiTI/edit?usp=sharing
- jumboard: https://jamboard.google.com/d/1yEwsyDGlrBQfzyoAN9BUJ87L-m39p6McUEE_FGm3Bwo/edit?usp=sharing

## LINK BOOST.ASIO:
- video: https://www.youtube.com/watch?v=rwOv_tw2eA4
- slides: https://github.com/CppCon/CppCon2016/blob/master/Tutorials/Asynchronous%20IO%20with%20Boost.Asio/Asynchronous%20IO%20with%20Boost.Asio%20-%20Michael%20Caisse%20-%20CppCon%202016.pdf

