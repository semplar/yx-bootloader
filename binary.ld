MEMORY {
	RAM (xrw)     : ORIGIN = 0x20000000, LENGTH = 20K
	FLASH (rx)    : ORIGIN = 0x08000000, LENGTH = 64K
}

SECTIONS {
	.isr_flash : ALIGN(4) {
		KEEP(*(.isr_vector_flash))
    } >FLASH

	.text : ALIGN(4) {
		*(.text*)
		*(.glue_7)
		*(.glue_7t)
		KEEP(*(.init))
		KEEP(*(.fini))
    } >FLASH

	.rodata : ALIGN(4) {
		*(.rodata*)
	} >FLASH

	.stack (NOLOAD) : ALIGN(4) {
		. = . + 0x400;
		_estack = . ;
	} >RAM

	.isr_ram (NOLOAD) : ALIGN(512) {
		KEEP(*(.isr_vector_ram))
	} >RAM

	.data : ALIGN(4) {
		_sdata = . ;
		*(.data*)
		_edata = . ;
    } >RAM AT >FLASH

	_ldata = LOADADDR(.data);

	.bss (NOLOAD) : ALIGN(4) {
		_sbss = . ;
		*(.bss*)
		*(COMMON)
		_ebss = . ;
    } >RAM

	/DISCARD/ : {
		*(.note*)
		*(.comment*)
		*(.ARM.exidx)
		*(.ARM.extab)
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
	}

	# debug
	.symtab        0 : { *(.symtab) }
	.strtab        0 : { *(.strtab) }
	.shstrtab      0 : { *(.shstrtab) }
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }

	# DWARF 1
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	# GNU DWARF 1 extensions
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	# DWARF 1.1 and DWARF 2
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	# DWARF 2
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	# SGI/MIPS DWARF 2 extensions
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}
