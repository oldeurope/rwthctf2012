#include "store.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <string.h>

void store_init()
{
	struct stat st;
	if (stat("store",&st) != 0)
	{
		mkdir("store", S_IRWXU);
	}
}

int store_put(const char* store, const char* key, const char* value)
{
	char filename[128];
	snprintf(filename, 128, "store/%s_%s", store, key);
	FILE* f = fopen(filename, "w");
	if (!f) {
		return 0;
	}
	fputs(value, f);
	fclose(f);
	return 1;
}

int store_get(const char* store, const char* key, char* value, int len)
{
	char filename[128];
	snprintf(filename, 128, "store/%s_%s", store, key);
	FILE* f = fopen(filename, "r");
	if (f == NULL) return 0;
	if (fgets(value, len, f) == NULL) return 0;
	fclose(f);
	return 1;
}

char* store_list(const char* store)
{
	struct dirent* d;
	DIR* dir = opendir("store/");
	if (dir == NULL) return NULL;
	int count = 0;
	while ((d = readdir(dir)) != NULL)
	{
		char* occ = strstr(d->d_name, "_");
		if (occ == NULL) continue;
		occ[0] = '\0';
		if (strcmp(d->d_name, store) == 0) count += strlen(++occ) + 1;
	}
	char* buf = malloc(count);
	char* cur = buf;
	dir = opendir("store/");
	if (dir == NULL) return NULL;
	while ((d = readdir(dir)) != NULL)
	{
		char* occ = strstr(d->d_name, "_");
		if (occ == NULL) continue;
		occ[0] = '\0';
		strcpy(cur, ++occ);
		cur += strlen(occ) + 1;
		cur[-1] = ' ';
	}
	return buf;
}