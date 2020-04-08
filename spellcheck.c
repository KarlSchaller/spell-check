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
#define NUM_WORKERS 4

//127.0.0.1

struct Entry {
	char *word;
	bool correctness;
}

struct Queue { 
    int front, rear, size; 
    unsigned capacity; 
    void *array; 
} logqueue, workqueue;
  
struct Queue *createQueue(unsigned capacity);
void enqueue(struct Queue *queue, void item);
void dequeue(struct Queue *queue);
int search(int n, char dict[][n], char *key);
void *workerfunction(void *args);
void *loggerfunction(void *args);

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
	// Socket
	int serverfd = socket(AF_INET, SOCK_STREAM, 0); // create socket
	if (serverfd < 0) {
        perror("Socket Error"); 
		exit(EXIT_FAILURE);
	}
    else
        puts("Socket");
	
	// Address
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	if (argc >= 3)
		address.sin_port = htons(atoi(argv[2]));
	else
		address.sin_port = htons(DEFAULT_PORT);
	
	// Bind
	if (bind(serverfd, (struct sockaddr *)&address, sizeof(address)) < 0) { // bind
        perror("Bind Error");
		exit(EXIT_FAILURE);
	}
	else
		puts("Bind");
	
	// List
	if (listen(serverfd, MAX_CLIENTS) < 0) { // listen
        perror("Listen Error"); 
        exit(EXIT_FAILURE); 
    }
	else
		puts("Listen");
	// ====================================================================
	
	
	// THREADS ============================================================
	// Create worker threads
	p_thread workerthreads[NUM_WORKERS];
	for (int i = 0; i < NUM_WORKERS; i++)
		pthread_create(workerthreads[i], NULL, workerfunction, NULL);
	
	// Create logger thread
	p_thread loggerthread;
	pthread_create(&loggerthread, NULL, loggerfunction, NULL);
	
	// Main thread
	int addrlen = sizeof(struct sockaddr_in);
	while (1) {
		int newsocket;
		if ((newsocket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) { 
			perror("Accept Error");
			exit(EXIT_FAILURE); 
		}
		else
			puts("Accept");
		//To create a thread: pthread_create(&name, NULL, methodName, NULL);
		enqueue(&workqueue, newsocket);
		//signal any sleeping workers that there's a new socket in the queue;
		printf("After accept call on newsocket # %d\n",newSocket);
	}
	/*
    valread = read( new_socket , buffer, 1024); 
    printf("%s\n",buffer ); 
    send(new_socket , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n");
	*/
	/*
	pthread_mutex_t name;
	pthread_mutex_lock(&name);
	pthread_mutex_unlock(&name);
	pthread_cond_t name;
	pthread_cond_init(&name, NULL);
	pthread_cond_wait(&name, &mutexName);
	pthread_cond_signal(&name);
	*/
	// ====================================================================


	// Join threads   	
	for (i=0; i < NUM_WORKERS; i++)
   		pthread_join(workerthreads[i], NULL);
	pthread_join(loggerthread, NULL);
	
	exit(0);
}

struct Queue *createQueue(unsigned capacity) {
    struct Queue *queue = (struct Queue *) malloc(sizeof(struct Queue)); 
    queue->capacity = capacity; 
    queue->front = queue->size = 0;  
    queue->rear = capacity - 1;
    queue->array = (int *) malloc(queue->capacity * sizeof(int)); 
    return queue;
} 
  
void enqueue(struct Queue *queue, void item) { 
    if (queue->size == queue->capacity) 
        return; 
    queue->rear = (queue->rear + 1)%queue->capacity; 
    queue->array[queue->rear] = item; 
    queue->size = queue->size + 1; 
    printf("%d enqueued to queue\n", item); 
}

void dequeue(struct Queue *queue) {
    if (queue->size == 0) 
        return INT_MIN;
    int item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%queue->capacity; 
    queue->size = queue->size - 1; 
    return item; 
}

// Search dict for key
bool search(int n, char dict[][n], char *key) {
	for (int i = 0; dict[i][0] != '\0'; i++) {
		if (strcmp(dict[i], key) == 0)
			return 1;
	}
	return 0;
}

//A server worker thread's main loop is as follows:
void* workerfunction(void *args) {
	while (1) {
		while (workqueue.size > 0) {
			int newsocket = dequeue(&workqueue);
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
				struct Entry newentry = {}
				enqueue(&logqueue, Entry);
				write the word and the socket response value (“OK” or “MISSPELLED”) to the log queue;
			}
			close socket
		}
	}
}

//A second server thread will monitor a log queue and process entries 
//by removing and writing them to a log file.
void* loggerfunction(void *args) {
	FILE *log = fopen("log.txt", "w");
	if (!log) {
		perror("Log File Error:");
		exit(EXIT_FAILURE);
	}
	while (1) {
		while (logqueue.size > 0) {
			struct Entry newentry = dequeue(&logqueue);
			fprintf(log, "%s %d", newentry.word, newentry.correctness);
		}
	}
	fclose(log);
}