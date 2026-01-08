OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

PROVIDE (__scratch_start = 0x00000080);
PROVIDE (__scratch_end = 0x00010000);
PROVIDE (__scratch2_start = 0xb0020000);
PROVIDE (__scratch2_end = 0xb0028000);
PROVIDE (__scratch3_start = 0x7e000000);
PROVIDE (__scratch3_end = 0x7e700000);

__arom_patch_start = 0xd100e508;
__arom_patch_end = 0xd1010000;

__btcm_user_start = __scratch2_start + __btcm_user_offset;
__btcm_user_end = __scratch2_end;

SECTIONS
{
    . = __scratch_start;

    .text : {
        __code_start = .;
        KEEP(*(.text.boot))
        *(.text)
        *(SORT(.text.*))
        *(.gnu.linkonce.t.*)
        __code_end = .;
    }

    .rodata : ALIGN(4) {
        __rodata_start = .;
        *(.rodata)
        *(SORT(.rodata.*))
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
    }

    /*
     * extra linker scripts tend to insert sections just after .rodata,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_rodata : {
        __rodata_end = .;
    }

    .data : ALIGN(4) {
        __data_start = .;
        *(.data)
        *(SORT(.data.*))
        *(.gnu.linkonce.d.*)
    }

    .ctors : ALIGN(4) {
        __ctor_list = .;
        KEEP(*(.ctors .init_array))
        __ctor_end = .;
    }

    .dtors : ALIGN(4) {
        __dtor_list = .;
        KEEP(*(.dtors .fini_array))
        __dtor_end = .;
    }

    /*
     * extra linker scripts tend to insert sections just after .data,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_data : {
        __data_end = .;
    }

    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss)
        *(SORT(.bss.*))
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    . = __btcm_user_start;

    /* heap section */
    __heap_start = . ;
    __heap_end = __btcm_user_end - __stack_size;
    ASSERT(__heap_end > __heap_start + __min_heap_size, "error: heap size too small")

    . = __heap_end;

    __stack_start = .;
    __stack_end = __stack_start + __stack_size;
    _end = .; PROVIDE (end = .);

    . = __arom_patch_start;
    .patch : AT (__bss_start) {
        __patch_start = .;
        *(.patch)
        *(SORT(.patch.*))
        *(.transfer_param_section)
        . = ALIGN(4);
    }
    __patch_load_addr = LOADADDR(.patch);
    __patch_size = SIZEOF(.patch);
    ASSERT(__arom_patch_end > __arom_patch_start + __patch_size, "error: patch code size exceeded the limit")

    /DISCARD/ : { *(.comment .note.* .eh_frame) }
}
