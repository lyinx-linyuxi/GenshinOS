#include "gdt.h"
#include "types.h"

void printf(char *str)
{
    // 显存位置在0xb8000
    // 之后的内存每两个字节为一组
    // 低字节为前景背景色设置，初始已经设置好黑底白字
    // 高字节为待显示字符的ASCII码
    static uint16_t *VideoMemory = (unsigned short *)0xb8000;

    static uint8_t x = 0, y = 0;
    // 屏幕大小为25*80字，每行80个字符，可以显示25行
    // 需要手动进行换行
    // x表示列，y表示行
    for (int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i]){ 
            case '\n':  // 检测换行符
                y++;
                x=0;
                break;
            default:
                VideoMemory[80 * y + x] = (VideoMemory[i] & 0xff00) | str[i];
                x++;
                break;
        }

        // 每行80个字符，超过80个字符需要换行
        if (x >= 80)
        {
            y++;
            x = 0;
        }

        // 超过25行需要清屏
        if (y >= 25)
        {
            for (y = 0; y < 25; y++)
            {
                for (x = 0; x < 80; x++)
                {
                    VideoMemory[80 * y + x] = (VideoMemory[i] & 0xff00) | ' '; // ' '表示空格显示空字符。
                }
            }
            x = 0;
            y = 0;
        }
    }
}

typedef void (*constructor)();
// external “C”表示按照C语言的方式进行编译，不进行name mangling
// name mangling是编译器对函数名进行改变，以便支持函数重载
// 可以看反编译后的结果，对于C++这种重载的特性，编译器会对函数名进行改变，以便区分不同的函数
// 构造函数的起始地址和结束地址，这两个变量在linker.ld中定义，extern表示在其他文件中定义
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    // 遍历构造函数数组，执行构造函数，完成全局对象的初始化，否则全局对象的构造函数不会被执行。
    // 对于C++类的全局对象的构造（实际上是一个匿名函数调用了构造函数），编译器会将构造函数的地址放在一个数组中，需要手动调用函数进行构造。
    for (constructor *i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(void *multiboot_structure, uint32_t magicnumber)
{

    printf("I thik it should be a new version");
    printf("this is another line");

    GlobalDescriptorTable gdt; // 初始化全局描述符表

    while (1)
        ;
}
