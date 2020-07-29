### Api client

Write here information about this part of the project
####Utilizzo
* Creare un oggetto di tipo ClientSocket 
```    
          boost::asio::io_service ioService;
          boost::asio::ip::tcp::resolver resolver(ioService);
          auto endpointIterator = resolver.resolve({ address, port });
          ClientSocket client(ioService, endpointIterator, filePath);
``` 
* Inviare messaggi al server richiamando i metodi forniti dalla classe(update,updateName,remove,removeDir,insert)
```    
          client.remove(filePath);
          ioService.run();
``` 
###Esempio di utilizzo
`
 
 
     auto address = argv[1];
     auto port = argv[2];
     auto filePath = argv[3];
 
     try {
         boost::asio::io_service ioService;
 
         boost::asio::ip::tcp::resolver resolver(ioService);
         auto endpointIterator = resolver.resolve({ address, port });
         ClientSocket client(ioService, endpointIterator, filePath);
         client.remove(filePath);
         ioService.run();
     } catch (std::fstream::failure& e) {
         std::cerr << e.what() << "\n";
     } catch (std::exception& e) {
         std::cerr << "Exception: " << e.what() << "\n";
     }
 
     return 0;
     }
`