#include <stdio.h>

void parseMarkdown(FILE * file) {
    char * line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, file)) != -1) {
        parseLine(line);
    }
}

char * parseLine(char * line) {
    int firstElement = 1;
    int headerLevel = 0;
    while (*line) {
        // 0x80 is 10000000
        // Unicode continuation bits look like this:
        // 110xxxxx 10xxxxxx = 2-byte character
        // 1110xxxx 10xxxxxx 10xxxxxx = 3-byte character
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx = 4-byte character
        //
        // Finally, 1-byte characters look like 0xxxxxxx
        // All special markdown characters are 1 byte
        int isASCII = (*line & 0x80) == 0;
        if (isASCII) {
            if (firstElement) {
                if (*line == "#") {
                    headerLevel++;
                } else {
                    firstElement = 0;
                }
            }
        }

        line++;
    }
}
