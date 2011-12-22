The qCGI language was created by Richard Quisumbing. It is a web preprocessor that offloads the program logic and flow control to an SQL server.

This implementation was created because I wanted some features not available in the original implementation (I think it was a simple elseif statement) and hated the proprietary parts of it (activation keys on a per hardware basis). This was coded as a proof of concept and not meant to replace the initial implementation.

This was coded in August 2010 when I was a newbie programmer and a year out of college. I'm open-sourcing it for educational purposes. I hope curious developers, specially web developers who are just starting out, can learn something from this and build their own implementations. Working on this taught me a whole lot about the inner workings of a web app and the lessons learned have gone a long way.

As this was only a toy project, there are many details you have to find out yourself. But here are a few tips:

Syntax
------

All qCGI commands are enclosed in `{.` and `}` tags. Check the example: [test.v][test]

Compiling
---------

To compile, you will need `g++` and the following libraries:

1. MySQL Client (mysqlclient) - this implementation supports MySQL only
2. SQLite (sqlite3) - session data is store in an SQLite DB
3. FastCGI (fcgi) - it's just way better than plain CGI
4. RudeCGI (rudecgi) - used for CGI parsing, escaping, etc..
5. Boost Regex (libboost\_regex) - I know regular expressions are not recommended for parsing but it was very easy to parse the whole language with a single regex

You will also need a modded RudeCGI library to support FastCGI.

Configuration
-------------

Many config parameters are still hardcoded into the code, like the connection to MySQL, so you'd have to edit that or make it read a config file.

Disclaimer
----------

This is implementation is far from production ready. Use it at your own risk.

[test]: https://github.com/engwan/qcgi/blob/master/test.v
