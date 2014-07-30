#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "cache.h"

struct cache *head, *tail;
int free_cache_size;

void initialize_cache() {
    head = NULL;
    tail = NULL;
    free_cache_size = MAX_CACHE_SIZE;
}

void insert(struct cache *node) {
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
}

void delete(struct cache *node) {
    if((node == head) && (node == tail)) {
	initialize_cache();
	free(node); 
    }
    else {
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
    }
}

struct cache *get_cache_item(char *uri) {
    struct cache *temp = head;
    while(temp != NULL) {
	if(!strcmp(temp->uri, uri)) {
	    delete(temp);
	    insert(temp);
	    return temp;
	}
	temp = temp->next;
    }
    return NULL;
}

void put_cache_item(char *uri, char *content, int length) {
    struct cache *cache_node = (struct cache *)malloc(sizeof(struct cache));

    cache_node->uri = malloc(256);
    strcpy(cache_node->uri, uri);

    cache_node->content = malloc(length);
    memcpy(cache_node->content, content, length);

    cache_node->length = length;

    while(length > free_cache_size) {
	delete(tail);
    }

    insert(cache_node);
}
