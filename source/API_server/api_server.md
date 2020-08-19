### Api server

Write here information about this part of the project

### Esempio di utilizzo
```
 
         try {
         boost::asio::io_service ioService;
         Server s;
         ServerSocket server(ioService, <porta>, s);
 
         ioService.run();
     } catch (std::exception& e) {
         std::cerr << "Exception: " << e.what() << "\n";
     }
 
     return 0; }
```

# todo
* Implementare messagi di risposta del server verso il client
* Modificare costruttore ServerSocket (done)
* Risolvere problemi sull'invio del file (done)
* Risolvere problemi anche sulla remove (header) (done)

