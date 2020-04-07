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

#define DEFAULT_DICTIONARY dictionary.txt
#define DEFAULT_PORT 8888
#define NUM_WORKERS 32
//127.0.0.1

int search(FILE *dict, char *key);

int main(int argc, char **argv, char** envp) {
	
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
	char words[maxlen][lines];
	
	// Fill array of words
	rewind(dict);
	int line = 0;
	while (!feof(dict)) {
		int i = 0;
		for (char c; !feof(dict) && (c = getc(dict)) != '\n'; i++)
			words[i][line] = c;
		words[i][line] = '\0';
		line++;
	}
	words[][line] = NULL;

	// Open port
	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0) 
        printf("Error in server creating\n"); 
    else
        printf("Server Created\n");
	if (argc >= 3)
		puts("opening port"); // open argv[2]
	else
		puts("opening port"); // open DEFAULT_PORT
	
	//The main thread creates a pool of NUM_WORKERS worker threads, and 
	//then immediately begins to behave in the following manner(to accept 
	//and distribute connection requests)
	p_thread workers[NUM_WORKERS];
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

	//A second server thread will monitor a log queue and process entries 
	//by removing and writing them to a log file.
	while (true) {
		while (the log queue is NOT empty) {
			
			remove an entry from the log
			write the entry to the log file
		}
	}
	
	//A server worker thread's main loop is as follows:
	while (true) {
		while (the work queue is NOT empty) {
			remove a socket from the queue
			notify that there's an empty spot in the queue
			service client
			close socket
		}
	}
	
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
	
	exit(0);
}

// Search dict for key
int search(FILE *dict, char *key) {
	int bufsize = 32;
	char *buffer = (char *)malloc(bufsize);
	while (getline(buffer, bufsize, dict) != -1){
		if (strstr(buffer, key))
			return 1;
	}
	return 0;
}