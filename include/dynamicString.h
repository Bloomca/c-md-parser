#pragma once

#include <stddef.h>

typedef struct {
    size_t len;
    size_t capacity;
    char * str;
} DynamicString;

DynamicString createDynStr(const char * initialStr, size_t len);
int appendDynStr(DynamicString * dynStr, const char * newStr, size_t len);
void freeDynStr(DynamicString * dynStr);