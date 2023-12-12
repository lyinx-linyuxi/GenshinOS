#ifndef __GDT_H
#define __GDT_H

#include "types.h"

class GlobalDescriptorTable
{
  public:
    class SegmentDescriptor
    {
      private:
        uint16_t limit_lo; // Lower 16 bits of the segment limit
        uint16_t base_lo; // Lower 16 bits of the segment base address
        uint8_t base_hi; // Next 8 bits of the segment base address
        uint8_t type; // Type of the segment
        uint8_t flags_limit_hi; // Flags and upper 4 bits of the segment limit
        uint8_t base_vhi; // Last 8 bits of the segment base address
      public:
        SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type); // Constructor for SegmentDescriptor
        uint32_t Base(); // Get the base address of the segment
        uint32_t Limit(); // Get the limit of the segment
    } __attribute__((packed));

    SegmentDescriptor nullSegmentSelector; // Null segment selector
    SegmentDescriptor unusedSegmentSelector; // Unused segment selector
    SegmentDescriptor codeSegmentSelector; // Code segment selector
    SegmentDescriptor dataSegmentSelector; // Data segment selector

  public:
    GlobalDescriptorTable(); // Constructor for GlobalDescriptorTable
    ~GlobalDescriptorTable(); // Destructor for GlobalDescriptorTable

    uint16_t CodeSegmentSelector(); // Get the code segment selector
    uint16_t DataSegmentSelector(); // Get the data segment selector
};



#endif