#ifndef MYDOCKER_H
# define MYDOCKER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <curl/curl.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

typedef struct
{
	char   *ptr;
	size_t len;
}
string;

void write_output(int stdout_stream, int stderr_stream);
char *create_container(char *command);
char *get_registry_token(char *repo);
char *get_json_field(char *json, char *field);
char *get_manifest(char *token, char* image, char *tag);
char **parse_json_array(char *json);
void pull_layers(char *image, char **layers, char *token, char *container_path);
char *substr(char *str, int start, int len);

#endif // MYDOCKER_H
