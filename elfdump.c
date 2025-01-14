// gcc -o elfdump.exe elfdump.c && ./elfdump.exe elfdump.c
// readelf -a b.out
// objdump -x b.out
// xxd -C b.out
#include <stdio.h>
#include <stdlib.h>

char elfb[] = {
0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x00, 0xb7, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf4, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00, 0x06, 0x00, 0x40, 0x00, 0x06, 0x00, 0x05, 0x00,
0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xa9, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa9, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
0xf4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xf4, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x50, 0xe5, 0x74, 0x64, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xac, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0xe5, 0x74, 0x64, 0x06, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x2f, 0x64, 0x61, 0x74, 0x61, 0x2f, 0x64, 0x61, 0x74, 0x61, 0x2f,
0x63, 0x6f, 0x6d, 0x2e, 0x74, 0x65, 0x72, 0x6d, 0x75, 0x78, 0x2f, 0x66, 0x69, 0x6c, 0x65, 0x73,
0x2f, 0x75, 0x73, 0x72, 0x2f, 0x6c, 0x69, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xff, 0x43, 0x00, 0xd1, 0xe0, 0x0f, 0x40, 0xb9, 0xff, 0x43, 0x00, 0x91,
0xc0, 0x03, 0x5f, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb, 0xff, 0xff, 0x6f, 0x00, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x50, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xfe, 0xff, 0x6f, 0x00, 0x00, 0x00, 0x00,
0x68, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74,
0x61, 0x62, 0x00, 0x2e, 0x67, 0x6e, 0x75, 0x2e, 0x68, 0x61, 0x73, 0x68, 0x00, 0x2e, 0x64, 0x79,
0x6e, 0x73, 0x74, 0x72, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x2e, 0x64, 0x79, 0x6e, 0x61,
0x6d, 0x69, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0xf6, 0xff, 0xff, 0x6f,
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x68, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xf4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xa8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

typedef          char      i8;
typedef          char      s8;
typedef unsigned char      u8;
typedef          short     i16;
typedef          short     s16;
typedef unsigned short     u16;
typedef          int       i32;
typedef          int       s32;
typedef unsigned int       u32;
typedef          long long i64;
typedef          long long s64;
typedef unsigned long long u64;
typedef          float     f32;
typedef          double    f64;

/* 32-bit ELF base types. */
typedef u32	Elf32_Addr;
typedef u16	Elf32_Half;
typedef u32	Elf32_Off;
typedef s32	Elf32_Sword;
typedef u32	Elf32_Word;

/* 64-bit ELF base types. */
typedef u64	Elf64_Addr;
typedef u16	Elf64_Half;
typedef s16	Elf64_SHalf;
typedef u64	Elf64_Off;
typedef s32	Elf64_Sword;
typedef u32	Elf64_Word;
typedef u64	Elf64_Xword;
typedef s64	Elf64_Sxword;

/* These constants are for the segment types stored in the image headers */
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7               /* Thread local storage segment */
#define PT_LOOS    0x60000000      /* OS-specific */
#define PT_HIOS    0x6fffffff      /* OS-specific */
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff
#define PT_GNU_EH_FRAME	(PT_LOOS + 0x474e550)
#define PT_GNU_STACK	(PT_LOOS + 0x474e551)
#define PT_GNU_RELRO	(PT_LOOS + 0x474e552)
#define PT_GNU_PROPERTY	(PT_LOOS + 0x474e553)

/* ARM MTE memory tag segment type */
#define PT_AARCH64_MEMTAG_MTE	(PT_LOPROC + 0x2)

/*
 * Extended Numbering
 *
 * If the real number of program header table entries is larger than
 * or equal to PN_XNUM(0xffff), it is set to sh_info field of the
 * section header at index 0, and PN_XNUM is set to e_phnum
 * field. Otherwise, the section header at index 0 is zero
 * initialized, if it exists.
 *
 * Specifications are available in:
 *
 * - Oracle: Linker and Libraries.
 *   Part No: 817–1984–19, August 2011.
 *   https://docs.oracle.com/cd/E18752_01/pdf/817-1984.pdf
 *
 * - System V ABI AMD64 Architecture Processor Supplement
 *   Draft Version 0.99.4,
 *   January 13, 2010.
 *   http://www.cs.washington.edu/education/courses/cse351/12wi/supp-docs/abi.pdf
 */
#define PN_XNUM 0xffff

/* These constants define the different elf file types */
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

/* This is the info that is needed to parse the dynamic section of the file */
#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH	15
#define DT_SYMBOLIC	16
#define DT_REL	        17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23
#define DT_ENCODING	32
#define OLD_DT_LOOS	0x60000000
#define DT_LOOS		0x6000000d
#define DT_HIOS		0x6ffff000
#define DT_VALRNGLO	0x6ffffd00
#define DT_VALRNGHI	0x6ffffdff
#define DT_ADDRRNGLO	0x6ffffe00
#define DT_ADDRRNGHI	0x6ffffeff
#define DT_VERSYM	0x6ffffff0
#define DT_RELACOUNT	0x6ffffff9
#define DT_RELCOUNT	0x6ffffffa
#define DT_FLAGS_1	0x6ffffffb
#define DT_VERDEF	0x6ffffffc
#define	DT_VERDEFNUM	0x6ffffffd
#define DT_VERNEED	0x6ffffffe
#define	DT_VERNEEDNUM	0x6fffffff
#define OLD_DT_HIOS     0x6fffffff
#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff

/* This info is needed when parsing the symbol table */
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

#define ELF_ST_BIND(x)		((x) >> 4)
#define ELF_ST_TYPE(x)		((x) & 0xf)

typedef struct {
  Elf32_Sword d_tag;
  union {
    Elf32_Sword	d_val;
    Elf32_Addr	d_ptr;
  } d_un;
} Elf32_Dyn;

typedef struct {
  Elf64_Sxword d_tag;		/* entry tag value */
  union {
    Elf64_Xword d_val;
    Elf64_Addr d_ptr;
  } d_un;
} Elf64_Dyn;

/* The following are used with relocations */
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x) & 0xff)

#define ELF64_R_SYM(i)			((i) >> 32)
#define ELF64_R_TYPE(i)			((i) & 0xffffffff)

typedef struct elf32_rel {
  Elf32_Addr	r_offset;
  Elf32_Word	r_info;
} Elf32_Rel;

typedef struct elf64_rel {
  Elf64_Addr r_offset;	/* Location at which to apply the action */
  Elf64_Xword r_info;	/* index and type of relocation */
} Elf64_Rel;

typedef struct elf32_rela {
  Elf32_Addr	r_offset;
  Elf32_Word	r_info;
  Elf32_Sword	r_addend;
} Elf32_Rela;

typedef struct elf64_rela {
  Elf64_Addr r_offset;	/* Location at which to apply the action */
  Elf64_Xword r_info;	/* index and type of relocation */
  Elf64_Sxword r_addend;	/* Constant addend used to compute value */
} Elf64_Rela;

typedef struct elf32_sym {
  Elf32_Word	st_name;
  Elf32_Addr	st_value;
  Elf32_Word	st_size;
  unsigned char	st_info;
  unsigned char	st_other;
  Elf32_Half	st_shndx;
} Elf32_Sym;

typedef struct elf64_sym {
  Elf64_Word st_name;		/* Symbol name, index in string tbl */
  unsigned char	st_info;	/* Type and binding attributes */
  unsigned char	st_other;	/* No defined meaning, 0 */
  Elf64_Half st_shndx;		/* Associated section index */
  Elf64_Addr st_value;		/* Value of the symbol */
  Elf64_Xword st_size;		/* Associated symbol size */
} Elf64_Sym;


#define EI_NIDENT	16

typedef struct elf32_hdr {
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;  /* Entry point */
  Elf32_Off	e_phoff;
  Elf32_Off	e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr {
  unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;		/* Entry point virtual address */
  Elf64_Off e_phoff;		/* Program header table file offset */
  Elf64_Off e_shoff;		/* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1

typedef struct elf32_phdr {
  Elf32_Word	p_type;
  Elf32_Off	p_offset;
  Elf32_Addr	p_vaddr;
  Elf32_Addr	p_paddr;
  Elf32_Word	p_filesz;
  Elf32_Word	p_memsz;
  Elf32_Word	p_flags;
  Elf32_Word	p_align;
} Elf32_Phdr;

typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;		/* Segment file offset */
  Elf64_Addr p_vaddr;		/* Segment virtual address */
  Elf64_Addr p_paddr;		/* Segment physical address */
  Elf64_Xword p_filesz;		/* Segment size in file */
  Elf64_Xword p_memsz;		/* Segment size in memory */
  Elf64_Xword p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

/* sh_type */
#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL		9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_NUM		12
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

/* sh_flags */
#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR		0x4
#define SHF_RELA_LIVEPATCH	0x00100000
#define SHF_RO_AFTER_INIT	0x00200000
#define SHF_MASKPROC		0xf0000000

/* special section indexes */
#define SHN_UNDEF	0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_LIVEPATCH	0xff20
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_HIRESERVE	0xffff
 
typedef struct elf32_shdr {
  Elf32_Word	sh_name;
  Elf32_Word	sh_type;
  Elf32_Word	sh_flags;
  Elf32_Addr	sh_addr;
  Elf32_Off	sh_offset;
  Elf32_Word	sh_size;
  Elf32_Word	sh_link;
  Elf32_Word	sh_info;
  Elf32_Word	sh_addralign;
  Elf32_Word	sh_entsize;
} Elf32_Shdr;

typedef struct elf64_shdr {
  Elf64_Word sh_name;		/* Section name, index in string tbl */
  Elf64_Word sh_type;		/* Type of section */
  Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
  Elf64_Addr sh_addr;		/* Section virtual addr at execution */
  Elf64_Off sh_offset;		/* Section file offset */
  Elf64_Xword sh_size;		/* Size of section in bytes */
  Elf64_Word sh_link;		/* Index of another section */
  Elf64_Word sh_info;		/* Additional section information */
  Elf64_Xword sh_addralign;	/* Section alignment */
  Elf64_Xword sh_entsize;	/* Entry size if section holds table */
} Elf64_Shdr;

#define	EI_MAG0		0		/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_OSABI	7
#define	EI_PAD		8

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

#define ELFDATANONE	0		/* e_ident[EI_DATA] */
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

#define EV_NONE		0		/* e_version, EI_VERSION */
#define EV_CURRENT	1
#define EV_NUM		2

#define ELFOSABI_NONE	0
#define ELFOSABI_LINUX	3

#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif

/*
 * Notes used in ET_CORE. Architectures export some of the arch register sets
 * using the corresponding note types via the PTRACE_GETREGSET and
 * PTRACE_SETREGSET requests.
 * The note name for these types is "LINUX", except NT_PRFPREG that is named
 * "CORE".
 */
#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3
#define NT_TASKSTRUCT	4
#define NT_AUXV		6
/*
 * Note to userspace developers: size of NT_SIGINFO note may increase
 * in the future to accomodate more fields, don't assume it is fixed!
 */
#define NT_SIGINFO      0x53494749
#define NT_FILE         0x46494c45
#define NT_PRXFPREG     0x46e62b7f      /* copied from gdb5.1/include/elf/common.h */
#define NT_PPC_VMX	0x100		/* PowerPC Altivec/VMX registers */
#define NT_PPC_SPE	0x101		/* PowerPC SPE/EVR registers */
#define NT_PPC_VSX	0x102		/* PowerPC VSX registers */
#define NT_PPC_TAR	0x103		/* Target Address Register */
#define NT_PPC_PPR	0x104		/* Program Priority Register */
#define NT_PPC_DSCR	0x105		/* Data Stream Control Register */
#define NT_PPC_EBB	0x106		/* Event Based Branch Registers */
#define NT_PPC_PMU	0x107		/* Performance Monitor Registers */
#define NT_PPC_TM_CGPR	0x108		/* TM checkpointed GPR Registers */
#define NT_PPC_TM_CFPR	0x109		/* TM checkpointed FPR Registers */
#define NT_PPC_TM_CVMX	0x10a		/* TM checkpointed VMX Registers */
#define NT_PPC_TM_CVSX	0x10b		/* TM checkpointed VSX Registers */
#define NT_PPC_TM_SPR	0x10c		/* TM Special Purpose Registers */
#define NT_PPC_TM_CTAR	0x10d		/* TM checkpointed Target Address Register */
#define NT_PPC_TM_CPPR	0x10e		/* TM checkpointed Program Priority Register */
#define NT_PPC_TM_CDSCR	0x10f		/* TM checkpointed Data Stream Control Register */
#define NT_PPC_PKEY	0x110		/* Memory Protection Keys registers */
#define NT_PPC_DEXCR	0x111		/* PowerPC DEXCR registers */
#define NT_PPC_HASHKEYR	0x112		/* PowerPC HASHKEYR register */
#define NT_386_TLS	0x200		/* i386 TLS slots (struct user_desc) */
#define NT_386_IOPERM	0x201		/* x86 io permission bitmap (1=deny) */
#define NT_X86_XSTATE	0x202		/* x86 extended state using xsave */
/* Old binutils treats 0x203 as a CET state */
#define NT_X86_SHSTK	0x204		/* x86 SHSTK state */
#define NT_X86_XSAVE_LAYOUT	0x205	/* XSAVE layout description */
#define NT_S390_HIGH_GPRS	0x300	/* s390 upper register halves */
#define NT_S390_TIMER	0x301		/* s390 timer register */
#define NT_S390_TODCMP	0x302		/* s390 TOD clock comparator register */
#define NT_S390_TODPREG	0x303		/* s390 TOD programmable register */
#define NT_S390_CTRS	0x304		/* s390 control registers */
#define NT_S390_PREFIX	0x305		/* s390 prefix register */
#define NT_S390_LAST_BREAK	0x306	/* s390 breaking event address */
#define NT_S390_SYSTEM_CALL	0x307	/* s390 system call restart data */
#define NT_S390_TDB	0x308		/* s390 transaction diagnostic block */
#define NT_S390_VXRS_LOW	0x309	/* s390 vector registers 0-15 upper half */
#define NT_S390_VXRS_HIGH	0x30a	/* s390 vector registers 16-31 */
#define NT_S390_GS_CB	0x30b		/* s390 guarded storage registers */
#define NT_S390_GS_BC	0x30c		/* s390 guarded storage broadcast control block */
#define NT_S390_RI_CB	0x30d		/* s390 runtime instrumentation */
#define NT_S390_PV_CPU_DATA	0x30e	/* s390 protvirt cpu dump data */
#define NT_ARM_VFP	0x400		/* ARM VFP/NEON registers */
#define NT_ARM_TLS	0x401		/* ARM TLS register */
#define NT_ARM_HW_BREAK	0x402		/* ARM hardware breakpoint registers */
#define NT_ARM_HW_WATCH	0x403		/* ARM hardware watchpoint registers */
#define NT_ARM_SYSTEM_CALL	0x404	/* ARM system call number */
#define NT_ARM_SVE	0x405		/* ARM Scalable Vector Extension registers */
#define NT_ARM_PAC_MASK		0x406	/* ARM pointer authentication code masks */
#define NT_ARM_PACA_KEYS	0x407	/* ARM pointer authentication address keys */
#define NT_ARM_PACG_KEYS	0x408	/* ARM pointer authentication generic key */
#define NT_ARM_TAGGED_ADDR_CTRL	0x409	/* arm64 tagged address control (prctl()) */
#define NT_ARM_PAC_ENABLED_KEYS	0x40a	/* arm64 ptr auth enabled keys (prctl()) */
#define NT_ARM_SSVE	0x40b		/* ARM Streaming SVE registers */
#define NT_ARM_ZA	0x40c		/* ARM SME ZA registers */
#define NT_ARM_ZT	0x40d		/* ARM SME ZT registers */
#define NT_ARM_FPMR	0x40e		/* ARM floating point mode register */
#define NT_ARM_POE	0x40f		/* ARM POE registers */
#define NT_ARM_GCS	0x410		/* ARM GCS state */
#define NT_ARC_V2	0x600		/* ARCv2 accumulator/extra registers */
#define NT_VMCOREDD	0x700		/* Vmcore Device Dump Note */
#define NT_MIPS_DSP	0x800		/* MIPS DSP ASE registers */
#define NT_MIPS_FP_MODE	0x801		/* MIPS floating-point mode */
#define NT_MIPS_MSA	0x802		/* MIPS SIMD registers */
#define NT_RISCV_CSR	0x900		/* RISC-V Control and Status Registers */
#define NT_RISCV_VECTOR	0x901		/* RISC-V vector registers */
#define NT_RISCV_TAGGED_ADDR_CTRL 0x902	/* RISC-V tagged address control (prctl()) */
#define NT_LOONGARCH_CPUCFG	0xa00	/* LoongArch CPU config registers */
#define NT_LOONGARCH_CSR	0xa01	/* LoongArch control and status registers */
#define NT_LOONGARCH_LSX	0xa02	/* LoongArch Loongson SIMD Extension registers */
#define NT_LOONGARCH_LASX	0xa03	/* LoongArch Loongson Advanced SIMD Extension registers */
#define NT_LOONGARCH_LBT	0xa04	/* LoongArch Loongson Binary Translation registers */
#define NT_LOONGARCH_HW_BREAK	0xa05   /* LoongArch hardware breakpoint registers */
#define NT_LOONGARCH_HW_WATCH	0xa06   /* LoongArch hardware watchpoint registers */

/* Note types with note name "GNU" */
#define NT_GNU_PROPERTY_TYPE_0	5

/* Note header in a PT_NOTE section */
typedef struct elf32_note {
  Elf32_Word	n_namesz;	/* Name size */
  Elf32_Word	n_descsz;	/* Content size */
  Elf32_Word	n_type;		/* Content type */
} Elf32_Nhdr;

/* Note header in a PT_NOTE section */
typedef struct elf64_note {
  Elf64_Word n_namesz;	/* Name size */
  Elf64_Word n_descsz;	/* Content size */
  Elf64_Word n_type;	/* Content type */
} Elf64_Nhdr;

/* .note.gnu.property types for EM_AARCH64: */
#define GNU_PROPERTY_AARCH64_FEATURE_1_AND	0xc0000000

/* Bits for GNU_PROPERTY_AARCH64_FEATURE_1_BTI */
#define GNU_PROPERTY_AARCH64_FEATURE_1_BTI	(1U << 0)

long file_size(char* fname) {
    FILE *file = fopen(fname, "rb");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
	fclose(file);
	return file_size;
}

long file_read(char* fname, char* buffer, int buffer_size) {
    FILE *file = fopen(fname, "rb");
    size_t bytes_read = fread(buffer, 1, buffer_size, file);
    buffer[bytes_read] = '\0';
	fclose(file);
	return bytes_read;
}
u32 file_write(char* fname, u8* buffer, u32 buffer_size) {
    FILE *file = fopen(fname, "wb");
    size_t bytes_written = fwrite(buffer, 1, buffer_size, file);
	fclose(file);
	return bytes_written;
}

char* mem_alloc(int size) {
	return (char*)malloc(size);
}

char* int2str(i64 value, char digits[16]) {
	int i = 14;
	do {
		digits[i--] = (value % 10) + '0';
		value /= 10;
	} while(value > 0 && i >= 0);
	return digits+i+1;
}
#pragma function(memcpy)
void* memcpy(void* dest, void* src, i64 n) {
	u8* destBytes = (u8*)dest;
	u8* srcBytes = (u8*)src;
	while(n--) *destBytes++ = *srcBytes++;
	return dest;
}
#define offsetof(t, f) (&(((t *)0)->f))

// TODO: try re-organizing the stuff from the phdrs and shdrs and see if
	// 1) we can cut anything out
	// 2) we can put the .text at the end
		// followed by a section we can write to
		// then maybe we can also map that section with a executable segment
		// so we write to one virtual address
		// then we execute at another virtual address that maps to the same physical address as the other one
		// effectively being able to write then execute without a syscall
		// or we just do the syscall to MPROTECT or whatever it is

typedef struct {
	Elf64_Ehdr ehdr;
	Elf64_Phdr phdrs[8]; // phdr, interp, gnu hash, .text, .dynamic, .dynamic, eh_frame, stack
	u64 unknown[59]; // section content
	Elf64_Shdr shdrs[6]; // null, .gnu.hash, .dynstr, .text, .dynamic, .shstrstab
} ELF;

int main(int argc, char *argv[]) {
	ELF* elf = (ELF*)elfb;
	printf("%x\n", offsetof(ELF, shdrs));
	printf("%x\n", elf->ehdr.e_shoff);
	printf("%x\n", 0x3d0+0x40*3+0x28);
	printf("%x\n", sizeof(ELF));
	for(int i = 0; i < 8; i++) {
		printf("%x, %x, %x, %x\n", elf->phdrs[i].p_type, elf->phdrs[i].p_flags, elf->phdrs[i].p_offset, elf->phdrs[i].p_filesz);
	}
	int prev_end = 0;
	for(int i = 0; i < 6; i++) {
		printf("%x, %x, %x, %x, %x, %x, %x\n", elf->shdrs[i].sh_type, elf->shdrs[i].sh_flags, prev_end, elf->shdrs[i].sh_offset, elf->shdrs[i].sh_size, elf->shdrs[i].sh_link, elf->shdrs[i].sh_info);
		prev_end = elf->shdrs[i].sh_offset + elf->shdrs[i].sh_size;
	}
	for(int i = 0; i < 8; i++) {
		printf("%x, %x\n", elf->phdrs[i].p_vaddr, elf->phdrs[i].p_paddr);
	}
	int buffer_size = 64*1024;
	char* buffer = mem_alloc(buffer_size);
	int ehdr_size = sizeof(Elf64_Ehdr);
	int phdr_size = sizeof(Elf64_Phdr);
	int shdr_size = sizeof(Elf64_Shdr);
	int shdrs_start = (int)offsetof(ELF, shdrs);
	int off = 0;
	memcpy(buffer, elfb, ehdr_size);
	off += ehdr_size;
	memcpy(buffer+off, elfb+ehdr_size+phdr_size*3, phdr_size); off += phdr_size; // segment LOAD    RE .dynamic .gnu.hash .text
	memcpy(buffer+off, elfb+ehdr_size+phdr_size*5, phdr_size); off += phdr_size; // segment DYNAMIC RW .dynamic
	memcpy(buffer+off, elfb+shdrs_start+shdr_size*3, shdr_size); off += shdr_size; // section .text     PROGBITS AX
	memcpy(buffer+off, elfb+shdrs_start+shdr_size*1, shdr_size); off += shdr_size; // section .gnu.hash GNU_HASH A
	memcpy(buffer+off, elfb+shdrs_start+shdr_size*4, shdr_size); off += shdr_size; // section .dynamic  DYNAMIC  WA
	memcpy(buffer+off, elfb+shdrs_start+shdr_size*5, shdr_size); off += shdr_size; // section .shstrtab STRTAB   
	//// lay out section data and update offset and size of sections
	// update offset and size of segments and sections
	// .dynamic data
	int osi, isi, dynamic_off, dynamic_size;
	osi = 2; isi = 4; dynamic_off = off; dynamic_size = elf->shdrs[isi].sh_size;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_addr = 0x100000+off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_offset = off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_size = elf->shdrs[isi].sh_size;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_link = 3; // link .dynamic to .shstrtab
	u64* dyn_data = (u64*)(buffer+off);
	memcpy(buffer+off, elfb+elf->shdrs[isi].sh_offset, elf->shdrs[isi].sh_size); off += elf->shdrs[isi].sh_size;
	// .gnu.hash data
	osi = 1; isi = 1;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_addr = 0x1000+off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_offset = off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_size = elf->shdrs[isi].sh_size;
	dyn_data[8*2+1] = 0x1000+off;
	memcpy(buffer+off, elfb+elf->shdrs[isi].sh_offset, elf->shdrs[isi].sh_size); off += elf->shdrs[isi].sh_size;
	// .shstrtab data
	osi = 3; isi = 5;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_addr = 0x1000+off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_offset = off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_size = elf->shdrs[isi].sh_size;
	dyn_data[6*2+1] = 0x1000+off;
	dyn_data[4*2+1] = 0x1000+off;
	memcpy(buffer+off, elfb+elf->shdrs[isi].sh_offset, elf->shdrs[isi].sh_size); off += elf->shdrs[isi].sh_size;
	// .text data
	osi = 0; isi = 3;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_addr = 0x1000+off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_offset = off;
	((Elf64_Shdr*)(buffer+ehdr_size+phdr_size*2+shdr_size*osi))->sh_size = elf->shdrs[isi].sh_size;
	memcpy(buffer+off, elfb+elf->shdrs[isi].sh_offset, elf->shdrs[isi].sh_size); off += elf->shdrs[isi].sh_size;
	//// update offset and size of segments
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*0))->p_offset = 0;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*0))->p_filesz = buffer_size;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*0))->p_memsz = buffer_size;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*0))->p_vaddr = 0x1000;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*0))->p_paddr = 0x1000;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*1))->p_offset = dynamic_off;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*1))->p_filesz = buffer_size-dynamic_off;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*1))->p_memsz = buffer_size-dynamic_off;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*1))->p_vaddr = 0x100000;
	((Elf64_Phdr*)(buffer+ehdr_size+phdr_size*1))->p_paddr = 0x100000;
	//// elf header
	((Elf64_Ehdr*)(buffer))->e_entry = 0x1000;
	((Elf64_Ehdr*)(buffer))->e_shoff = ehdr_size+phdr_size*2;
	((Elf64_Ehdr*)(buffer))->e_phnum = 2;
	((Elf64_Ehdr*)(buffer))->e_shnum = 4;
	((Elf64_Ehdr*)(buffer))->e_shstrndx = 3;
	// make RE segment and .text section be at the end and have a bunch of 0 padding to just overwrite
		// if i run out of padding space then I gotta update the segment and section header
		// but since it's at the end, I dont have to update the other segments or sections
	file_write("b.out", buffer, buffer_size);
    return 0;
}

int test_main(int argc, char *argv[]) {
    if (argc != 2) { printf("provide file name\n"); return 1; }
	long buffer_size = file_size(argv[1]) + 1;
    char* buffer = mem_alloc(buffer_size);
	file_read(argv[1], buffer, buffer_size);
    printf("%s", buffer);
    return 0;
}
