# C/C++ 小工具程序
***
自用的一些工具类，陆续补充

Utils class for tiny issue
***


## keyValueConfigFile
```
class name: ConfigFileOperation

读写 key=value 的配置文件的工具类，
1. 支持 # 号注释
2. 支持 key=int
3. 支持 key=string
 
read and write key=value file. 
1. support '#' to comment
2. support key=int
3. support key=string

Compile：
g++ ConfigFileOperation.cpp  -o main -D _TEST_
