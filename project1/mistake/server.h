#ifndef _SERVER_H
#define _SERVER_H

void *dynamic_pool_size_update(void *arg);

int getCacheIndex(char *request);

void addIntoCache(char *mybuf, char *memory, int memory_size);

void deleteCache();

void initCache();

char* getContentType(char *mybuf);

int readFromDisk();

void *dispatch(void *arg);

void *worker(void *arg);

#endif /* _SERVER_H */
