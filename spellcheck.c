/*
============================================================================
Name        : Karl Schaller
Date        : 03/25/2020
Course      : CIS3207
Homework    : Assignment 3 Spell Check
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8080
#define MAX_CLIENTS 32
#define NUM_WORKERS 32

//127.0.0.1

int search(int n, char dict[][n], char *key);
void* workerfunction(void *args);
void* loggerfunction(void *args);

int main(int argc, char **argv, char** envp) {
	
	// DICTIONARY =========================================================
	// Open dictionary file
	FILE *dict;
	if (argc >= 2)
		dict = fopen(argv[1], "r");
	else
		dict = fopen(DEFAULT_DICTIONARY, "r");
	if (dict == NULL)
		perror("Could not open dictionary");
	
	// Initialize array of words from dictionary file
	int maxlen = 1, lines = 2;
	while (!feof(dict)) {
		int len = 1;
		while (!feof(dict) && getc(dict) != '\n')
			len++;
		if (len > maxlen)
			maxlen = len;
		lines++;
	}
	char words[lines][maxlen];
	
	// Fill array of words
	rewind(dict);
	int line = 0;
	while (!feof(dict)) {
		int i = 0;
		for (char c; !feof(dict) && (c = getc(dict)) != '\n'; i++)
			words[line][i] = c;
		words[line][i] = '\0';
		line++;
	}
	words[line][0] = '\0';
	
	// Print dictionary
	for (line = 0; words[line][0] != '\0'; line++)
		printf("%s\n", words[line]);
	printf("%d\n", search(maxlen, words, "aal"));
	printf("%d\n", search(maxlen, words, "asdfhsfgasc"));
	// ====================================================================


	// SOCKET =============================================================
	// Create socket and listen
	int serverfd = socket(AF_INET, SOCK_STREAM, 0); // create socket
	if (serverfd < 0) {
        perror("Socket Error"); 
		exit(EXIT_FAILURE);
	}
    else
        puts("Socket");
	struct sockaddr_in address; // create address
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	if (argc >= 3)
		address.sin_port = htons(atoi(argv[2])); // port argv[2]
	else
		address.sin_port = htons(DEFAULT_PORT); // port DEFAULT_PORT
	if (bind(serverfd, (struct sockaddr *)&address, sizeof(address)) < 0) { // bind
        perror("Bind Error");
		exit(EXIT_FAILURE);
	}
	else
		puts("Bind");
	if (listen(serverfd, MAX_CLIENTS) < 0) { // listen
        perror("Listen Error"); 
        exit(EXIT_FAILURE); 
    }
	else
		puts("Listen");
	// ====================================================================
	
/*	
	// THREADS ============================================================
	// Create logger thread
	p_thread loggerthread;
	pthread_create(&loggerthread, NULL, loggerfunction, NULL);
	
	// Create worker threads
	p_thread workerthreads[NUM_WORKERS];
	for (int i = 0; i < NUM_WORKERS; i++)
		pthread_create(workerthreads[i], NULL, workerfunction, NULL);
	
	// Main thread
	int addrlen = sizeof(struct sockaddr_in);
    if ((newsocket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) { 
        perror("Accept Error");
        exit(EXIT_FAILURE); 
    }
	else
		puts("Accept");
    valread = read( new_socket , buffer, 1024); 
    printf("%s\n",buffer ); 
    send(new_socket , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n");
	
	

	
	//The main thread creates a pool of NUM_WORKERS worker threads, and 
	//then immediately begins to behave in the following manner(to accept 
	//and distribute connection requests)
	pthread_mutex_t name;
	pthread_mutex_lock(&name);
	pthread_mutex_unlock(&name);
	pthread_cond_t name;
	pthread_cond_init(&name, NULL);
	pthread_cond_wait(&name, &mutexName);
	pthread_cond_signal(&name);
	while (true) {
		connected_socket = accept(listening_socket);
		//To create a thread: pthread_create(&name, NULL, methodName, NULL);
		add connected_socket to the work queue;
		signal any sleeping workers that there's a new socket in the queue;
	}
*/
	// ====================================================================
	
	exit(0);
}

// Search dict for key
int search(int n, char dict[][n], char *key) {
	for (int i = 0; dict[i][0] != '\0'; i++) {
		if (strcmp(dict[i], key) == 0)
			return 1;
	}
	return 0;
}
/*
//A server worker thread's main loop is as follows:
void* workerfunction(void *args) {
	while (true) {
		while (the work queue is NOT empty) {
			remove a socket from the queue
			notify that there's an empty spot in the queue
			service client
			//and the client servicing logic is:
			while (there's a word left to read) {
				read word from the socket
				if (the word is in the dictionary) {
					echo the word back on the socket concatenated with "OK";
				}
				else {
					echo the word back on the socket concatenated with "MISSPELLED";
				}
				write the word and the socket response value (“OK” or “MISSPELLED”) to the log queue;
			}
			close socket
		}
	}
}

//A second server thread will monitor a log queue and process entries 
//by removing and writing them to a log file.
void* loggerfunction(void *args) {	
	while (true) {
		while (the log queue is NOT empty) {
			
			remove an entry from the log
			write the entry to the log file
		}
	}
}*/