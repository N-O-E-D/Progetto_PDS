### Api server

Write here information about this part of the project

###Esempio di utilizzo
`    
 
         try {
         boost::asio::io_service ioService;
 
         ServerSocket server(ioService, <porta>, <path cartella>);
 
         ioService.run();
     } catch (std::exception& e) {
         std::cerr << "Exception: " << e.what() << "\n";
     }
 
     return 0; }