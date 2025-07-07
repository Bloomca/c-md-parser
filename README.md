## C Markdown Parser

This is a simple library for parsing markdown in pure C. This is a fun/learning project, not really meant for any sort of production use.

Currently, it supports:

- headers
- line breaks
- italics
- bold text
- inline code

So it can parse this:
```
## Header

Description _line1_
Description **line2**

Some `inline code`
```

## Local Setup

This project uses [xmake](https://xmake.io). By default, running `xmake` will install the testing library ([cmocka](https://cmocka.org/)) dependency and build all 3 targets.

If you want to use it, you probably want to build the dynamic library, you can target it with `xmake build parserlib`, and if you want to develop some feature, you probably only care about running tests, which can be executed with `xmake run tests`.