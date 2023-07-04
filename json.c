#include "mydocker.h"

static void skip_whitespace(char *str, int *i)
{
    while (str[*i] == ' ' || str[*i] == '\n')
    {
        (*i)++;
    }
}

char *substr(char *str, int start, int len)
{
    char *sub = (char *)malloc(sizeof(char) * (len + 1));
    sprintf(sub, "%.*s", len, str + start);
    return sub;
}

static int count_char(char *str, char c)
{
    int count;

    count = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == c)
            count++;
    }
    return count;
}

static char *get_array_str(char *json, int *i)
{
    int open;
    int start;

    start = *i;
    open = 0;
    while (json[*i] != '\0')
    {
        if (json[*i] == '[')
            open++;
        else if (json[*i] == ']')
            open--;

        if (open == 0)
        {
            (*i)++;
            return substr(json, start, *i - start);
        }

        (*i)++;
    }
}

static char **parse_pair(char *json, int *i)
{
    int start;
    char **pair;

    pair = (char **)malloc(sizeof(char *) * 2);

    skip_whitespace(json, i);
    if (json[*i] != '"')
        return NULL;
    (*i)++;
    start = *i;
    while (json[*i] != '"' && json[*i] != '\0')
        (*i)++;
    if (json[*i] != '"')
        return NULL;
    pair[0] = substr(json, start, *i - start);
    (*i)++;
    skip_whitespace(json, i);
    if (json[*i] != ':')
        return NULL;
    (*i)++;
    skip_whitespace(json, i);
    if (isdigit(json[*i]))
    {
        start = *i;
        while (isdigit(json[*i]))
            (*i)++;
        pair[1] = substr(json, start, *i - start);
    }
    else if (json[*i] == '"')
    {
        (*i)++;
        start = *i;
        while (json[*i] != '"' && json[*i] != '\0')
            (*i)++;
        if (json[*i] != '"')
            return NULL;
        pair[1] = substr(json, start, *i - start);
        (*i)++;
    }
    else if (json[*i] == '[')
    {
        pair[1] = get_array_str(json, i);
    }
    else
    {
        return NULL;
    }

    return pair;
}

static void free_pair(char **pair)
{
    free(pair[0]);
    free(pair[1]);
    free(pair);
}

char *get_json_field(char *json, char *field)
{
    int  i;
    char **pair;

    i = 0;
    if (json[i] != '{')
        return NULL;
    i++;
    while (json[i] != '}' && json[i] != '\0')
    {
        pair = parse_pair(json, &i);
        if (pair == NULL)
            return NULL;
        if (!strcmp(pair[0], field))
        {
            free(pair[0]);
            return pair[1];
        }
        free_pair(pair);
        if (json[i] == ',')
            i++;
    }

    return NULL;
}

char **parse_json_array(char *json)
{
    int  i;
    int  len;
    int  start;
    char **array;

    i = 0;
    if (json[i] != '[')
        return NULL;
    len = count_char(json, ',') + 1;
    array = (char **)malloc(sizeof(char *) * (len + 1));
    i++;
    
    for (int k = 0; k < len; k++)
    {
        skip_whitespace(json, &i);
        start = i;
        while (json[i] != ',' && json[i] != ']')
            i++;
        array[k] = substr(json, start, i - start);
        i++;
    }
    array[len] = NULL;

    return array;
}
