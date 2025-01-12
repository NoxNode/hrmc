##!as -c -o minelf.o minelf.s && ld -o minelf.bin minelf.o && objcopy -j .text minelf.bin minelf.exe && chmod +x minelf.exe && ./minelf.exe; echo $?
##!xxd minelf.exe
##!./minelf.exe
##!objdump -d -j .text minelf.bin
##!dd ibs=1 obs=1 skip=120 count=400 if=minelf.bin of=minelf.exe
##!readelf -a minelf.exe
##!man dd
//  88:	d2800000 	mov	x0, #0x0                   	// #0
//  8c:	d2800ba8 	mov	x8, #0x5d                  	// #93
//  90:	d4000001 	svc	#0x0
ehdr:                     // ELF header
	.byte  0x7F           // e_ident[EI_MAG0]
	.ascii "ELF"          // e_ident[EI_MAG1-3]
	.byte  2              // e_ident[EI_CLASS] (1=32-bit, 2=64-bit)
	.byte  1              // e_ident[EI_DATA] (1=little endian, 2=big endian)
	.byte  1              // e_ident[EI_VERSION] (ELF version)
	.byte  0              // e_ident[EI_OSABI] (0=System V ABI)
	.quad  0              // e_ident[EI_ABIVERISON], next 7 bytes are reserved
	.short 3              // e_type (1=relocatable, 2=executable, 3=shared obj)
	.short 0xB7           // e_machine (0x03=x86, 0x3e=x64, 0xF3=RISC V, 0x28=ARM, 0xB7=ARM64)
	.long  1              // e_version (ELF verion)
	.quad  _start         // e_entry (addr of start of entry point)
	.quad  phdr - ehdr    // e_phoff (addr of start of program header)
	.quad  shdr - ehdr    // e_shoff (addr of start of section header)
	.long  0              // e_flags (depends on architecture)
	.short ehdrsize       // e_ehsize (sizeof elf header)
	.short phdrsize       // e_phentsize (sizeof program header table entry)
	.short 4              // e_phnum (num entries in program header table)
	.short shdrsize       // e_shentsize (sizeof section header table entry)
	.short 3              // e_shnum (num entries in section header table)
	.short 1              // e_shstrndx (index of sections names entry in section header table)

.equ ehdrsize, . - ehdr

phdr:                     // Program header
	.long  1              // p_type (1=loadble segement)
	.long  5              // p_flags (1=executable, 2=writable, 4=readable)
	.quad  0              // p_offset (offset of segment in file image)
	.quad  ehdr           // p_vaddr (virtual address of segment in memory)
	.quad  ehdr           // p_paddr (physical address " " if on system where relevant)
	.quad  filesize       // p_filesz (size of segment in file image)
	.quad  filesize       // p_memsz (size of segment in memory)
	.quad  0x200000       // p_align (p_vaddr == p_offset % p_align)

.equ phdrsize, . - phdr

shdr:
	.long 1               // sh_name
	.long 1               // sh_type (1=PROGBITS)
	.quad 4               // sh_flags
	.quad shdr            // sh_addr
	.quad shdr - ehdr     // sh_offset
	.quad shdrsize        // sh_size
	.long 0               // sh_link
	.long 0               // sh_info
	.quad 0x0001          // sh_addralign
	.quad shdrsize        // sh_entsize
.equ shdrsize, . - shdr
	.long 1               // sh_name
	.long 1               // sh_type (1=PROGBITS)
	.quad 4               // sh_flags
	.quad shdr            // sh_addr
	.quad shdr - ehdr     // sh_offset
	.quad shdrsize        // sh_size
	.long 0               // sh_link
	.long 0               // sh_info
	.quad 0x0001          // sh_addralign
	.quad shdrsize        // sh_entsize

// your program here
_start:
	mov x0, #0
	mov x8, #93
	svc #0
  
.equ filesize, . - ehdr
