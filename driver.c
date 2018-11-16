#include "server.h"

/* CIS3207: Lab 3 - Networked Spell Checker
	November 4, 2018
	by: Sean Reddington

	This program runs a spell checking server that clients can connect to
	using network sockets to check the spelling of words. A "worker" thread
	is created and assigned to each client that connects to the server.
	This thread will receive words from the client and check the spelling
	of the word using the dictionary data structure created. The worker thread
	will send the results of the spell check back to the user, as well as to
	another "log" thread, which logs the results of all spell checks done
	during the server's execution to the "log.txt" file.
	
	*See README for a more in-depth description.*

*/
int main(int argc, char **argv){

	/*Check if the command line arguments contain a specified port number or dictionary file
		if not, the unprovided arguments will be assigned to DEFAULT_DICTIONARY and DEFAULT_PORT.*/
	switch(argc){
		case 1: //no args -> both defaults
			dict_file = DEFAULT_DICTIONARY;
			connection_port = DEFAULT_PORT;
			break;
		case 2: //one arg
			if(argType(argv[1])){
				connection_port = atoi(argv[1]); //arg is port
				dict_file = DEFAULT_DICTIONARY; //use default dictionary
			}else{
				connection_port = DEFAULT_PORT; //use default port
				dict_file = argv[1]; //arg is dictionary
			}
			break;
		case 3: //two args -> no defaults
			if(argType(argv[1])){
				connection_port = atoi(argv[1]); //arg 1 is port
				dict_file = argv[2]; //arg 2 is dictionary
			}else{
				dict_file = argv[1]; //arg 1 is dictionary
				connection_port = atoi(argv[2]); //arg 2 is port
			}
			break;
		default:
			puts("Error: Invalid execution arguments.");
			return 1;

	}

	printf("Port: %d\nDictionary: %s\n", connection_port, dict_file);


    //Server can't use ports below 1024 and ports above 65535 which don't exist.
	if(connection_port < 1024 || connection_port > 65535){
		puts("Port number is either too low(below 1024), or too high(above 65535).");
		return -1;
	}

	//Creates server's connection socket
	connection_socket = open_listenfd(connection_port);
	if(connection_socket == -1){
		printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
		return -1;
	}

	//initialize counter variables
	clients_serviced = 0;
	num_clients = 0;
	num_threads = 0;

	
	dictionary = buildDictionary(dict_file);
	//printDictionary(dictionary); //testing puroses

	//Creates log file
	if((log_file = fopen("log.txt", "w+")) == NULL){
		puts("Log file could not be opened");
		return -1;
	}
	fclose(log_file);

	//Creates log thread
	if(pthread_create(&log_thread, NULL, runLogThread, (NULL)) < 0){
		perror("Error: Log thread creation failed!");
		return 1;
	}

	//initialize log_queue
	log_queue = createLogQueue();
	puts("Log queue initialized");

	//initialize work_queue
	work_queue = createWorkQueue();
	puts("Work queue initialized");

	//Creates thread pool
	while(num_threads < MAX_THREAD_POOL){
		//create worker thread
		if(pthread_create(&thread_pool[num_threads], NULL, runWorkerThread, NULL) < 0){
			perror("Error: Worker thread creation failed!");
			return 1;
		}
		num_threads++;
	}

	//Start server and begin receiving connections
	puts("Waiting for incoming connections...");
	runServer();

	//join thread to ensure the log file is finished before finishing execution
	if(pthread_join(log_thread, NULL) < 0){
		perror("Error: Log thread join failed!");
	}

	puts("exiting server!");
    return 0;
}