#ifndef SERVER_H
#define SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "workQueue.h"
#include "logQueue.h"
#include "spellChecker.h"
#define BUF_LEN 512
#define MAX_CLIENTS 50 //max clients allowed to be serviced before server terminates
#define MAX_THREAD_POOL 50 //max number of worker threads created
#define MAX_WORKQUEUE 25 //max number of clients allowed in work queue at a time
#define MAX_LOGQUEUE 100 //max number of log results allowed in log queue at a time
#define DEFAULT_PORT 8888
#define DEFAULT_DICTIONARY "DEFAULT_DICTIONARY.txt"



char **dictionary; //pointer to dictionary data structure
char *dict_file; //pointer to dictionary FILE
int num_clients; //number of clients that have connected to the server
int clients_serviced; //number of clients that have been serviced and left the server
int num_threads; //number of worker threads created
pthread_t log_thread;
pthread_t thread_pool[MAX_THREAD_POOL];

//client work queue data
WORKQUEUE_t *work_queue;
pthread_mutex_t m_workQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_spot_in_workQueue = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_client_in_workQueue = PTHREAD_COND_INITIALIZER;

//worker log queue data
LOGQUEUE_t *log_queue;
pthread_mutex_t m_logQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_spot_in_logQueue = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_result_in_logQueue = PTHREAD_COND_INITIALIZER;
FILE *log_file;

//sockaddr_in holds information about the user connection which needs to be passed into accept().
struct sockaddr_in client_addr;
int client_len = sizeof(client_addr);
int connection_port;
int connection_socket, bytes_returned;

//static messages
char* msg_greeting = "\n//////////////////////////////////////////////////////\n"
					   "*      *** Welcome to Sean's Spell Checker! ***      *\n"
					   "*                                                    *\n"
					   "* Send some text to check the spelling of each word. *\n"
					   "*    The Server will then send back the results.     *\n"
					   "*                                                    *\n"
					   "*    Send the escape key to close the connection.    *\n"
					   "*                                                    *\n"
					   "//////////////////////////////////////////////////////\n";

char* msg_response = "You sent: "; //used for testing purposes
char* msg_prompt = ">>>";
char* msg_error = "Error: Server did not recieve message.\n";
char* msg_close = "Goodbye!\n";

/* Function to run the server of the spell checker. The server continues to accept client
	connections and insert them into the work_queue to be serviced by worker threads. */
void runServer(){

	//continue running server until MAXCLIENTS is reached
    while(clients_serviced < MAX_CLIENTS){

        pthread_mutex_lock(&m_workQueue); //lock work_queue
		//while loop to wait() the server thread while the work_queue full
        while(work_queue->size == MAX_WORKQUEUE)
            pthread_cond_wait(&c_spot_in_workQueue, &m_workQueue);
        

		//accept incoming client connections
		int client_socket;
		
		while(client_socket = accept(connection_socket, (struct sockaddr*)&client_addr, &client_len)){
			puts("Connection accepted!");
			write(client_socket, msg_greeting, strlen(msg_greeting));
			num_clients++;
			enQueueClient(num_clients, client_socket, work_queue); //add new client to work_queue
			pthread_cond_signal(&c_client_in_workQueue); //signal worker thread to service client
			pthread_mutex_unlock(&m_workQueue); //unlock work_queue
		}

		//client acception failed
		if(client_socket < 0){
			perror("Error connecting to client.\n");
			return;
		}
    }
}

/* Function to run the log thread which removes LOG nodes from the log_queue
	and prints the LOG data to the "log.txt" file. */
void *runLogThread(){

    while(1){

        pthread_mutex_lock(&m_logQueue); //lock log_queue
		//while loop to wait() the log thread while the log_queue is empty
        while(LogQueueIsEmpty(log_queue))
            pthread_cond_wait(&c_result_in_logQueue, &m_logQueue);
        
		//obtain log from the log_queue
		LOG_t *new_log = deQueueLog(log_queue); 

		pthread_cond_signal(&c_spot_in_logQueue); //signal worker threads of new spot in the log_queue
		pthread_mutex_unlock(&m_logQueue);


		//Opens log file
		if((log_file = fopen("log.txt", "a+")) == NULL){
			puts("Error: Log file could not be opened.");
			pthread_exit(0);
		}

		//Outputs log data to the log file
		fprintf(log_file, "Thread ID: %d\n", new_log->thread_id);
		fprintf(log_file, "Client ID: %d\n", new_log->client_id);
		fprintf(log_file, "%s\n", new_log->results);

		fclose(log_file);
		free(new_log);
    }
}

/* Function called by worker threads to exchange data through the socket of the thread's client.
	For every message sent by the client, the assigned worker thread will check the spelling of the message,
	reply to the client with the results of the spell check, and add the results to the log queue. */
void serviceClient(CLIENT_t *c){

	int clientSocket = c->client_socket;
	int bytesReturned;
	char recvBuffer[BUF_LEN];

	//Begin sending and receiving messages.
	while(1){
		send(clientSocket, msg_prompt, strlen(msg_prompt), 0);

		//recv() will store the message from the user in the buffer, returning
		//how many bytes the server received.
		bytesReturned = recv(clientSocket, recvBuffer, BUF_LEN, 0);

		//Check if the server got a message, send a message back or quit if the
		//client specified it.
		if(bytesReturned == -1){
			send(clientSocket, msg_error, strlen(msg_error), 0);
		}
		else if(recvBuffer[0] == 27){ //'27' is the escape key
			send(clientSocket, msg_close, strlen(msg_close), 0);
			close(clientSocket);
			break;
		}
		else{ //process and spell check message

			//testing purposes
			// send(clientSocket, msg_response, strlen(msg_response), 0);
			// send(clientSocket, recvBuffer, bytesReturned, 0);

			char *word = malloc(sizeof(char)*BUF_LEN);
			strncpy(word, recvBuffer, strcspn(recvBuffer, "\r\n"));
			char *results = checkWord(word, dictionary); //spell checks word sent by client
			send(clientSocket, results, strlen(results), 0);

			//Add the results to the log queue
			pthread_mutex_lock(&m_logQueue); //lock log_queue
			//while loop to wait() the worker thread until there is an open spot in the log_queue
			while(log_queue->size == MAX_LOGQUEUE)
				pthread_cond_wait(&c_spot_in_logQueue, &m_logQueue);


			//adds results to log_queue
			enQueueLog(c->client_num, syscall(SYS_gettid), results, log_queue);
			pthread_cond_signal(&c_result_in_logQueue); //signal the log thread of a new log to output
			pthread_mutex_unlock(&m_logQueue); //unlock the log_queue

		}
	}//end while
}

/* Function to spawn and run worker threads to service the clients connected to the server.
	When the work_queue is not empty, a worker thread will deQueue the next client waiting
	in the queue and begin to service the client until they choose to quit the server. */
void *runWorkerThread(){

	pthread_mutex_lock(&m_workQueue);//lock work_queue
	//while loop to wait() the worker thread while the work_queue is empty
	while(workQueueIsEmpty(work_queue))
		pthread_cond_wait(&c_client_in_workQueue, &m_workQueue);

	//obtain client from the work_queue
	CLIENT_t *client = deQueueClient(work_queue);
	
	pthread_cond_signal(&c_spot_in_workQueue); //signal server of a new spot in the work_queue
	pthread_mutex_unlock(&m_workQueue);
	puts("Client assigned");
	serviceClient(client); //begins spell checking words the client sends to the server
	free(client);
	clients_serviced++;
	pthread_exit(0);


}

/* Copied from the Computer Systems textbook.
	This function creates a socket descriptor, and binds the socket descriptor the specified port.
	bind() associates the socket descriptor created with socket() to the port we want the server to listen on.
	Once the descriptor is bound, the listen() call will prepare the socket so that we can call accept() on
	it and get a connection to a user. */
int open_listenfd(int port)
{
	int listenfd, optval=1;
	struct sockaddr_in serveraddr;

	/* Create a socket descriptor */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}

	 /* Eliminates "Address already in use" error from bind */
	 if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
	 (const void *)&optval , sizeof(int)) < 0){
	 	return -1;
	 }

	 //Reset the serveraddr struct, setting all of it's bytes to zero.
	 //Some properties are then set for the struct, you don't
	 //need to worry about these. 
	 //bind() is then called, associating the port number with the
	 //socket descriptor.
	 bzero((char *) &serveraddr, sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	 serveraddr.sin_port = htons((unsigned short)port);
	 if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
	 	return -1;
	 }

	 //Prepare the socket to allow accept() calls. The value 20 is 
	 //the backlog, this is the maximum number of connections that will be placed
	 //on queue until accept() is called again. 
	 if (listen(listenfd, 20) < 0){
	 	return -1;
	 }

	 return listenfd;
}

/*Determines whether a command line argument is a port number or dictionary file.
	Returns 1 if arg contains only digits (port) or 0 otherwise (dictionary file). */
int argType(char *arg){
	while(*arg){
		if(isdigit(*arg++) == 0)
			return 0;
	}
	return 1;
}
#endif