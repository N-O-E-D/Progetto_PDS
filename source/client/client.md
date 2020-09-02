# Client

### How client works

#### ARCHITECTURE
* It receives three arguments: the path to back up, the IP and port of the backup server;
* Client has two threads, one acts as producer and the other as consumer;
* The communication between producer and consumer is managed by a thread-safe queue provided by the Boost library;
* In addition, there is a thread-safe map that is used for another purpose.

#### PRODUCER
* The producer consists in a file system watcher that notices all the changes in the path provided by the user;
* If a notable change is noted, the producer puts it into the queue;
* If the queue is full, it removes the oldest entry from the queue, puts the new one and set the map as to sync;
* The producer can be stopped using an appropriate method. 

#### CONSUMER
* First of all, it syncs the map if it is set as to be synced;
* Then if there is an entry in the queue, the consumer processes it sending to the server the message corresponding to the change in the file system;
* If there are any exceptions, they are caught: some has dedicated handlers, others the general one.

