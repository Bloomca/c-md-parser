#include <stdlib.h>
#include "parserState.h"

void resetParserLineState(ParserState *parserState) {
    parserState->canParseHeader = 1;
    parserState->headerLevel = 0;
    parserState->openedTag = 0;
    parserState->firstLine = 1;
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