#include <stdio.h>
#include <dynamicString.h>

typedef struct {
    int canParseHeader;
    int headerLevel;
    int openedTag;
    DynamicString str;
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

ParserState createParserState() {
    ParserState parserState;

    parserState.str = createDynStr("", 0);

    resetParserLineState(&parserState);

    return parserState;
}

int appendStringToParser(ParserState *parserState, char *text, size_t len) {
    return appendDynStr(&parserState->str, text, len);
}

int appendCharToParser(ParserState *parserState, char ch) {
    return appendDynChar(&parserState->str, ch);
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


void parseLine(char *line, size_t len, ParserState * parserState, DynamicString *resultDynStr) {
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

            if (*line == '_') {
                // we need to create a substructure and save all characters inside it
                // the reason is that it might not be closed
                // TODO: handle that
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
        } else {
            appendCharToParser(parserState, *line);
        }

        line++;
    }

    appendOpeningTag(parserState);
}

DynamicString parseMarkdown(FILE *file) {
    char * line = NULL;
    size_t len = 0;
    ssize_t nread;

    ParserState parserState = createParserState();
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
