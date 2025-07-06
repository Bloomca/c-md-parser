#include <stdio.h>
#include <stdlib.h>
#include <dynamicString.h>

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

static int isASCIICharacter(char ch);

void resetParserLineState(ParserState *parserState) {
    parserState->canParseHeader = 1;
    parserState->headerLevel = 0;
    parserState->openedTag = 0;
}

void resetParserStr(ParserState *parserState) {
    freeDynStr(&parserState->str);

    parserState->str = createDynStr("", 0);
}

ParserState createParserState(Tag tag) {
    ParserState parserState;

    parserState.tag = tag;
    parserState.str = createDynStr("", 0);
    parserState.subState = NULL;

    resetParserLineState(&parserState);

    return parserState;
}

ParserState * getNestedState(ParserState *parserState) {
    ParserState *nestedState = parserState;

    for (;;) {
        if (nestedState->subState == NULL) {
            break;
        }

        nestedState = nestedState->subState;
    }

    return nestedState;
}

void createSubState(ParserState *parserState, Tag tag) {
    ParserState *nestedState = getNestedState(parserState);

    ParserState *subState = malloc(sizeof(ParserState));
    *subState = createParserState(tag);
    nestedState->subState = subState;
}

void concludeSubState(ParserState *parserState, Tag tag) {
    // TODO: conclude all nested substates as just regular text
    // without wrapping it into tags

    // 1. Find substate for the tag
    // 2. Find substate/state for the parent of that tag
    // 3. Add substate text to the parent text, including
    // opening and closing tags

    ParserState *parentState = parserState;

    for (;;) {
        if (parentState->subState == NULL) {
            return;
        }

        if (parentState->subState->tag == tag) {
            break;
        }

        parentState = parentState->subState;
    }

    if (tag == ITALIC) {
        appendDynStr(&parentState->str, "<i>", 3);
    } else if (tag == INLINE_CODE) {
        appendDynStr(&parentState->str, "<code>", 6);
    } else if (tag == BOLD) {
        appendDynStr(&parentState->str, "<strong>", 8);
    }

    appendDynStr(&parentState->str, parentState->subState->str.str, parentState->subState->str.len);
    
    if (tag == ITALIC) {
        appendDynStr(&parentState->str, "</i>", 4);
    } else if (tag == INLINE_CODE) {
        appendDynStr(&parentState->str, "</code>", 7);
    } else if (tag == BOLD) {
        appendDynStr(&parentState->str, "</strong>", 9);
    }

    // cleanup
    freeDynStr(&parentState->subState->str);
    free(parentState->subState);
    parentState->subState = NULL;
}

int appendStringToParser(ParserState *parserState, char *text, size_t len) {
    return appendDynStr(&parserState->str, text, len);
}

int appendCharToParser(ParserState *parserState, char ch) {
    ParserState *nestedState = getNestedState(parserState);
    return appendDynChar(&nestedState->str, ch);
}

void increaseHeaderLevel(ParserState * parserState) {
    if (parserState->headerLevel < 6) {
        parserState->headerLevel++;
    }
}

int prependOpeningTag(ParserState *parserState) {
    parserState->openedTag = 1;

    if (parserState->headerLevel == 0) {
        return appendStringToParser(parserState, "<p>", 3);
    }

    if (parserState->headerLevel == 1) {
        return appendStringToParser(parserState, "<h1>", 4);
    }

    if (parserState->headerLevel == 2) {
        return appendStringToParser(parserState, "<h2>", 4);
    }

    if (parserState->headerLevel == 3) {
        return appendStringToParser(parserState, "<h3>", 4);
    }

    if (parserState->headerLevel == 4) {
        return appendStringToParser(parserState, "<h4>", 4);
    }

    if (parserState->headerLevel == 5) {
        return appendStringToParser(parserState, "<h5>", 4);
    }

    if (parserState->headerLevel == 6) {
        return appendStringToParser(parserState, "<h6>", 4);
    }

    // should never happen
    return 0;
}

int appendOpeningTag(ParserState *parserState) {
    if (parserState->openedTag == 0) {
        return 1;
    }

    if (parserState->headerLevel == 0) {
        return appendStringToParser(parserState, "</p>", 4);
    }

    if (parserState->headerLevel == 1) {
        return appendStringToParser(parserState, "</h1>", 5);
    }

    if (parserState->headerLevel == 2) {
        return appendStringToParser(parserState, "</h2>", 5);
    }

    if (parserState->headerLevel == 3) {
        return appendStringToParser(parserState, "</h3>", 5);
    }

    if (parserState->headerLevel == 4) {
        return appendStringToParser(parserState, "</h4>", 5);
    }

    if (parserState->headerLevel == 5) {
        return appendStringToParser(parserState, "</h5>", 5);
    }

    if (parserState->headerLevel == 6) {
        return appendStringToParser(parserState, "</h6>", 5);
    }

    // should never happen
    return 0;
}


void parseLine(char *line, size_t len, ParserState *parserState, DynamicString *resultDynStr) {
    while (*line) {
        ParserState *nestedState = getNestedState(parserState);
        int isASCII = isASCIICharacter(*line);
        if (isASCII) {
            if (*line == '#') {
                if (parserState->canParseHeader) {
                    increaseHeaderLevel(parserState);
                    line++;
                    continue;
                }
            }

            if (*line == '_') {
                if (nestedState->tag == ITALIC) {
                    concludeSubState(parserState, ITALIC);
                    line++;
                    continue;
                } else {
                    createSubState(parserState, ITALIC);
                    line++;
                    continue;
                }
            }

            if (*line == '`') {
                if (nestedState->tag == INLINE_CODE) {
                    concludeSubState(parserState, INLINE_CODE);
                    line++;
                    continue;
                } else {
                    createSubState(parserState, INLINE_CODE);
                    line++;
                    continue;
                }
            }

            if (*line == '*') {
                if (nestedState->previousCharacter == '*') {
                    if (nestedState->tag == BOLD) {
                        concludeSubState(parserState, BOLD);
                        ParserState *newNestedState = getNestedState(parserState);
                        newNestedState->previousCharacter = 'a';
                        line++;
                        continue;
                    } else {
                        createSubState(parserState, BOLD);
                        // random character
                        nestedState->previousCharacter = 'a';
                        line++;
                        continue;
                    }
                } else {
                    nestedState->previousCharacter = '*';
                    line++;
                    continue;
                }
            }

            // we ignore newlines for now
            if (*line == '\n') {
                line++;
                continue;
            }
        }

        if (parserState->canParseHeader) {
            parserState->canParseHeader = 0;

            if (*line == ' ') {
                prependOpeningTag(parserState);
            } else {
                int currentHeaderLevel = parserState->headerLevel;
                parserState->headerLevel = 0;
                prependOpeningTag(parserState);

                if (currentHeaderLevel != 0) {
                    for (int i = 0; i < currentHeaderLevel; i++) {
                        appendCharToParser(parserState, '#');
                    }
                }

                appendCharToParser(parserState, *line);
            }
        } else if (nestedState->previousCharacter == '*') {
            appendCharToParser(parserState, '*');
            appendCharToParser(parserState, *line);
        } else {
            appendCharToParser(parserState, *line);
        }

        nestedState->previousCharacter = *line;
        line++;
    }

    appendOpeningTag(parserState);
}

DynamicString parseMarkdown(FILE *file) {
    char * line = NULL;
    size_t len = 0;
    ssize_t nread;

    Tag noTag = NONE;
    ParserState parserState = createParserState(noTag);
    DynamicString resultDynStr = createDynStr("", 0);

    while ((nread = getline(&line, &len, file)) != -1) {
        parseLine(line, len, &parserState, &resultDynStr);

        appendDynStr(&resultDynStr, parserState.str.str, parserState.str.len);

        resetParserStr(&parserState);
        resetParserLineState(&parserState);
    }

    return resultDynStr;
}

// internal

// Unicode continuation bits look like this:
// 110xxxxx 10xxxxxx = 2-byte character
// 1110xxxx 10xxxxxx 10xxxxxx = 3-byte character
// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx = 4-byte character
//
// Finally, 1-byte characters look like 0xxxxxxx
// All special markdown characters are 1 byte
static int isASCIICharacter(char ch) {
    // 0x80 is 10000000
    return (ch & 0x80) == 0;
}
