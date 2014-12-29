uSQL
====

uSQL stands for unnamedSQL, a course project for Introduction to Database Systems in 2014 fall, Tsinghua University.

How to run
----

- Install bison, flex etc.
- Run `make`
- `./usql` or `./usql < input.sql`

Command line options
----

- `-d target.db`: use `target.db` as target directory, databases will be stored here
- `-t`: print execution time after every sql sentences
- `-v`: more verbose
- `-g`: enable debug on parser
- `-h`: help

Supported SQL Statement
----

see `parser/sql.yy`

TODO
----

- Better support for `NULL`
- Show meaningful information on "syntax error"
- Improve peformance when selecting from multiple tables
- Write more TODOs
