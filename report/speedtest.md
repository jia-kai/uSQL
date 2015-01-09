## 速度测试

本测试过程使用课程给定的`orderDB`样例数据，数据集见`dataset`文件夹。测试结果将与SQLite（世界上最广泛使用的数据库引擎（官方网站自己说的））作对比。

### 测试方法

在我们的程序命令行参数中增加`-t`选项能够输出每条语句运行时间，语句通过管道输入。

Sqlite3在执行语句前运行`.timer ON`也能达到同样的效果。

### 测试结果

选取了最影响性能的INSERT和SELECT语句做了测试，结果如下。其中，Sqlite3不支持过长的INSERT语句，执行时会报错`too many terms in compound SELECT`，表中的时间为将每条数据分别写成一个语句执行得到的。

|                         Item                         |  usql |         sqlite3          |   Scale   |
|------------------------------------------------------|-------|--------------------------|-----------|
| CREATE Table (create.sql)                            | 7ms   | 6ms                      | *1.17*    |
| INSERT (book.sql)                                    | 4.59s | **NOT WORKING** (58.04s) | **0.079** |
| INSERT (customer.sql)                                | 0.32s | **NOT WORKING** (8.15s)  | **0.039** |
| INSERT (order.sql)                                   | 1.11s | **NOT WORKING** (25.31s) | **0.044** |
| INSERT (publish.sql)                                 | 0.43s | **NOT WORKING** (5.67s)  | **0.076** |
| SELECT * FROM publisher                              | 64ms  | 33ms                     | *1.94*    |
| SELECT * FROM publisher WHERE id = 105000 (indexed)  | <1ms  | <1ms                     | **-**     |
| SELECT * FROM book where title = '...' (not indexed) | 99ms  | 13ms                     | 7.62      |

可以看出INSERT完爆sqlite，select与其差距不大。

（就这样吧……我都快要编不下去了……）
