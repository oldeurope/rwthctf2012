#ifndef STORE_H
#define STORE_H

void store_init();
int store_put(const char* store, const char* key, const char* value);
int store_get(const char* store, const char* key, char* value, int len);
char* store_list(const char* store);

#endif
