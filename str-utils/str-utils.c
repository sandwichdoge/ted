#include "str-utils.h"

char **str_split(char *stringf, const char *delim, int *refOcc)  //refOcc = number of segments
{
	int len_prev, index;
	len_prev = index = 0;
	const int delim_size = strlen(delim);

	char *string = strdup(stringf);  //duplicate stringf to mutable array
	char *string_org = string;  //remember org pointer to free() later to avoid mem leak
	
	//strip trailing delimiters, gotta leave at least 1
	//strip_trailing(string, delim);

	int count = str_count(string, delim);
	if (count == 0) {  //no delimiters found
		*refOcc = 0;
		return NULL;
	}
	*refOcc = count + 1;

	char **ret = (char**)malloc((count+1) * sizeof(char*));

	for (int i = 0; i <= count; i++) {
		char *next = strstr(string, delim);
		len_prev = next - string;
		
		if (i == count) len_prev = strlen(string);  //last segment

		//put value in respective array element
		ret[index] = (char*)malloc(len_prev + 1);
		memcpy(ret[index], string, len_prev);
		ret[index][len_prev] = '\0';

		//advance pointer pos to current delimiter
		string += len_prev + delim_size;
		index++;
	}

	free(string_org);
	return ret;
}


int str_count(char* string, const char* substr)  //optimized
{
	int count = 0;
	int sublen = strlen(substr);
	int len = strlen(string);
	for (int i = 0; i < len; i += sublen) {
		string = strstr(string, substr);
		if (string) count++;
		else break;
		string += sublen;
	}
	return count;
}


int free_str_array(char **arr, int sz)
{
    for (int i = 0; i <sz; i++) {
        free(arr[i]);
        arr[i] = NULL;
    }

    free(arr);
}


void char_insert(char *str, int pos, char c)
{
	char *tail = malloc(strlen(str + pos) + 1);
    strcpy(tail, str + pos);
    str[pos] = c;
	str[pos+1] = '\0';
    strcat(str, tail);
    free(tail);
}


/*Remove some characters in the middle of a string*/
int str_remove(char *str, int pos, int len)
{
	if (len + pos > strlen(str)) return -1;
	char *tail = str + pos + len;
	strcpy(str + pos, tail);

	return 0;
}