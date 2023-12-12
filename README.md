# Welcome to GenshinOS

Hello traveller!

## loader.s

```asm
.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)
// 3个grub启动需要的变量

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM


.section .text
.extern kernelMain
.global loader

loader:
    mov $kernel_stack, %esp

    call callConstructors

    push %eax
    push %ebx
    call kernelMain

_stop:
    cli
    hlt
    jmp _stop

.section .bss
.space 2*1024*1024 # 2 MiB
kernel_stack:
```

C++不允许修改栈顶指针，因此借助一个汇编文件进行帮助。同时，操作系统启动时，Linux下0号扇区的引导程序是grub，需要遵循grub对于操作系统内核的一些约定（多引导协议头），他才会识别这是一个内核程序而不是别的用户程序，同样这个行为发生在我们调用kernel.cpp之前，也需要借助汇编的力量（还需要连接脚本的力量）。

[grub引导程序介绍](https://zhuanlan.zhihu.com/p/626225954)

chat一下：

这段代码是一个汇编语言程序，主要用于设置和启动一个基于multiboot标准的操作系统内核。它使用了一些特定的汇编指令和约定。让我来逐行解释它：

1. `.set MAGIC, 0x1badb002`：
   - 这行定义了一个名为 `MAGIC` 的常量，其值为 `0x1badb002`。这是multiboot标准要求的魔数（magic number），用于识别multiboot头部。

2. `.set FLAGS, (1<<0 | 1<<1)`：
   - 定义 `FLAGS` 常量。这里使用位操作将第0位和第1位设为1。这些标志通知multiboot引导程序提供哪些额外信息。

3. `.set CHECKSUM, -(MAGIC + FLAGS)`：
   - 定义 `CHECKSUM` 常量，其值为 `-(MAGIC + FLAGS)`。这个校验和用于确保multiboot头部的总和为0。

4. `.section .multiboot`：
   - 指定接下来的代码属于 `.multiboot` 段，这是用于存放multiboot头部的。

5. `.long MAGIC`、`.long FLAGS`、`.long CHECKSUM`：
   - 将 `MAGIC`、`FLAGS` 和 `CHECKSUM` 的值放入 `.multiboot` 段。这构成了multiboot头部。

6. `.section .text`：
   - 开始定义 `.text` 段，这通常包含程序的执行代码。

7. `.extern kernelMain`、`.global loader`：
   - `kernelMain` 被声明为外部函数（在其他地方定义）。`loader` 被声明为全局符号，这样其他文件也能引用它。

8. `loader:`：
   - 定义 `loader` 标签，这是程序的入口点。

9. `mov $kernel_stack, %esp`：
   - 将 `kernel_stack` 的地址移动到栈指针寄存器 `%esp`。这设置了程序的栈。

10. `call callConstructors`：
    - 调用 `callConstructors` 函数，这通常用于初始化C++环境（如调用静态对象的构造函数）。

11. `push %eax`、`push %ebx`：
    - 将寄存器 `%eax` 和 `%ebx` 的值压栈。这些值可能由multiboot引导程序提供，包含有用的启动信息。

12. `call kernelMain`：
    - 调用 `kernelMain` 函数，这是内核的主要入口点。

13. `_stop:`、`cli`、`hlt`、`jmp _stop`：
    - `_stop` 标签标记了一个无限循环，用于在 `kernelMain` 返回后停止执行。`cli` 指令清除中断，`hlt` 指令暂停处理器，`jmp _stop` 导致无限循环。

14. `.section .bss`、`.space 2*1024*1024`：
    - 定义 `.bss` 段，并在其中分配2MiB的空间。`.bss` 段用于存储未初始化的全局变量。

15. `kernel_stack:`：
    - 定义 `kernel_stack` 标签，这是之前设置栈指针时使用的地址。

这段代码主要用于在引导过程中设置一个适合启动操作系统内核的环境。它遵循multiboot规范，确保内核能够被multiboot兼容的引导加载器（如GRUB）正确加载。

## linker.ld

```ld

ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    . = 0x0100000; // multiboot header要求装入内存的地址要高于1M

    .text :
    {
        *(.multiboot)
        *(.tex)
    }

    .data :
    {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.*)));
        end_ctors = .;

        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

    /DISCARD/ :
    {
        *( .fini_array* )
        *( .comment )
    }

}
```

连接脚本，将cpp代码和asm代码连接在一起形成一个可执行文件。建议使用反编译看看可执行文件的汇编代码，很有意思，可以看到如何完成multiboot header的工作让grub认为这是一个os内核，以及Cpp支持重载的底层原理。

```BASH
objdump -d your_binary_file
```

具体是怎么链接的，我帮你问了chatgpt：

这段代码是一个链接器脚本（Linker Script），它用于控制程序的内存布局。这个特定的脚本看起来是为了创建一个适用于32位x86架构的ELF（Executable and Linkable Format）文件，通常用于Linux或类Unix系统。我会逐行解释这段脚本：

1. `ENTRY(loader)`：
   - 指定程序的入口点为 `loader` 函数。这意味着当程序启动时，控制权将首先传递到 `loader` 函数。

2. `OUTPUT_FORMAT(elf32-i386)`：
   - 设置输出文件格式为 `elf32-i386`，这是32位x86体系结构上ELF文件的格式。

3. `OUTPUT_ARCH(i386:i386)`：
   - 指定输出文件的目标架构为32位的i386。

4. `SECTIONS`：
   - 开始定义各个内存区段（Section）的布局。

5. `. = 0x0100000`：
   - 设置当前位置计数器（`.`）的值为 `0x0100000`（1MB）。这是内存中的地址，意味着接下来定义的区段将从这个地址开始放置。

6. `.text :`：
   - 定义 `.text` 区段。这个区段通常包含程序的执行代码。

7. `{ *(.multiboot) *(.text) }`：
   - 将 `.multiboot` 和 `.text` 段中的内容合并到 `.text` 区段。`.multiboot` 是multiboot标头所在的段，这对于启动基于multiboot的内核（如GRUB引导的Linux内核）是必需的。

8. `.data :`：
   - 定义 `.data` 区段。这个区段通常包含程序的已初始化数据。

9. `{ start_ctors = .; ... end_ctors = .; *(.data) }`：
   - 这里定义了构造函数（ctors）的开始和结束位置，并将 `.data` 段的内容放入 `.data` 区段。

10. `.bss :`：
    - 定义 `.bss` 区段。这个区段用于存储程序的未初始化数据。

11. `{ *(.bss) }`：
    - 将 `.bss` 段的内容放入 `.bss` 区段。

12. `/DISCARD/ :`：
    - 定义一个丢弃区段。在这里，某些标记的段将不会被包含在最终的二进制文件中。

13. `{ *( .fini_array* ) *( .comment ) }`：
    - 指定 `.fini_array*` 和 `.comment` 段中的内容不被包括在最终的输出中。`.fini_array` 通常包含析构函数，而 `.comment` 包含编译器的注释信息。

这个链接器脚本的主要目的是为特定的硬件和引导程序配置程序的内存布局。通过精确控制哪些数据和代码位于内存的哪个部分，它可以确保程序能够正确地被操作系统或引导加载程序加载和执行。

Makefile看不懂参数直接chat，目前只加入了禁止使用编译环境相关的参数。
