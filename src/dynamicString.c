#include <stdlib.h>
#include <string.h>
#include <dynamicString.h>

static size_t MIN_CAPACITY = 64;

DynamicString createDynStr(const char * initialStr, size_t len) {
    DynamicString dynStr;

    dynStr.capacity = len * 2;

    if (dynStr.capacity < MIN_CAPACITY) {
        dynStr.capacity = MIN_CAPACITY;
    }

    dynStr.str = malloc(dynStr.capacity);
    
    if (!dynStr.str) {
        // ?????
        printf("Could not allocate dynamic string");
        exit(1);
    }

    strcpy(dynStr.str, initialStr);
    dynStr.len = len;

    return dynStr;
}

int appendDynStr(DynamicString * dynStr, const char * newStr, size_t len) {
    // 1 is needed for the NULL terminator
    int newCapacity = dynStr->len + len + 1;
    if (!ensureCapacity(dynStr, newCapacity)) {
        return 0;
    }

    memcpy(dynStr->str + dynStr->len, newStr, len);
    dynStr->len = dynStr->len + len;

    return 1;
}

int appendDynCh(DynamicString * dynStr, const char newCh) {
    int newCapacity = dynStr->len + 2;
    if (!ensureCapacity(dynStr, newCapacity)) {
        return 0;
    }

    dynStr->str[dynStr->len] = newCh;
    dynStr->len++;
    dynStr->str[dynStr->len] = '\0';

    return 1;
}

static int ensureCapacity(DynamicString * dynStr, size_t neededCapacity) {
    if (dynStr->capacity > neededCapacity) {
        // we already have enough memory for new value
        return 1;
    }

    int newCapacity = dynStr->capacity;
    while (newCapacity < neededCapacity) {
        newCapacity = newCapacity * 2;
    }

    char * newStr = realloc(dynStr->str, newCapacity);

    if (!newStr) {
        return 0;
    }

    dynStr->str = newStr;
    dynStr->capacity = newCapacity;

    return 1;
}

void freeDynStr(DynamicString * dynStr) {
    free(dynStr->str);
    dynStr->str = NULL;
    dynStr->capacity = 0;
    dynStr->len = 0;
}