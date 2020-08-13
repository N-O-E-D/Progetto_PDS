## Server

Server telnet in ascolto sulla porta 5000 (per metterlo in ascolto anche su altre porte, replicare la riga 157 di main.cpp indicando un altro numero di porta).
Una volta avviato, richiederà l'autenticazione (credenziali presenti in questo documento e anche in main.cpp), dopodichè si potranno inviare comandi 
del tipo "COMANDO path" e il server riconoscerà il comando (quest'ultima parte è provvisoria e scritta solo per prova). In ordine

1) avviare il programma
2) da un terminale, digitare "telnet localhost 5000" (se il server è in ascolto su più porte contemporaneamente, eseguire questo punto su altri terminali)
3) autenticazione nel formato *nome_utente password*
4) riconoscimento dei comandi (es. ADD /usr/file.txt -> il server riconoscerà il comando, quindi la prima parte della stringa)
5) chiusura della conessione

### Credenziali

- **nome utente**: Lorenzo - **password**: passwd1
- **nome utente** Giandonato -  **password**: passwd2
- **nome utente**: Bruno - **password**: passwd3

### Problemi
- Impossibilità di specificare quante richieste avere in coda (parametro della listen nelle librerie std dei socket) --> probabilmente è importante (?)
