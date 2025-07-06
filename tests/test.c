#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static char * FILE_NAME = "test.md";

static FILE * setupFile(char * text) {
    FILE * fd = fopen(FILE_NAME, "w");
    if (fd == NULL) {
        exit(1);
    }

    fputs(text, fd);

    fclose(fd);

    fd = fopen(FILE_NAME, "r");
    return fd;
}

static void teardownFile(FILE * fd, DynamicString * dynStr) {
    fclose(fd);

    int result = remove(FILE_NAME);

    if (result != 0) {
        exit(1);
    }

    freeDynStr(dynStr);
}

static void test_correctParagraphWrapperTag(void **state) {
    (void) state; /* unused parameter */

    FILE * fd = setupFile("Hello, world!");

    DynamicString dynStr = parseMarkdown(fd);

    assert_string_equal(dynStr.str, "<p>Hello, world!</p>");

    teardownFile(fd, &dynStr);
}

static void test_correctHeaderWrapperTag(void **state) {
    (void) state; /* unused parameter */

    FILE * fd = setupFile("### Hello, world!");

    DynamicString dynStr = parseMarkdown(fd);

    assert_string_equal(dynStr.str, "<h3>Hello, world!</h3>");

    teardownFile(fd, &dynStr);
}

static void test_correctMultilineSupport(void **state) {
    (void) state; /* unused parameter */

    FILE * fd = setupFile("# Description\nSome text\n\nAnother line");

    DynamicString dynStr = parseMarkdown(fd);

    assert_string_equal(dynStr.str, "<h1>Description</h1><p>Some text</p><p>Another line</p>");

    teardownFile(fd, &dynStr);
}

static void test_correctItalicSupport(void **state) {
    (void) state; /* unused parameter */

    FILE * fd = setupFile("some text _with italics_ and more");

    DynamicString dynStr = parseMarkdown(fd);

    assert_string_equal(dynStr.str, "<p>some text <i>with italics</i> and more</p>");

    teardownFile(fd, &dynStr);
}

static void test_correctInlineCodeSupport(void **state) {
    (void) state; /* unused parameter */

    FILE * fd = setupFile("some text `with inline code` and _more_");

    DynamicString dynStr = parseMarkdown(fd);

    assert_string_equal(dynStr.str, "<p>some text <code>with inline code</code> and <i>more</i></p>");

    teardownFile(fd, &dynStr);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_correctParagraphWrapperTag),
        cmocka_unit_test(test_correctHeaderWrapperTag),
        cmocka_unit_test(test_correctMultilineSupport),
        cmocka_unit_test(test_correctItalicSupport),
        cmocka_unit_test(test_correctInlineCodeSupport),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}