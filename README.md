编程约束：
对外开放的接口：大写_下划线
内部接口：static 小写或__xx

i,l -> int
f,d -> float
z -> bool
s -> string
v -> void
O -> Object
[ -> array
-----
A -> any
-----
## 包管理



## 编译
编译文件
编译包
编译项目
安装
运行
部署

## 目录
bin/
 |--tools/
 |   |--compile
 |   |--disassemble
 |   |--pack
 |   |--install
 |   |--get
 |   |--put
 |   `--run
 |--koala
doc/
cfg/
 |--settings.cfg
src/
Makefile

-----------------------------------------------------------------------------
settings.cfg
repository=~/koala_repo

```

project
    |--verson.cfg
    |--src
    |   |--koala/
    |   |   |--lang/
    |   |       |--string.kl
    |   |       |--klass.kl
    |   |       |--person.kl
    |   |       `--greeting.kl
    |   `--demo.kl
    |--target
    |   |--koala/
    |   |   |--lang/
    |   |       |--string.klc
    |   |       |--klass.klc
    |   |       |--person.klc
    |   |       `--greeting.klc
    |   `--demo.klc
    |--pkg
    |   |--koala
    |   |   |--lang.klc
    |   `--demo.klc

单文件编译(找不到符号报错，指定源文件和目标文件)
compile src/koala/lang/person.kl -o target/koala/lang/person.klc -kpath target/
compile src/koala/lang/greeting.kl -o target/koala/lang/greeting.klc -kpath target/
package target/koala/lang/* -o pkg/koala/lang.klc

单文件编译
compile src/demo.kl -o target/demo.klc
package target/demo.klc -o target/demo.klc

run demo(.klc) 123 "abc" -kpath pkg/

编译查找顺序：
 內建
 当前project下的target
 当前project下的源码
 repo

koala build
 1. 强制package名和目录名一样
 2. 强制项目结构
 3. 同一个包的不同文件一起编译
 4. 不同包的相互依赖，同时编译多个包(共用编译环境)
    import，函数原型，变量类型，结构体类型
    编译进行多次编译，第一遍无法解析的语句，放到第二遍解析
    编译环境包含多个包，可以互相访问符号
    不同包之间是相互交叉编译

koala install
 1. 复制文件到本地repo中

koala run demo(.klc) 123 "abc" -kpath pkg/

verson.cfg
 koala/lang@0.1.2
 hello@1.2

虚拟机加载按照.klc文件为单位进行加载
编译一个kl文件的时候，如果依赖同一个包下的其他kl文件，则同时编译这2个文件???
  编译单个文件，不自动编译其依赖文件??
    同一个包，自动编译?? 如何判断是同一个包呢?编译哪个文件呢?
      找不到本地符号报错(同包的其他文件中的符号也认为是本地符号)
    不同包呢?? 自动编译外部包，根据import "koala/lang"
    不允许循环依赖(包内和不同包都不允许出现循环依赖)
  编译包，同时编译包下的所有kl文件，共享编译环境
  编译包，外部包依赖??? 不编译，找不到就报错，不允许相互依赖
```

同一个包可以放在不同路径上，使用方法和其他包一样,导入再使用
版本使用git的tag进行管理
install自动打tag，版本已经存在自动覆盖
编译器和虚拟机版本号
  编译器可以有多个compile同时发布，koala选择不同的版本
  虚拟机可以有多个run同时发布，koala选择不同的版本
__.PKG包含编译器和虚拟机版本号

## 类型系统


## QA
  xml封装
  编译解析xml文件??

## klc结构
index | type        | name      | attribute
  0      1,var          1           2
  1      2,func         3           4
  2      3,struct     index(Hello)
         4,intf
         5,field      index      Hello, 2
         6,method     index      Hello, 4
constant:

1  (
2  )
3  (V)
4 hello
5 koala/lang.string
6 SayHello
7 (koala/lang.string;)(V)
8 100
  1 5 ; 2 3

var_desc:
 name_index
 descriptor_index

field_desc:
 name_index
 descriptor_index
 struct_name_index

func_desc:
 name_index
 descriptor_index
 code_index
 code_length

method_desc:
 name_index
 descriptor_index
 struct_name_index
 code_index

struct_desc:
 name_index

code_desc:
 stack_size
 code_index
 code_length

code_area:
 xxx
 yyy
 zzz

