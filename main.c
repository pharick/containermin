#include "mydocker.h"

int main(int argc, char *argv[])
{
	int  stdout_pipe[2];
	int	 stderr_pipe[2];
	char *command;
	int  status;
	char *token;
	char *manifest;
	char *fs_layers;
	char *colon;
	char *image;
	char *tag;
	char *container_path;

	setbuf(stdout, NULL);

	command = argv[3];
	colon = strchr(argv[2], ':');
	if (colon != NULL)
	{
		image = substr(argv[2], 0, colon - argv[2]);
		tag = substr(argv[2], colon - argv[2] + 1, strlen(argv[2]) - (colon - argv[2]));
	}
	else
	{
		image = strdup(argv[2]);
		tag = strdup("latest");
	}

	container_path = create_container(command);

	token = get_registry_token(image);
	manifest = get_manifest(token, image, tag);
	fs_layers = get_json_field(manifest, "fsLayers");
	char **layers = parse_json_array(fs_layers);
	pull_layers(image, layers, token, container_path);
	
	free(token);
	free(manifest);
	free(image);
	free(tag);

	pipe(stdout_pipe);
	pipe(stderr_pipe);

	unshare(CLONE_NEWPID);
	int child_pid = fork();
	
	if (child_pid == -1)
	{
		printf("Error forking!");
		return 1;
	}

	if (child_pid == 0)
	{
		status = chroot(container_path);
		dup2(stdout_pipe[1], STDOUT_FILENO);
		dup2(stderr_pipe[1], STDERR_FILENO);
		close(stdout_pipe[1]);
		close(stderr_pipe[1]);
		close(stdout_pipe[0]);
		close(stderr_pipe[0]);
		execv(command, &argv[3]);
	}
	else
	{
		close(stdout_pipe[1]);
		close(stderr_pipe[1]);
		wait(&status);
	}

	free(container_path);
	write_output(stdout_pipe[0], stderr_pipe[0]);

	return WEXITSTATUS(status);
}
