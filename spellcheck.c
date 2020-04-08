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
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define MAX_CLIENTS 32
#define NUM_WORKERS 4

struct Entry {
	char *word;
	bool correctness;
};

struct Queue { 
    int front, rear, size; 
    unsigned capacity; 
    void **array;
	size_t memsize;
} *workqueue, *logqueue;

pthread_mutex_t workmutex, logmutex;
pthread_cond_t worknotempty, lognotempty, worknotfull, lognotfull;
char **words;
int maxlen = 1, lines = 2;
  
struct Queue *createqueue(unsigned capacity, size_t memsize);
int isfull(struct Queue *queue);
int isempty(struct Queue *queue);
void enqueue(struct Queue *queue, void *item);
void dequeue(struct Queue *queue, void *dest);
bool search(char **dict, char *key);
void alphnum(char *str);
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
	while (!feof(dict)) {
		int len = 1;
		while (!feof(dict) && getc(dict) != '\n')
			len++;
		if (len > maxlen)
			maxlen = len;
		lines++;
	}
	words = (char **)malloc(lines * sizeof(char *));
	for (int i = 0; i < lines; i++)
		words[i] = (char *)malloc(maxlen * sizeof(char));
	
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
	printf("%d\n", search(words, "aal"));
	printf("%d\n", search(words, "asdfhsfgasc"));
	// ====================================================================


	// SOCKET =============================================================
	// Socket
	int serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd < 0) {
        perror("Socket Error"); 
		exit(EXIT_FAILURE);
	}
    else
        puts("Socket");
	
	// Address
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // 127.0.0.1
	if (argc >= 3)
		address.sin_port = htons(atoi(argv[2]));
	else
		address.sin_port = htons(DEFAULT_PORT);
	
	// Bind
	if (bind(serverfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind Error");
		exit(EXIT_FAILURE);
	}
	else
		puts("Bind");
	
	// Listen
	if (listen(serverfd, MAX_CLIENTS) < 0) {
        perror("Listen Error"); 
        exit(EXIT_FAILURE); 
    }
	else
		puts("Listen");
	// ====================================================================
	
	
	// THREADS ============================================================
	workqueue = createqueue(MAX_CLIENTS, sizeof(int));
	logqueue = createqueue(MAX_CLIENTS, sizeof(struct Entry));
	pthread_mutex_init(&workmutex, NULL);
	pthread_mutex_init(&logmutex, NULL);
	pthread_cond_init(&worknotempty, NULL);
	pthread_cond_init(&lognotempty, NULL);
	pthread_cond_init(&worknotfull, NULL);
	pthread_cond_init(&lognotfull, NULL);
	
	/*int i = 1;
	enqueue(workqueue, &i);
	enqueue(workqueue, &i);
	dequeue(workqueue, &i);
	dequeue(workqueue, &i);
	puts("done");*/
	
	// Create worker threads
	pthread_t workerthreads[NUM_WORKERS];
	for (int i = 0; i < NUM_WORKERS; i++)
		pthread_create(&(workerthreads[i]), NULL, workerfunction, NULL);
	puts("Workers");
	
	// Create logger thread
	pthread_t loggerthread;
	pthread_create(&loggerthread, NULL, loggerfunction, NULL);
	puts("Logger");
	
	// Main thread
	int addrlen = sizeof(struct sockaddr_in);
	while (1) {
		int newsocket;
		if ((newsocket = accept(serverfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) { 
			perror("Accept Error");
			exit(EXIT_FAILURE); 
		}
		else
			puts("Main: Accept");
		puts("Main: Before connection enqueue");
		pthread_mutex_lock(&workmutex);
		while (isfull(workqueue))
			pthread_cond_wait(&worknotfull, &workmutex);
		enqueue(workqueue, &newsocket);
		puts("Main: After connection enqueue");
		pthread_mutex_unlock(&workmutex);
		pthread_cond_signal(&worknotempty);
	}
	// ====================================================================


	// Join threads   	
	for (int i = 0; i < NUM_WORKERS; i++)
   		pthread_join(workerthreads[i], NULL);
	pthread_join(loggerthread, NULL);
	
	close(serverfd);
	free(words);
	free(workqueue);
	free(logqueue);
	exit(0);
}

struct Queue *createqueue(unsigned capacity, size_t memsize) {
    struct Queue *queue = (struct Queue *) malloc(sizeof(struct Queue)); 
    queue->capacity = capacity; 
	queue->memsize = memsize;
    queue->front = queue->size = 0;  
    queue->rear = capacity - 1;
    queue->array = malloc(queue->capacity * sizeof(void *));
	for (int i = 0; i < capacity; i++)
		queue->array[i] = malloc(memsize);
    return queue;
}

int isfull(struct Queue *queue) { return (queue->size == queue->capacity); } 
  
int isempty(struct Queue *queue) { return (queue->size == 0); } 

void enqueue(struct Queue *queue, void *item) { 
    if (isfull(queue)) 
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    memcpy(queue->array[queue->rear], item, queue->memsize);
    queue->size = queue->size + 1; 
    //puts("Enqueued"); 
}

void dequeue(struct Queue *queue, void *dest) {
    if (isempty(queue)) 
        return;
    void *item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%queue->capacity; 
    queue->size = queue->size - 1; 
    memcpy(dest, item, queue->memsize);
	//puts("Dequeued");
}

// Search dict for key
bool search(char **dict, char *key) {
	for (int i = 0; dict[i][0] != '\0'; i++) {
		if (strcmp(dict[i], key) == 0)
			return 1;
	}
	return 0;
}

// Remove all non-alphanumeric characters from str
void alphnum(char *str) {
	int i = 0, j = 0;
	char c;
	while ((c = str[i++]) != '\0') {
		if (isalnum(c))
			str[j++] = c;
	}
	str[j] = '\0';
}

//A server worker thread's main loop is as follows:
void *workerfunction(void *args) {
	while (1) {
		int newsocket;
		puts("Work: Before connection dequeue");
		pthread_mutex_lock(&workmutex);
		while (isempty(workqueue))
			pthread_cond_wait(&worknotempty, &workmutex);
		dequeue(workqueue, &newsocket);
		puts("Work: After connection dequeue");
		pthread_mutex_unlock(&workmutex);
		pthread_cond_signal(&worknotfull);
		int numbytes = 1;
		while (numbytes > 0) {
			struct Entry newentry;
			char *buf = (char *)calloc(1024, sizeof(char));
			puts("Work: Before read");
			numbytes = read(newsocket, buf, 1024); //read word from the socket
			puts("Work: After read");
			alphnum(buf);
			//buf[strlen(buf)-2] = '\0';
			newentry.word = (char *)malloc((strlen(buf)+1)*sizeof(char));
			strcpy(newentry.word, buf);
			puts("Work: After strcpy");
			printf("Work: word \"%s\"\n", buf);
			printf("Work: correctness \"%d\"\n", search(words, buf)); //debug
			if (newentry.correctness = search(words, buf))
				write(newsocket, strcat(buf, "OK\n"), strlen(buf)+4);
			else
				write(newsocket, strcat(buf, "MISSPELLED\n"), strlen(buf)+12);
			puts("Work: Before log enqueue");
			pthread_mutex_lock(&logmutex);
			while (isfull(logqueue))
				pthread_cond_wait(&lognotfull, &logmutex);
			enqueue(logqueue, &newentry);
			puts("Work: After log enqueue");
			pthread_mutex_unlock(&logmutex);
			pthread_cond_signal(&lognotempty);
			free(buf);
		}
		close(newsocket);
	}
}

//A second server thread will monitor a log queue and process entries 
//by removing and writing them to a log file.
void *loggerfunction(void *args) {
	FILE *log = fopen("log.txt", "w");
	if (!log) {
		perror("Log: Log File Error:");
		exit(EXIT_FAILURE);
	}
	else
		puts("Log: Log opened");
	while (1) {
		struct Entry newentry;
		puts("Log: Before log dequeue");
		pthread_mutex_lock(&logmutex);
		while (isempty(logqueue))
			pthread_cond_wait(&lognotempty, &logmutex);
		dequeue(logqueue, &newentry);
		puts("Log: After log dequeue");
		pthread_mutex_unlock(&logmutex);
		pthread_cond_signal(&lognotfull);
		printf("Log: word \"%s\"\n", newentry.word);
		printf("Log: correctness \"%d\"\n", newentry.correctness);
		if (newentry.correctness)
			fprintf(log, "%s OK\n", newentry.word);
		else
			fprintf(log, "%s MISSPELLED\n", newentry.word);
		puts("Log: Wrote log");
		free(newentry.word);
	}
	fclose(log);
}