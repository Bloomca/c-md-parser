#include <stdio.h>
#include <dynamicString.h>

typedef struct {
    int canParseHeader;
    int headerLevel;
    int openedTag;
} ParserState;

static int isASCIICharacter(char ch);

void resetParserLineState(ParserState * parserState) {
    parserState->canParseHeader = 1;
    parserState->headerLevel = 0;
    parserState->openedTag = 0;
}

ParserState createParserState() {
    ParserState parserState;

    resetParserLineState(&parserState);

    return parserState;
}

void increaseHeaderLevel(ParserState * parserState) {
    if (parserState->headerLevel < 6) {
        parserState->headerLevel++;
    }
}

int prependOpeningTag(ParserState * parserState, DynamicString * resultDynStr) {
    parserState->openedTag = 1;

    if (parserState->headerLevel == 0) {
        return appendDynStr(resultDynStr, "<p>", 3);
    }

    if (parserState->headerLevel == 1) {
        return appendDynStr(resultDynStr, "<h1>", 4);
    }

    if (parserState->headerLevel == 2) {
        return appendDynStr(resultDynStr, "<h2>", 4);
    }

    if (parserState->headerLevel == 3) {
        return appendDynStr(resultDynStr, "<h3>", 4);
    }

    if (parserState->headerLevel == 4) {
        return appendDynStr(resultDynStr, "<h4>", 4);
    }

    if (parserState->headerLevel == 5) {
        return appendDynStr(resultDynStr, "<h5>", 4);
    }

    if (parserState->headerLevel == 6) {
        return appendDynStr(resultDynStr, "<h6>", 4);
    }

    // should never happen
    return 0;
}

int appendOpeningTag(ParserState * parserState, DynamicString * resultDynStr) {
    if (parserState->openedTag == 0) {
        return 1;
    }

    if (parserState->headerLevel == 0) {
        return appendDynStr(resultDynStr, "</p>", 4);
    }

    if (parserState->headerLevel == 1) {
        return appendDynStr(resultDynStr, "</h1>", 5);
    }

    if (parserState->headerLevel == 2) {
        return appendDynStr(resultDynStr, "</h2>", 5);
    }

    if (parserState->headerLevel == 3) {
        return appendDynStr(resultDynStr, "</h3>", 5);
    }

    if (parserState->headerLevel == 4) {
        return appendDynStr(resultDynStr, "</h4>", 5);
    }

    if (parserState->headerLevel == 5) {
        return appendDynStr(resultDynStr, "</h5>", 5);
    }

    if (parserState->headerLevel == 6) {
        return appendDynStr(resultDynStr, "</h6>", 5);
    }

    // should never happen
    return 0;
}


void parseLine(char * line, size_t len, ParserState * parserState, DynamicString * resultDynStr) {
    while (*line) {
        int isASCII = isASCIICharacter(*line);
        if (isASCII) {
            if (*line == '#') {
                if (parserState->canParseHeader) {
                    increaseHeaderLevel(parserState);
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
                prependOpeningTag(parserState, resultDynStr);
            } else {
                int currentHeaderLevel = parserState->headerLevel;
                parserState->headerLevel = 0;
                prependOpeningTag(parserState, resultDynStr);

                if (currentHeaderLevel != 0) {
                    for (int i = 0; i < currentHeaderLevel; i++) {
                        appendDynChar(resultDynStr, '#');
                    }
                }

                appendDynChar(resultDynStr, *line);    
            }
        } else {
            appendDynChar(resultDynStr, *line);
        }

        line++;
    }

    appendOpeningTag(parserState, resultDynStr);
}

DynamicString parseMarkdown(FILE * file) {
    char * line = NULL;
    size_t len = 0;
    ssize_t nread;

    ParserState parserState = createParserState();
    DynamicString resultDynStr = createDynStr("", 0);

    while ((nread = getline(&line, &len, file)) != -1) {
        parseLine(line, len, &parserState, &resultDynStr);
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
