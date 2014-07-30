#include <stdio.h>
#include "csapp.h"
#include "cache.h"

void process_request(int connfd);
void send_request(int fd, char *uri, char *host, char *path, int port);
void sigpipe_handler(int signal);
void *execute_job(void *arg);

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";

int main(int argc, char **argv)
{
    int listenfd, port;
    int *connfd = malloc(sizeof(*connfd));
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    pthread_t tid;

    //If port is not specified, give proper error message
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    Signal(SIGPIPE, sigpipe_handler);
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port); 
    while(1) {
	clientlen = sizeof(clientaddr);
	*connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	haddrp = inet_ntoa(clientaddr.sin_addr);
	printf("server connected to %s (%s)\n", hp->h_name, haddrp);
	//Make the server concurrent using threads.
	Pthread_create(&tid, NULL, execute_job, connfd);
    }

    return 0;
}

/*Each thread executes this function*/
void *execute_job(void *arg) {
    Pthread_detach(pthread_self());
    int connfd = *((int *)arg);
    process_request(connfd);
    Close(connfd);
    Pthread_exit(NULL);
    return 0;
}

/*This function parses the URI, and calls send_request() to forward the request to server*/
void process_request(int connfd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], host[MAXLINE], path[MAXLINE];
    rio_t rio;
    size_t size;
    int port = 80;

    Rio_readinitb(&rio, connfd);
    size = Rio_readlineb(&rio, buf, MAXLINE);

    if(size < 0) {
	fprintf(stderr, "rio_readlineb error: %s\n",strerror(errno));
	return;
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")) {
	fprintf(stderr, "Method not implemented\n");
	Close(connfd);
    }

    sscanf(uri, "http://%s", host);

    char *h = strchr(host, '/');
    if (h != NULL) {
	strcpy(path, h);
	*h = '\0';
    }

    char *p = strchr(host, ':');
    if (p != NULL) { 
	port = atoi(p + 1);
	*p = '\0';
    }

    send_request(connfd, uri, host, path, port);

}


/* This function first checks the cache if the requested object is cached.
 * If the object is cached, then gets the content from the cache.
 * If not, then the request is forwarded to the server to get the content.
 * Caching is done for response returned from the server.
 * */
void send_request(int fd, char *uri, char *host, char *path, int port) {
    char buf[MAXBUF], response[MAXBUF];
    struct cache *cached_node;
    int connfd, cache_item_size = 0;
    char cache_content[MAX_OBJECT_SIZE];
    rio_t rio;
    
    cached_node = get_cache_item(uri);
    if(cached_node != NULL) {
	printf("Cached object for for URI %s\n", uri);
	rio_writen(fd, cached_node->content, cached_node->length);
	return;
    }
    
    printf("\n Opening connection to remote server");
    int server_fd = Open_clientfd(host, port);
    
    if(server_fd < -1) {
	printf("Invalid file descriptor\n");
	return;
    }

    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    sprintf(buf, "%sHost: %s\r\n", buf, host);
    strcat(buf, user_agent_hdr);
    strcat(buf, accept_hdr);
    strcat(buf, accept_encoding_hdr);
    strcat(buf, "Connection: close\r\n");
    strcat(buf, "Proxy-Connection: close\r\n");
    strcat(buf,"\r\n");
    printf("HTTP request buf: \n%s\n", buf);
    
    Rio_writen(server_fd, buf, strlen(buf));

    Rio_readinitb(&rio, server_fd);

    strcpy(cache_content,"");

    do {
	strcpy(response,"");
	connfd = Rio_readnb(&rio, response, MAXBUF);
	if((cache_item_size += connfd) < MAX_OBJECT_SIZE)
	    strcat(cache_content, response);
	Rio_writen(fd, response, connfd);
    }while(connfd > 0);

    if(cache_item_size < MAX_OBJECT_SIZE)
	put_cache_item(uri, cache_content, cache_item_size);

    return;

}

void sigpipe_handler(int signal) {
    printf("Signal SIGPIPE recieved!\n");
    return;
}
