/*
 * This is the NXP LPC1768 linker file for code running from flash.
 *
 * TODO: 
 * - handle the exotic input sections (e.g. glue and veneer, C++ sections)
 * - add additional Ethernet and USB RAM memory regions (2x16k)
 * - add boot ROM memory regions
 *
 * See also: http://bitbucket.org/jpc/lpc1768/
 *
 * Copyright (c) 2010 LoEE - Jakub Piotr Cłapa
 * This program is released under the new BSD license.
 */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(Reset_Handler)

MEMORY {
  flash (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
    /* Local static ram, 32K */
  ram0   (rwx) : ORIGIN = 0x10000000, LENGTH =  32K
    /* AHB SRAM, banks 0 and 1 */
  ahb0   (rwx) : ORIGIN = 0x2007C000, LENGTH = 16K
  ahb1   (rwx) : ORIGIN = 0x20080000, LENGTH = 16K
}

SECTIONS {
  . = 0;

  .text : {
    _stext = .;
    KEEP(*(.cs3.interrupt_vector))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
    _etext = .;
  } > flash

  .data : {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > ram0 AT > flash

  .bss : {
    _sbss = .;
    *(.bss*)
    . = ALIGN(4);
    _ebss = .;
  } > ram0
 
/*    .stackarea (NOLOAD) :
    {
        . = ALIGN(8);
        _sstack = .;

        *(.stackarea .stackarea.*)

        . = ALIGN(8);
        _estack = .;

        . = ALIGN(4);
        _end = . ;
        PROVIDE (end = .);
    } > ram0 */

 
  _sstack = ORIGIN(ram0) + LENGTH(ram0);
}
