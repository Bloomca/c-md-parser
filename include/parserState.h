#pragma once

#include "dynamicString.h"

typedef enum {
    NONE,
    ITALIC,
    BOLD,
    INLINE_CODE,
} Tag;

typedef struct ParserState {
    int canParseHeader;
    int headerLevel;
    int openedTag;
    char previousCharacter;
    Tag tag;
    DynamicString str;
    // subState is always a pointer on heap, you need to free it manually
    struct ParserState *subState;
} ParserState;

void resetParserLineState(ParserState *parserState);
void resetParserStr(ParserState *parserState);
ParserState createParserState(Tag tag);
ParserState * getNestedState(ParserState *parserState);
void createSubState(ParserState *parserState, Tag tag);
void concludeSubState(ParserState *parserState, Tag tag);
int appendStringToParser(ParserState *parserState, char *text, size_t len);
int appendCharToParser(ParserState *parserState, char ch);
void increaseHeaderLevel(ParserState * parserState);