#include "mydocker.h"

static int make_directories(char *container_path, char *container_command_path)
{
	for (int i = strlen(container_path) + 1; container_command_path[i] != '\0'; i++)
	{
		if (container_command_path[i] == '/')
		{
			container_command_path[i] = '\0';
			if (mkdir(container_command_path, 0777) < 0)
				return -1;
			container_command_path[i] = '/';
		}
	}
	return 0;
}

static int copy_file(char *from_path, char *to_path)
{
	int  from;
	int  to;
	char buffer[BUFFER_SIZE];
	int  ret;

	from = open(from_path, O_RDONLY);
	if (from < 0)
		return -1;
	to = open(to_path, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (to < 0)
		return -1;
	
	while ((ret = read(from, buffer, BUFFER_SIZE)) > 0)
	{
		if (ret < 0 || write(to, buffer, ret) < 0)
			return -1;
	}

	close(from);
	close(to);

	return 0;
}

char *create_container(char *command)
{
	char container_path[] = "/tmp/container_XXXXXX";
	char container_command_path[BUFFER_SIZE];
	
	mkdtemp(container_path);
	
	sprintf(container_command_path, "%s%s", container_path, command);
	if (make_directories(container_path, container_command_path) < 0)
	{
		printf("Make directories error\n");
		exit(1);
	}
	if (copy_file(command, container_command_path) < 0)
	{
		printf("Copy binary error\n");
		exit(1);
	}

	return strdup(container_path);
}

void write_output(int stdout_stream, int stderr_stream)
{
	char buffer[BUFFER_SIZE];
	int  ret;

	while ((ret = read(stdout_stream, buffer, BUFFER_SIZE)) > 0)
	{
		write(STDOUT_FILENO, buffer, ret);
	}

	while ((ret = read(stderr_stream, buffer, BUFFER_SIZE)) > 0)
	{
		write(STDERR_FILENO, buffer, ret);
	}
}
