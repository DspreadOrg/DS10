OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

__scratch_start = 0x00000080;
__scratch_end = 0x00010000;
__scratch2_start = 0xb0020000;
__scratch2_end = 0xb0028000;

#define AROM_HAS_SRAM    (AROM_VERSION == 20200730 || \
                          AROM_VERSION == 20211016 || \
                          AROM_VERSION == 20221106 || \
                          AROM_VERSION == 20240430 || \
                          AROM_VERSION == 20221111)

#if AROM_VERSION == 20200730        /* CraneG/CraneM */
__arom_patch_start = 0xd100e508;
__arom_patch_end = 0xd1010000;
__aram_ctext_start = 0xd1f10000;
#elif AROM_VERSION == 20211016      /* CraneL */
__aram_ctext_start = 0xd1f10000;
#elif AROM_VERSION == 20221106      /* CraneL(X) */
__aram_ctext_start = 0xf0000000;
#elif AROM_VERSION == 20221111      /* CraneGT */
__aram_ctext_start = 0xd1f14000;
#elif AROM_VERSION == 20240430      /* Lapwing */
__aram_ctext_start = 0xa4050000;
#endif

#if AROM_HAS_SRAM
__aram_ctext_size = 0x8000;
#endif

#if AROM_VERSION == 20221111        /* CraneGT */
__ddr_base = 0x78000000;
__ddr_max_size = 0x08000000; /* 128MB, get real ddr size depends on efuse ddr type */
PROVIDE (__scratch3_start = __ddr_base);
PROVIDE (__scratch3_end = __ddr_base + 0x00800000); /* DDR 8M */
#elif AROM_VERSION == 20240430        /* Lapwing */
__ddr_base = 0x07800000;
__ddr_max_size = 0x00800000; /* Only 8MB(0x07800000 ~ 0x08000000) is used by Lapwing boot2 and flasher. */
PROVIDE (__scratch3_start = __ddr_base);
PROVIDE (__scratch3_end = __ddr_base + 8M); /* DDR 8MB */
#elif AROM_VERSION == 20230909      /* Fulmar CR5 */
/*
 * Fulmar cr5 DDR memory configuraton (Min size is 32MB):
 *   firmware area : 0 up to 28MB
 *   reserved      : the remainder size
 *   scratch3      : 1MB
 *   dsp.bin       : 3MB
 */
__ddr_base = 0x70000000;
__ddr_max_size = 0x08000000; /* 128MB, get real ddr size depends on efuse ddr type */
PROVIDE (__scratch3_start = __ddr_base + 0x01B80000); /* use dsp ir buffer base addr */
PROVIDE (__scratch3_end = __scratch3_start + 0x00100000);
#else
__psram_base = 0x7E000000;
__psram_max_size = 0x02000000; /* 32MB, get real psram size depends on efuse(pram has phy) or auto detect(psram no phy) */
PROVIDE (__scratch3_start = __psram_base);
PROVIDE (__scratch3_end = __psram_base + 0x00800000); /* psram 8M */
#endif

__btcm_user_start = __scratch2_start + __btcm_user_offset;
__btcm_user_end = __scratch2_end;
__lzma_decode_start = __scratch3_start;
__lzma_decode_size = (__scratch3_end - __scratch3_start) / 4;
__lzma_alloc_base = __scratch2_start;
__lzma_alloc_size = __btcm_user_offset;

MEMORY{
    atcm : ORIGIN = __scratch_start, LENGTH = __scratch_end - __scratch_start + 64K
#if AROM_HAS_SRAM
    sram : ORIGIN = __aram_ctext_start, LENGTH = __aram_ctext_size
#endif
#if AROM_VERSION == 20200730
    patch : ORIGIN = __arom_patch_start, LENGTH = __arom_patch_end - __arom_patch_start
#endif
}

SECTIONS
{
    .boot : {
        __code_start = .;
        *startup.o(.text .text.*)
        . = ALIGN(64);
    } > atcm

    .text : {
        KEEP(*(.ro.loadtable))
        KEEP(*(.init))
        *(.text.*syscall_register)
        *lzma.o(.text .text.*)
        *LzmaDec.o(.text .text.*)
        *syscall.o(.text .text.*)
        *decompress.o(.text .text.*)
        *cinit.o(.text .text.*)
#ifdef MODULE_LOADTABLE
        *loadtable.o(.text .text.*)
#endif
        . = ALIGN(4);
        __code_end = .;
    } > atcm

    .ctext : ALIGN(4) {
        _ctext = .;
        *(.text)
        *(SORT(.text.*))
        *(.gnu.linkonce.t.*)
        . = ALIGN(4);
    } > atcm
    . = ALIGN(4);
    _ctext_load_addr = LOADADDR(.ctext);
    _ctext_offset = _ctext_load_addr - _start;
    _ctext_size = SIZEOF(.ctext);

    .rodata : ALIGN(4) {
        _rodata = .;
        *(.rodata)
        *(SORT(.rodata.*))
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        __ctor_list = .;
        KEEP(*(.ctors .init_array))
        __ctor_end = .;
        __dtor_list = .;
        KEEP(*(.dtors .fini_array))
        __dtor_end = .;
        . = ALIGN(4);
#if AROM_HAS_SRAM && defined(APPLICATION_FLASHER)
    } > sram AT > atcm
#else
    } > atcm
#endif
    . = ALIGN(4);
    _rodata_loadaddr = LOADADDR(.rodata);
    _rodata_offset = _rodata_loadaddr - _start;
    _rodata_size = SIZEOF(.rodata);

    .data : ALIGN(4) {
        _data = .;
        *(.data)
        *(SORT(.data.*))
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
#if AROM_HAS_SRAM && defined(APPLICATION_FLASHER)
    } > sram AT > atcm
#else
    } > atcm
#endif
    . = ALIGN(4);
    _data_load_addr = LOADADDR(.data);
    _data_offset = _data_load_addr - _start;
    _data_size = SIZEOF(.data);
#if AROM_VERSION != 20200730
    _end = _data_load_addr + _data_size;
#endif

    .bss (NOLOAD) : ALIGN(4) {
        __bss_start = .;
        *(.bss)
        *(SORT(.bss.*))
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
#if AROM_HAS_SRAM && defined(APPLICATION_FLASHER)
    } > sram AT > atcm
#else
    } > atcm
#endif

    . = __btcm_user_start;

    /* heap section */
    __heap_start = . ;
    __heap_end = __btcm_user_end - __stack_size;
    ASSERT(__heap_end > __heap_start + __min_heap_size, "error: heap size too small")

    . = __heap_end;

    __stack_start = .;
    __stack_end = __stack_start + __stack_size;

#if AROM_VERSION == 20200730
    .patch : {
        _patch = .;
        *(.patch)
        *(SORT(.patch.*))
        . = ALIGN(4);
    } > patch AT > atcm
    _patch_load_addr = LOADADDR(.patch);
    _patch_offset = _patch_load_addr - _start;
    _patch_size = SIZEOF(.patch);
    _end = _patch_load_addr + _patch_size;
    ASSERT(__arom_patch_end > __arom_patch_start + _patch_size, "error: patch code size exceeded the limit")
#endif

    /DISCARD/ : { *(.comment .note.* .eh_frame .ARM.exidx* .ARM.extab*) }
}
