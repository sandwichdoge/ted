#include "fileops.h"

size_t file_get_size(char *path)
{
    FILE *fd = fopen(path, "r");
    if (fd == NULL) return -1;
    fseek(fd, 0L, SEEK_END);
    size_t ret = ftell(fd);
    fclose(fd);
    return ret;
}


char **file_read_to_array(char *path, int *line_count)
{
    size_t sz = file_get_size(path);
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        *line_count = -1;
        return NULL;
    }
    
    char* buf = (char*)malloc(sz);
    fread(buf, sz, 1, fd);

    char **lines = str_split(buf, "\n", line_count);
    
    if (*line_count == 0) { //There's only 1 line
        lines = malloc(sizeof(char*));
        lines[0] = malloc(sz + 1);
        strcpy(lines[0], buf);
        *line_count += 1;
    }
    
    free(buf);

    return lines;
}



/*This is a customized version
 *It will check the flg of line to see if the linebreak is real or was generated for viewing
 *flg_pos is always the last element of line, right after NULL TERM
 */
int list_write_to_file(line_t *head, char *path, int flg_pos)
{
    FILE *fd = fopen(path, "w");
    if (fd == NULL) return -1;

    char *buf = calloc(flg_pos, 1);
    int len;

    /*print a line, then print LF*/
    while (head) {
        memset(buf, 0, flg_pos);
        strncpy(buf, head->str, flg_pos-1);
        len = strlen(buf);

        /*only write LF for lines with real LF*/
        if (head->str[flg_pos] == 1) {
            buf[len] = '\n';
            len++;
        }

        fwrite(buf, len, 1, fd);
        head = head->next;
    }

    free(buf);
    fclose(fd);
    
    return 0;
}
