
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache {
    struct cache *next;
    struct cache *prev;
    char *uri;
    char *content;
    int length;
};

void initialize_cache();

struct cache *get_cache_item(char *uri);

void put_cache_item(char *uri, char *content, int length);

void update_cache_item(struct cache *item);
