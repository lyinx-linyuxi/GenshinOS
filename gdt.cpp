#include "gdt.h"

/*
    GDT的结构：
    0x00: 段界限，低16位
    0x08: 段基址，低16位
    0x10: 段基址，中8位
    0x18: 段属性，包括段界限的高4位
    0x20: 段界限，高4位
    0x28: 段基址，高8位
    0x30: 未使用
    可以参考一篇博客：https://blog.csdn.net/abc123lzf/article/details/109289567
*/
GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0, 0, 0),
      unusedSegmentSelector(0, 0, 0),
      codeSegmentSelector(0, 64 * 1024 * 1024, 0x9A),
      dataSegmentSelector(0, 64 * 1024 * 1024, 0x92)
{

    uint32_t i[2];
    i[0] = (uint32_t)this;
    i[1] = sizeof(GlobalDescriptorTable) << 16;

    //to tell processor to use the GDT
    asm volatile("lgdt (%0)" : : "p"(((uint8_t *)i) + 2)); 
    // 内联汇编，可以参考https://zhuanlan.zhihu.com/p/69670366?utm_id=0
    // volatile表示不进行优化
}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}

uint16_t GlobalDescriptorTable::DataSegmentSelector()
{
    // 返回数据段的选择子
    return (uint8_t *)&dataSegmentSelector - (uint8_t *)this;
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector()
{
    // 返回代码段的选择子
    return (uint8_t *)&codeSegmentSelector - (uint8_t *)this;
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t flags)
{
    uint8_t *target = (uint8_t *)this;

    // 判断limit的大小，如果超过65536，就需要使用分大小4GB的分页，否则使用512KB大小的分页。
    if (limit <= 65536)
    {
        target[6] = 0x40;
    }
    else
    {
        if ((limit & 0xfff) != 0xfff)
        {
            limit = (limit >> 12) - 1;
        }
        else
        {
            limit = limit >> 12;
        }
        target[6] = 0xc0;
    }

    target[0] = limit & 0xff;
    target[1] = (limit >> 8) & 0xff;
    target[6] |= (limit >> 16) & 0xf;

    target[2] = base & 0xff;
    target[3] = (base >> 8) & 0xff;
    target[4] = (base >> 16) & 0xff;
    target[7] = (base >> 24) & 0xff;

    target[5] = flags;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Base()
{
    // 返回段基址
    uint8_t *target = (uint8_t *)this;
    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];
    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit()
{
    // 返回段界限
    uint8_t *target = (uint8_t *)this;
    uint32_t result = target[6] & 0x0f;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    if ((target[6] & 0xC0) == 0xC0)
    {
        result = (result << 12) | 0xfff;
    }

    return result;
}