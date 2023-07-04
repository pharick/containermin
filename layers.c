#include "mydocker.h"

static int unpack_layer(char *filename, char *container_path)
{
    int  res;
    char command[128];
    
    sprintf(command, "tar xf %s -C %s", filename, container_path);
    res = system(command);
}

static size_t curl_file_write_function(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static int pull_one_layer(char *image, char *sum, char *token, char *container_path)
{
    CURL     *curl;
	CURLcode res;
	int      status;
    char     url[256];
    char     filename[128];
    FILE     *file;

    curl = curl_easy_init();
	if (!curl)
		return -1;

    sprintf(url, "https://registry.hub.docker.com/v2/library/%s/blobs/%s", image, sum);
    sprintf(filename, "%s.tar", sum);
    file = fopen(filename, "wb");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
	curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_file_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    fclose(file);
    
    if (status != 200)
    {
		return -1;
    }

    unpack_layer(filename, container_path);

    return 0;
}

void pull_layers(char *image, char **layers, char *token, char *container_path)
{
    char *sum;
    int  res;

	for (int i = 0; layers[i] != NULL; i++)
	{
        sum = get_json_field(layers[i], "blobSum");
        res = pull_one_layer(image, sum, token, container_path);
	}
}
