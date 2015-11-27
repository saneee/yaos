/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

/* OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386") */
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

mboot_load_addr = 0x00100000;

SECTIONS
{
	/* Link the kernel at this address: "." means the current address */
        /* Must be equal to KERNLINK */
	. = 0x100000; 

	PROVIDE(begin = .);

	.text : AT(mboot_load_addr) {
		*(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
	}

	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	/* Adjust the address for the data segment to the next page */
	. = ALIGN(0x1000);

	/* Conventionally, Unix linkers provide pseudo-symbols
	 * etext, edata, and end, at the end of the text, data, and bss.
	 * For the kernel mapping, we need the address at the beginning
	 * of the data section, but that's not one of the conventional
	 * symbols, because the convention started before there was a
	 * read-only rodata section between text and data. */
	PROVIDE(data = .);

	/* The data segment */
	.data : {
		*(.data)
                *(.ref.data)
	}
        . = ALIGN((1<<12));
        .init.begin : {
        __init_begin = .;
        }
 __per_cpu_load = .; 
.data..percpu  :  { __per_cpu_start = .; *(.data..percpu..first) . = ALIGN((1 << 12)); *(.data..percpu..page_aligned) . = ALIGN((1 << 6)); *(.data..percpu..read_mostly) . = ALIGN((1 << 6)); *(.data..percpu) *(.data..percpu..shared_aligned) __per_cpu_end = .; } 
 . = ALIGN((1 << 12)); .init.text : { _sinittext = .; *(.init.text) _einittext = .; }
.init.data : {
*(.init.data)
 . = ALIGN(8); __start_mcount_loc = .; *(__mcount_loc) __stop_mcount_loc = .; *(.init.rodata) 
. = ALIGN(8); __start_ftrace_events = .;
 *(_ftrace_events)
 __stop_ftrace_events = .; __start_ftrace_enum_maps = .; 
*(_ftrace_enum_map) __stop_ftrace_enum_maps = .;
 . = ALIGN(8); __start_syscalls_metadata = .; *(__syscalls_metadata) __stop_syscalls_metadata = .; 
. = ALIGN(8); __start_kprobe_blacklist = .; *(_kprobe_blacklist) __stop_kprobe_blacklist = .; 
. = ALIGN(8); __clk_of_table = .; *(__clk_of_table) *(__clk_of_table_end) . = ALIGN(8); __cpu_method_of_table = .; *(__cpu_method_of_table) *(__cpu_method_of_table_end) 
. = ALIGN(8); __cpuidle_method_of_table = .; *(__cpuidle_method_of_table) *(__cpuidle_method_of_table_end) 
. = ALIGN(32); __dtb_start = .; *(.dtb.init.rodata) __dtb_end = .; 
. = ALIGN(32); __earlycon_table = .; *(__earlycon_table) *(__earlycon_table_end) 
. = ALIGN(8); __earlycon_of_table = .; *(__earlycon_of_table) *(__earlycon_of_table_end) 
. = ALIGN(16); __setup_start = .; *(.init.setup) __setup_end = .; 
__initcall_start = .; *(.initcallearly.init) 
__initcall0_start = .; *(.initcall0.init) *(.initcall0s.init) 
__initcall1_start = .; *(.initcall1.init) *(.initcall1s.init) 
__initcall2_start = .; *(.initcall2.init) *(.initcall2s.init) 
__initcall3_start = .; *(.initcall3.init) *(.initcall3s.init) 
__initcall4_start = .; *(.initcall4.init) *(.initcall4s.init) 
__initcall5_start = .; *(.initcall5.init) *(.initcall5s.init) 
__initcallrootfs_start = .; *(.initcallrootfs.init) *(.initcallrootfss.init) 
__initcall6_start = .; *(.initcall6.init) *(.initcall6s.init) 
__initcall7_start = .; *(.initcall7.init) *(.initcall7s.init) 
__initcall_end = .; 
__con_initcall_start = .; *(.con_initcall.init) __con_initcall_end = .; __security_initcall_start = .; *(.security_initcall.init) __security_initcall_end = .; 
. = ALIGN(4); __initramfs_start = .; *(.init.ramfs) . = ALIGN(8); *(.init.ramfs.info) }

        . = ALIGN(0x80);

        .module_data : {
                _module_data_start = .;
                *(.module_data)
                 _module_data_end = .;
        }

        . = ALIGN(0x80);

        .yaoscall_data : {
                _yaoscall_data_start = .;
                *(.yaoscall_data)
                 _yaoscall_data_end = .;
        }
 . = ALIGN(8);
 .altinstructions :  {
  _alt_instructions_start = .;
  *(.altinstructions)
  _alt_instructions_end = .;
 }
.altinstr_replacement : {
  *(.altinstr_replacement)
 }


 
 . = ALIGN((1 << 12));
 /* freed after init ends here */
 .init.end :  {
  __init_end = .;
 }
/* BSS */

 . = ALIGN((1 << 12));
PROVIDE(edata = .);

 .bss :  {
  __bss_start = .;
  *(.bss..page_aligned)
  *(.bss)
  . = ALIGN((1 << 12));
  __bss_stop = .;
 }
 PROVIDE(end = .);

	/DISCARD/ : {
		*(.eh_frame .rela.eh_frame .note.GNU-stack)
	}
}
mboot_load_end = mboot_load_addr + (edata - begin);
mboot_bss_end = mboot_load_addr + (end - begin);
mboot_entry_addr = mboot_load_addr + (mboot_entry - begin);
