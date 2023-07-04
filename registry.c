#include "mydocker.h"

static void init_string(string *str)
{
	str->len = 0;
	str->ptr = malloc(str->len + 1);
	str->ptr[0] = '\0';
}

static size_t curl_write_function(void *ptr, size_t size, size_t nmemb, string* str)
{
	size_t new_len;

	new_len = str->len + size * nmemb;
	str->ptr = realloc(str->ptr, new_len + 1);
	memcpy(str->ptr + str->len, ptr, size * nmemb);
	str->ptr[new_len] = '\0';
	str->len = new_len;
	return size * nmemb;
}

char *get_registry_token(char *repo)
{
	CURL     *curl;
	CURLcode res;
	int      status;
	string   body;
	char     *token;
	char     url[128];

	curl = curl_easy_init();
	if (!curl)
		return NULL;

	sprintf(url, "https://auth.docker.io/token?service=registry.docker.io&scope=repository:library/%s:pull", repo);
	init_string(&body);
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
	
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	curl_easy_cleanup(curl);
	
	if (status != 200)
	{
		free(body.ptr);
		return NULL;
	}

	token = get_json_field(body.ptr, "token");
	free(body.ptr);
	return token;
}

char *get_manifest(char *token, char* image, char *tag)
{
	CURL     *curl;
	CURLcode res;
	int      status;
	string   body;
	char     url[128];

	curl = curl_easy_init();
	if (!curl)
		return NULL;

	sprintf(url, "https://registry.hub.docker.com/v2/library/%s/manifests/%s", image, tag);
	init_string(&body);
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
	curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
	
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	curl_easy_cleanup(curl);
	
	if (status != 200)
		return NULL;

	return body.ptr;
}
