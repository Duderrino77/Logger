#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include<string.h>
#include "network.h"

//TYPES

#define MESSAGE_SIZE 256
#define FILE_NAME "fileA"

typedef struct node
{
	struct node*	next;
	char				log_message[MESSAGE_SIZE];
}node;

typedef struct log_queue
{
	node*	head;
	node*	tail;
}log_queue;


//data
static pthread_mutex_t log_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static int log_queue_inited = 0;
static log_queue queue;

static pthread_t logger_thread;
static pthread_cond_t log_queue_cond;
static pthread_mutex_t log_queue_mutex;

static pthread_cond_t log_node_free_cond;
static pthread_mutex_t log_node_free_mutex;

//METHODS

void file_out(char * msg){
    FILE * fp;
    if((fp=fopen(FILE_NAME,"a"))==NULL)
    {
        printf("The file can not be opened.\n");
        return;
    }
    fputs(msg, fp);
	fputs("\n", fp);
    fclose(fp);
}

static node* create_node(const char message[])
{
	node*  new_node = (node*)calloc(1, sizeof(node));
	if(!new_node)
	{
		perror("couldn't allocate memory for");
	}
	
	strncpy(new_node->log_message, message, MESSAGE_SIZE);
	return new_node;	
}

static void insert_queue_log(log_queue* queue, const char message[])
{
	pthread_mutex_lock(&log_queue_mutex);
	node* message_node = create_node(message);
	
	if(!queue->head)
	{
		queue->head = message_node;
		queue->tail = queue->head ;	
	}
	else
	{
		queue->tail->next = message_node;
		queue->tail = message_node;
	}
	
	  /* notify worker thread */
    pthread_cond_signal(&log_queue_cond);

    pthread_mutex_unlock(&log_queue_mutex);
}

static node* pop_message(log_queue* queue)
{
	if(!queue->head)
		return NULL;
	
	node* cur = queue->head;
	
	if(queue->head == queue->tail)
		queue->tail = queue->head = NULL;
	else
		queue->head = queue->head->next;
		
	return cur;
}

static void
log_queue_run(void)
{
    struct node *node;

	printf("log_queue_run()\n");
    for (;;) 
	{
        pthread_mutex_lock(&log_queue_mutex);
        while (queue.head == NULL) 
		{
            pthread_cond_wait(&log_queue_cond, &log_queue_mutex);
        }
		
		node = pop_message(&queue);
		pthread_mutex_unlock(&log_queue_mutex);
		if(node)
		{
			printf("%s\n", node->log_message);
			file_out(node->log_message);
			send_net_message(node->log_message);
			
			pthread_mutex_lock(&log_node_free_mutex);

			free(node);
		}

        pthread_cond_signal(&log_node_free_cond);
        pthread_mutex_unlock(&log_node_free_mutex);
	}
}


static int init_queue(log_queue* q)
{
	q->head =q->tail = NULL;

	pthread_cond_init(&log_queue_cond, NULL);
    pthread_mutex_init(&log_queue_mutex, NULL);
    pthread_cond_init(&log_node_free_cond, NULL);
    pthread_mutex_init(&log_node_free_mutex, NULL);
	
	if (pthread_create(&logger_thread, NULL, (void *(*)(void *))&log_queue_run, NULL) != 0)
        return -1;
	
	return 0;
}

static void logger_atexit()
{
	printf("Exit looger()\n");
	pthread_join(logger_thread, NULL);
}
//PUBLIC
int  logger_init()
{
	if(log_queue_inited) return 0;
	pthread_mutex_lock(&log_init_mutex);

    if (log_queue_inited == 0) 
	{
		if (init_queue(&queue) != 0) 
		{
			printf("exit()\n");
            pthread_mutex_unlock(&log_init_mutex);
            return -1;
        }
    }
	
	if(init_network("127.0.0.1" , 90909) < 0)
	{
		printf("exit()\n");
		pthread_mutex_unlock(&log_init_mutex);
		return -1;
	}
	
    log_queue_inited = 1;

    pthread_mutex_unlock(&log_init_mutex);
	atexit(logger_atexit);
	printf("logger_init() - finished\n");
}

void log_write(const char * log)
{
	insert_queue_log(&queue, log);
}

