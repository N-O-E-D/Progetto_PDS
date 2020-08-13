* dichiarare un array di lunghezza EVP_MAX_MD_SIZE di tipo unsigned char
``` 
unsigned char md_value[EVP_MAX_MD_SIZE]; //md_value conterrà il valore dell'hash
``` 
* richiamare la funzione computeHash(std::string const& path,unsigned char* md_value) per il calcolo dell'hash. 
Questa funzione ritornerà la lunghezza dell'hash.
``` 
unsigned int md_len=computeHash("/home/lorenzo/Scrivania/prova.txt",md_value);
``` 
* per la lettura:
``` 
printf("The digest is: ");
    for(int i = 0; i < md_len; i++)
        printf("%02x", md_value[i]);
    printf("\n");
``` 
* per il confronto da due hash :
``` 
compareHash(md_value1,md_value2,md_len); //ritorna un bool
``` 
md_value sono due array di tipo  unsigned char . Per convertire da string ad unsigned char* fare
reintrepret_cast<unsigned char*>(nomestringa.data());