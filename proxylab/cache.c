/**
 * Cache for web proxy. 
 * Prajwal Yadapadithaya (pyadapad) and Abhinav KR (akuruvad)
 *
 * Cache is a doubly linked list with nodes as cache objects. LRU policy is followed by
 * placing the cache node accessed at the beginning of the list. During eviction, the 
 * tail node is removed.
 **/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "cache.h"
#include "csapp.h"

struct cache *head, *tail;
int free_cache_size;
pthread_rwlock_t lock;

/*Initialize cache to an empty cache*/
void initialize_cache() {
    head = NULL;
    tail = NULL;
    free_cache_size = MAX_CACHE_SIZE;
    pthread_rwlock_init(&lock, NULL);
}

/*Insert a cache node at the beginning of the list.*/
void insert(struct cache *node) {
    pthread_rwlock_wrlock(&lock);
    if(head == NULL) {
	node->next = NULL;
	node->prev = NULL;
	head = node;
	tail = node;
    }
    else {
	node->next = head;
	node->prev = NULL;
	head->prev = node;
	head = node;
    }
    free_cache_size -= node->length;
    pthread_rwlock_unlock(&lock);
}

/*Delete a cache node*/
void delete(struct cache *node) {
    if((node == head) && (node == tail)) {
	initialize_cache();
	free(node); 
    }
    else {
	pthread_rwlock_wrlock(&lock);
	if(node != head)
	    node->prev->next = node->next;
	else
	    head = node->next;

	if(node != tail)
	    node->next->prev = node->prev;
	else
	    tail = node->prev;

	free_cache_size += node->length;
	free(node);
	pthread_rwlock_unlock(&lock);
    }
}

/*Get the cache node from a given URI*/
struct cache *get_cache_item(char *uri) {
    pthread_rwlock_rdlock(&lock);
    struct cache *temp = head;
    while(temp != NULL) {
	if(!strcmp(temp->uri, uri)) {
	    pthread_rwlock_unlock(&lock);
	    delete(temp);
	    insert(temp);
	    return temp;
	}
	temp = temp->next;
    }
    pthread_rwlock_unlock(&lock);
    return NULL;
}

/*Create a cache node for given inputs, and place the node at the beginning of the list*/
void put_cache_item(char *uri, char *content, int length) {
    struct cache *cache_node = (struct cache *)malloc(sizeof(struct cache));

    cache_node->uri = malloc(1024);
    strcpy(cache_node->uri, uri);

    cache_node->content = malloc(length);
    memcpy(cache_node->content, content, length);

    cache_node->length = length;

    while(length > free_cache_size) {
	delete(tail);
    }

    insert(cache_node);
}
