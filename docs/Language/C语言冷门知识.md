# C语言冷门知识

## 1. __attribute__

具体请参见[官方文档](https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html)。👏

这里还是建议看官方文档而不是网络搜索，因为每个人的翻译水平有限，很可能产生歧义，导致更多的错误。

`__attribute__` 是 GNU C 特色之一，在系统中随处可见。它可以设置函数属性，变量属性和类型属性。

- 函数属性：`noreturn, noinline, always_inline, pure, const, nothrow, sentinel, format, format_arg, no_instrument_function, section, constructor, destructor, used, unused, deprecated, weak, malloc, alias, warn_unused_result, nonnull`
- 类型属性：`aligned, packed, transparent_union, unused, deprecated, may_alias`
- 变量属性：`aligned, packed`

属性的书写格式为 `__attribute__((xxx))`。当然，你也可以在这些属性关键字前后添加下划线，来防止头文件中存在同名的宏导致的错误，如：`__attribute__((__xxx__))`。

这些属性将影响编译器的行为，告知编译器它应该如何编译程序。

### 1.1. format

`__attribute__((format(archetype, string-index, first-to-check)))`

format 属性可以给被声明的函数加上类似 `printf` 和 `scanf` 的特征，使编译器检查函数声明和实际调用参数之间的格式化字符串是否匹配。

- `archetype`：只能填入 `printf` 或 `scanf`，指定按照哪种方式来检测参数；
- `string-index`：说明格式化字符串是第几个参数；
- `first-to-check`：指定第一个用于格式化可变参数是传入的第几个参数

这里需要注意的是：如果函数是一个成员函数，那么他还会传入一个隐藏的 `this` 指针作为第一个参数。这时，需要注意指定的参数位置是否正确！

```cpp
extern void log(const char *fmt, ...) __attribute__((__format__(printf, 1, 2)));

log("integer: %d\n", 0);
log("string: %s\n", "abc");
```

上述代码中，属性告知编译器，检查从函数 `log` 第 2 参数开始的参数是否能依次格式化到第 1 个参数指定的字符串中。

### 1.2. noreturn

该属性告诉编译器，函数永远不会 return，而是退出程序。在使用该属性时，建议开启 `-O2` 优化，编译器会自动将该函数的 return 语句忽略，并且直接退出。该函数之后的所有代码都会被优化掉（忽略）。

通知编译器该函数永不返回（即退出程序），只能用来修饰函数的声明；如果修饰函数的定义，则会导致 Clang 告警和 GCC 报错。

如果你使用这个属性修饰某一个函数，请记得在函数出口使用 `exit` 保证程序的正确退出；否则不仅会导致编译器的警告，Clang 编译的程序还会导致意想不到的情况。

下面给出官方的例子：

```cpp
void fatal () __attribute__ ((__noreturn__));
          
void fatal (/* ... */) {
  /* ... */ /* Print error message. */ /* ... */
  exit (1);
}
```

### 1.3.  packed

该属性指定一个变量或结构体的某个字段应该按照尽可能小的对齐方式，除非使用 aligned 属性指定了更大的值。所以，packed 属性可以达到让该变量、结构体或结构体字段不要对齐的效果。

```cpp
struct __attribute__((__packed__)) packed_struct_t {
    char a;
    int b;
    char c;
};
```

如上结构体将之占用 6 字节；而如果不使用该属性修饰，则由于默认 4 字节对其而占用 12 字节。

## 2. C 语言宏定义

### 2.1. # 运算符

# 运算符将宏的一个参数转换为字符串字面量。它仅允许出现在带参数的宏的替代列表中 (# 运算符所执行的操作可以理解为“字符串化”)。# 运算符与后面的参数之间可以有空格或缩进。

通常在程序的调试过程中，我们可能希望打印变量的值来观察程序的走向，如“a = 3”等。那我们可以这么写：

```cpp
#define print_int(n) printf(# n " = %d\n", (n))
```

当你调用 `print_int(i/j)` 时，会被预处理为 `printf("i/j"" = %d\n", i/j)` 。

### 2.2. ## 运算符

它可以将两个符号“粘合”在一起，成为一个符号。如果其中一个符号是宏参数，“粘合”操作会在参数完成替换后发生。同样，记号与操作符之间可以有空格和缩进。

如 `#define CONCAT(x,y) x ##y`，调用 `CONCAT(a,b)` 则会得到 ab。但是，如果你想使用 `CONCAT(a,CONCAT(b,c))` 则会得到奇怪的结果。

问题在于 `CONCAT(a,CONCAT(b,c))` 不会按照"正常"的方式扩展 `CONCAT(b,c)` 得出 bc。然后 `CONCAT(a,bc)` 给出 abc。在替换列表中，位于 ## 运算符之前和之后的宏参数在替换时不被扩展，因此，`CONCAT(a,CONCAT(b,c))` 扩展为 `aCONCAT(b,c)`，而不会进一步扩展，因为没有名为 `aCONCAT` 的宏。

你可以通过再定义一个宏来解决这个问题：`#define CONCAT2(x,y) CONCAT(x,y)`。这样调用 `CONCAT2(a,CONCAT2(b,c))` 就会得到 abc。

### 2.3. 宏定义可变参数

宏定义也支持可变参数，使用 `..., __VA_ARGS__` 配合实现，也可以结合其他宏定义使用：

```cpp
#define FOO(a, ...) a##__VA_ARGS__
FOO(1, 2, 3)    // 预处理为：12,3
#define PRINT_SELF(...) #__VA_ARGS__
PRINT_SELF(1, 2, 3)    // 预处理为 "1, 2, 3"
```

### 2.4.（嵌套）宏定义的展开顺序

宏展开只是简单的字符串替换。但我们在使用宏定义的时候也经常会遇到嵌套展开等复杂的情况。因此了解宏定义展开的顺序尤为重要：

- 不会递归展开：宏定义展开虽然可能需要做多次替换，但是一定不会递归展开；
- 有内而外展开：先展开内层宏，再展开外层宏；
- 如果宏定义中某参数前有 # 运算符，则不展开对应参数，而是直接将该参数转为一个字符串；
- 如果宏定义中某参数前/后有 ## 运算符，则不展开对应参数，而是将 ## 运算符前后的参数连接成为一个新的符号（不是字符串）

因此，假如我们定义了 `#define f(a) f(2 * (a))` 但是调用时对应的宏定义只会被展开一次。如 `f(1)` 会被展开为 `f(2)`，但是 `f(2)` 并不会再次被展开，因为它就是被 `f(1)` 展开过的。

因此这也解释了为什么使用 ## 运算符时，嵌套 `CONCAT` 就会报错而新定义 `CONCAT2` 调用 `CONCAT` 再嵌套就不会报错。我们来看一下它是如何展开的：

```cpp
#define CONCAT(x, y) x ## y
#define CONCAT2(x, y) CONCAT(x,y)

// 下面展示了 CONCAT2(a,CONCAT2(b,c)) 在预处理时被展开的过程
CONCAT2(a,CONCAT2(b,c))
CONCAT2(a,CONCAT(b,c))
CONCAT2(a, b ## c) => CONCAT2(a, bc)
CONCAT(a, bc)
a ## bc => abc
```

### 2.5. #undef 指令 🔥

该指令用于取消之前使用 `#define` 定义的宏。如果想使用一个名称但是又不确定之前是否用过，为了安全起见可以先使用 `#undef` 取消定义然后再定义。

### 2.6. 预定义宏

C99 开始引入一个特殊标识符 `__func__`，该标识符是一个变量名而非宏定义，因此不属于宏定义的范畴，但是它的作用和 `__FILE__, __LINE__` 类似。

```cpp
#include <stdio.h>

void func() {
    printf("This function is %s\n", __func__);
    printf("This is line %d\n", __LINE__);
}

int main() {
    printf("This file is %s\n", __FILE__);
    printf("This date is %s\n", __DATE__);
    printf("This time is %s\n", __TIME__);
    printf("This function is %s\n", __func__);
    printf("This is line %d\n", __LINE__);
    func();
    return 0;
}
```

### 2.7. 泛型选择

C11 中新增了一种表达式，称为 *泛型选择表达式* ，可根据表达式的类型选择一个值。

`_Generic` 是 C11 的关键字，第一项是一个表达式，后面每项都由一个类型、一个冒号和一个值组成，如 `_Generic(x, int: 0, float: 1, double: 2, default3)` 。当然，后面的值也可以是字符串：

```cpp
#include <stdio.h>

#define TYPE(x) _Generic((x), int: "data is int", float: "data is float", double: "data is double", default: "other")

int main() {
		printf("%s\n", TYPE(1));
    printf("%s\n", TYPE(1.0f));
    printf("%s\n", TYPE(1.0));
    printf("%s\n", TYPE(1l));
    return 0;
}
```