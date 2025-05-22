MEMORY
{
    RAM   (RX)  :  ORIGIN = 0xA0000000, LENGTH = 0x1FBFFFFF
    FLASH (RWX) :  ORIGIN = 0xBFC00000, LENGTH = 0x003FFFFF
}

SECTIONS {
    .intvecs (0xBFC00000): AT (0xBFC00000) {
        KEEP(*(.intvecs))
    }> FLASH

    .text : {
        KEEP(*(.text.Reset_Handler))
        KEEP(*(.text))
        *(.text.*)
        . = ALIGN(4);
        KEEP(*(.ctors))
        . = ALIGN(4);
        KEEP(*(.dtors))
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(.init_array*));
        __init_array_end = .;
        KEEP(*(.init))
        KEEP(*(.fini*))
    }> FLASH AT> FLASH 

    .rodata : {
        *(.rodata);
        *(.rodata.*)
    }>FLASH AT> FLASH

    __etext = .;

    .data : {
        __data_load__ = LOADADDR(.data);
        __data_start__ = .;
        KEEP(*(.data));
        KEEP(*(.data.*));
        . = ALIGN(4);
        __data_end__ = .;
    }> RAM AT> FLASH

    .bss : {
        __bss_start__ = .;
        *(.shbss)
        KEEP(*(.bss))
        *(.bss.*)
        *(COMMON)
        *(.sbss)
        . = ALIGN(4);
        __bss_end__ = .;
    }> RAM AT> RAM

    .heap : {
        __heap_start__ = .;
        end = __heap_start__;
        _end = end;
        __end = end;
        KEEP(*(.heap));
        __heap_end__ = .;
        __HeapLimit = __heap_end__;
    }> RAM AT> RAM

    .stack (NOLOAD) : ALIGN(0x8) {
        _stack = .;
        KEEP(*(.stack));
    }> RAM AT> RAM

    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    PROVIDE(__stack = __StackTop);
}
