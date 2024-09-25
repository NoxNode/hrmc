##as -c -o hrmc.o hrmc.s && objcopy -O binary -j .text hrmc.o hrmc.exe && ./hrmc.exe; echo $?
##xxd -p hrmc.exe
##xxd hrmc.exe
##xxd dump.bin
##rm *.exe *.o *.obj
##objdump -M intel -d -j .text hrmc.exe
##objdump -M intel -x hrmc.exe
/* Human Readable Machine Code compiler for Windows x64
hrmc.s is a compiler for hrmc written in gnu assembler
*/
.intel_syntax noprefix
.code64

## DOS Header
mzhdr:
	.ascii "MZ"          # e_magic
	.word 0              # e_cblp                      UNUSED
	.word 0              # e_cp                        UNUSED
	.word 0              # e_crlc                      UNUSED
	.word 0              # e_cparhdr                   UNUSED
	.word 0              # e_minalloc                  UNUSED
	.word 0              # e_maxalloc                  UNUSED
	.word 0              # e_ss                        UNUSED
	.word 0              # e_sp                        UNUSED
	.word 0              # e_csum                      UNUSED
	.word 0              # e_ip                        UNUSED
	.word 0              # e_cs                        UNUSED
	.word 0              # e_lsarlc                    UNUSED
	.word 0              # e_ovno                      UNUSED
	.quad 0              # e_res                       UNUSED
	.word 0              # e_oemid                     UNUSED
	.word 0              # e_oeminfo                   UNUSED
	.quad 0              # e_res2_0                    UNUSED
	.quad 0              # e_res2_1                    UNUSED
	.long 0              # e_res2_2                    UNUSED
	.long pesig          # e_lfanew

## PE header
pesig:
	.ascii "PE\0\0"
pehdr:
	.word 0x8664         # Machine x86_64
	.word 1              # NumberOfSections
	.long 0              # TimeDateStamp               UNUSED
	.long 0              # PointerToSymbolTable        UNUSED
	.long 0              # NumberOfSymbols             UNUSED
	.word opthdrsize     # SizeOfOptionalHeader (including data directories)
	.word 0x023          # Characteristics (1=no relocs, 2=executable, 0x20=large addresses)
opthdr:
	.word 0x20B          # Magic (PE32+)
	.byte 0              # MajorLinkerVersion          UNUSED
	.byte 0              # MinorLinkerVersion          UNUSED
	.long codesize       # SizeOfCode
	.long 0              # SizeOfInitializedData       UNUSED
	.long 0              # SizeOfUninitializedData     UNUSED
	.long start          # AddressOfEntryPoint
	.long start           # BaseOfCode                  UNUSED
	.quad 0x400000       # ImageBase
	.long 4              # SectionAlignment
	.long 4              # FileAlignment
	.word 0              # MajorOperatingSystemVersion UNUSED
	.word 0              # MinorOperatingSystemVersion UNUSED
	.word 0              # MajorImageVersion           UNUSED
	.word 0              # MinorImageVersion           UNUSED
	.word 4              # MajorSubsystemVersion
	.word 0              # MinorSubsystemVersion       UNUSED
	.long 0              # Win32VersionValue           UNUSED
	.long filesize       # SizeOfImage
	.long hdrsize        # SizeOfHeaders
	.long 0              # CheckSum                    UNUSED
	.word 2              # Subsystem (2=GUI,3=CUI,10=EFI app)
	.word 0x400          # DllCharacteristics          UNUSED
	.quad 0x100000       # SizeOfStackReserve          UNUSED
	.quad 0x1000         # SizeOfStackCommit
	.quad 0x1000000      # SizeOfHeapReserve
	.quad 0x1000         # SizeOfHeapCommit            UNUSED
	.long 0              # LoaderFlags                 UNUSED
	.long 16             # NumberOfRvaAndSizes

## Data directories (first one is Export Table)
## each data directory has 2 32-bit fields, but for now I'll just use 1 64-bit field
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0

opthdrsize = . - opthdr

## PE code section
	.ascii ".text\0\0\0" # Name
	.long codesize       # VirtualSize
	.long hdrsize        # VirtualAddress
	.long codesize       # SizeOfRawData
	.long start           # PointerToRawData
	.long 0              # PointerToRelocations        UNUSED
	.long 0              # PointerToLinenumbers        UNUSED
	.word 0              # NumberOfRelocations         UNUSED
	.word 0              # NumberOfLinenumbers
	.long 0xE0000020     # Characteristics (0x20=code, MSB_2=execute, MSB_4=read, MSB_8=write)
hdrsize = . - mzhdr

## Entry Point
start:
	lea rbx, [rip+hrmc_table]      # rbx = &hrmc_table
	lea rsi, [rip+hrmc_bytecode]   # rsi = &hrmc_bytecode
	lea rdi, [rip+hrmc_dest]       # rdi = &hrmc_dest

## hrmc compiler v4
compile_loop:
# c = u8[rsi]; d = u8[rsi + 1]; rsi += 2 (c for code, d for data)
	movzx ecx, byte ptr [rsi]
	movzx edx, byte ptr [rsi+1]
	add rsi, 2
# r8 = c << 4; r9 = d << 4; r10 = c & 0xF0 (useful for later)
	mov r8, rcx
	shl r8, 4
	mov r9, rdx
	shl r9, 4
	mov r10, rcx
	and r10, 0xF0
# u128[rdi] = u128[hrmc_table + (c << 4)]; rdi += 16
	movdqa xmm0, [rbx+r8]
	movdqa [rdi], xmm0
	add rdi, 16
# if c is 0 then we done compiling, jmp to the code we generated
	cmp cl, 0
	je hrmc_dest
# if c & 0xF0 is 0xD0 then u32[dest-9] = d << 4
	cmp r10, 0xD0
	jne end_of_global_offset_patching
	mov [rdi-9], r9d
	jmp compile_loop
end_of_global_offset_patching:
# if c & 0xF0 is 0xE0 then u8[dest-9] = d
	cmp r10, 0xE0
	jne end_of_const_imm_patching
	mov [rdi-9], dl
	jmp compile_loop
end_of_const_imm_patching:
# if c is 0xBB then u32[dest-4] = u64[hrmc_table + (d << 4)] - dest
	cmp cl, 0xBB
	jne end_of_branch_backwards_patching
	mov rax, [rbx + r9]
	sub rax, rdi
	mov [rdi-4], eax
	jmp compile_loop
end_of_branch_backwards_patching:
# if c & 0xF0 is 0x50 then u32[dest-11] = sign_ext(d) << 3
	cmp r10, 0x50
	jne end_of_stack_offset_patching
	movsx eax, dl
	shl eax, 3
	mov [rdi-11], eax
	jmp compile_loop
end_of_stack_offset_patching:
# if c & 0xF0 is not 0x70 then continue compile_loop
	cmp r10, 0x70
	jne compile_loop

# if c is 0x7E then u64[hrmc_table + (d << 4)] = dest
	cmp cl, 0x7E
	jne end_of_tag_enscribing
	mov [rbx + r9], rdi
end_of_tag_enscribing:
# if c is not 0x7F then continue compile_loop
	cmp cl, 0x7F
	jne compile_loop
	lea r11, [rsi-2] # src_iter
	mov r12, rdi # dest_iter
	mov r13, rdx # target_d
fixup_branch_forwards_loop:
# src_iter -= 2; c = [src_iter]; d = [src_iter+1]; dest_iter -= 16
	sub r11, 2
	mov cl, [r11]
	mov dl, [r11+1]
	sub r12, 16
# if d != target_d then continue fixup_branch_forwards_loop
	cmp rdx, r13
	jne fixup_branch_forwards_loop
# if c == 0xBF or c == 0xB0 then u32[dest_it-4] = dest - dest_it
	cmp cl, 0xBF
	je fixup
	cmp cl, 0xB0
	jne end_of_fixup
fixup:
	mov rax, rdi
	sub rax, r12
	mov [r12-4], eax
end_of_fixup:
# if c is not 7E then continue fixup_branch_forwards_loop
	cmp cl, 0x7E
	jne fixup_branch_forwards_loop
	jmp compile_loop

.align 16, 0
hrmc_table:
.zero 16*5 # 00-04
.asciz "GetStdHandle"; .align 16,0 # 05
.zero 16*4 # 06-09
.asciz "WriteFile"; .align 16,0 # 0A
.zero 16*5 # 0B-0F
pop rax; movzx rax, BYTE PTR [rax]; push rax   ; .align 16, 0x90 # 10
pop rax; movsx rax, BYTE PTR [rax]; push rax   ; .align 16, 0x90 # 11
pop rax; movzx rax, WORD PTR [rax]; push rax   ; .align 16, 0x90 # 12
pop rax; movsx rax, WORD PTR [rax]; push rax   ; .align 16, 0x90 # 13
pop rax; mov eax, DWORD PTR [rax]; push rax    ; .align 16, 0x90 # 14
pop rax; movsxd rax, DWORD PTR [rax]; push rax ; .align 16, 0x90 # 15
.zero 16*2 # 16-17
pop rax; mov rax, QWORD PTR [rax]; push rax ; .align 16, 0x90 # 18
pop rax; mov rax, QWORD PTR [rax]; push rax ; .align 16, 0x90 # 19
.zero 16*6 # 1A-1F
.zero 16*16 # 20-2F
.zero 16*2 # 30-31
.byte 'K', 0,   'E', 0,    'R', 0,   'N', 0,    'E', 0,   'L', 0,    '3', 0,   '2', 0    # 32 pop rax; mov rax, QWORD PTR [rax]; push rax
.byte '.', 0,   'D', 0,    'L', 0,   'L', 0,    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 # 33 pop rax; mov rax, QWORD PTR [rax]; push rax
.zero 16*12 # 33-3F
.zero 16*16 # 40-4F
.zero 16*16 # 90-9F
.zero 16*1 # 60
pop rcx;                          .align 16, 0x90 # 61
pop rcx; pop rdx;                 .align 16, 0x90 # 62
pop rcx; pop rdx; pop r8;         .align 16, 0x90 # 63
pop rcx; pop rdx; pop r8; pop r9; sub rsp, 0x20; .align 16, 0x90 # 64 rsp-20 is for shadow space
.zero 16*10 # 65-6E
and rsp, -16; .align 16, 0x90 # 6F stack must be 16 byte aligned on function entry
.zero 16*14 # 70-7D
or al,al ; .align 16, 0x90 # 7E
or rax,rax ; .align 16, 0x90 # 7F
.zero 16*16 # 80-8F
pop rax; mov BYTE PTR[rax],al ;   .align 16, 0x90 # 50
pop rax; mov BYTE PTR[rax],al ;   .align 16, 0x90 # 51
pop rax; mov WORD PTR[rax],ax ;   .align 16, 0x90 # 52
pop rax; mov WORD PTR[rax],ax ;   .align 16, 0x90 # 53
pop rax; mov DWORD PTR[rax],eax ; .align 16, 0x90 # 54
pop rax; mov DWORD PTR[rax],eax ; .align 16, 0x90 # 55
.zero 16*2 # 56-57
pop rax; mov QWORD PTR[rax],rax ; .align 16, 0x90 # 58
pop rax; mov QWORD PTR[rax],rax ; .align 16, 0x90 # 59
.zero 16*5 # 5A-5E
push rbp; mov rbp, rsp ;          .align 16, 0x90 # 5F
pop rcx; pop rax; or rax, rcx; push rax ; .align 16, 0x90 # A0
pop rax; not rax; push rax; ; .align 16, 0x90 # A1
cqo; pop rcx; pop rax; div rax, rcx; push rax ; .align 16, 0x90 # A2
pop rcx; pop rax; shr rax, rcx; push rax ; .align 16, 0x90 # A3
pop rcx; pop rax; shl rax, rcx; push rax ; .align 16, 0x90 # A4
cqo; pop rcx; pop rax; div rax, rcx; push rdx ; .align 16, 0x90 # A5
pop rcx; pop rax; xor rax, rcx; push rax ; .align 16, 0x90 # A6
pop rcx; pop rax; and rax, rcx; push rax ; .align 16, 0x90 # A7
pop rax; neg rax; push rax; ; .align 16, 0x90 # A8
pop rcx; pop rax; sar rax, rcx; push rax ; .align 16, 0x90 # A9
pop rcx; pop rax; add rax, rcx; push rax ; .align 16, 0x90 # AA
pop rcx; pop rax; sub rax, rcx; push rax ; .align 16, 0x90 # AB
pop rcx; pop rax; imul rcx; push rdx ; .align 16, 0x90 # AC
cqo; pop rcx; pop rax; idiv rax, rcx; push rax ; .align 16, 0x90 # AD
pop rcx; pop rax; imul rcx; push rax ; .align 16, 0x90 # AE
cqo; pop rcx; pop rax; idiv rax, rcx; push rdx ; .align 16, 0x90 # AF
.byte 0x90,0x90, 0x90,0x90,0x90,0x90;pop rax; test rax, rax; je mzhdr; # B0 
.zero 16*6 # B1-B6
pop rax; mov rsp, rbp; pop rbp; ret ; .align 16, 0x90 # B7
.zero 16*3 # B8-BA
.byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90, 0x90,0x90,0x90;jmp mzhdr; # BB
.zero 16*3 # BC-BE
.byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90, 0x90,0x90,0x90;jmp mzhdr; # BF
.zero 16*1 # C0
pop rcx; pop rax; cmp rax,rcx; setne al; movsx rax, al ; push rax; .align 16, 0x90 # C1
pop rcx; pop rax; cmp rax,rcx; setbe al; movsx rax, al ; push rax; .align 16, 0x90 # C2
pop rcx; pop rax; cmp rax,rcx; setae al; movsx rax, al ; push rax; .align 16, 0x90 # C3
pop rcx; pop rax; cmp rax,rcx; setl al; movsx rax, al ; push rax; .align 16, 0x90 # C4
.zero 16*1 # C5
pop rcx; pop rax; cmp rax,rcx; setle al; movsx rax, al ; push rax; .align 16, 0x90 # C6
pop rcx; pop rax; cmp rax,rcx; setg al; movsx rax, al ; push rax; .align 16, 0x90 # C7
.zero 16*1 # C8
pop rcx; pop rax; cmp rax,rcx; setge al; movsx rax, al ; push rax; .align 16, 0x90 # C9
pop rcx; pop rax; cmp rax,rcx; seta al; movsx rax, al ; push rax; .align 16, 0x90 # CA
pop rcx; pop rax; cmp rax,rcx; setb al; movsx rax, al ; push rax; .align 16, 0x90 # CB
.zero 16*2 # CC-CD
pop rcx; pop rax; cmp rax,rcx; sete al; movsx rax, al ; push rax; .align 16, 0x90 # CE
.zero 16*1 # CF
.zero 16*1 # D0
.byte 0x90,0x90,0x90,0x90; mov rax, QWORD PTR [rbx+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90; # D1
.zero 16*3 # D2-D4
.byte 0x90,0x90,0x90; pop rax; mov QWORD PTR[rbx+0x76543210],rax; .byte 0x90, 0x90,0x90,0x90,0x90; # D5
.zero 16*4 # D6-D9
.byte 0x90,0x90,0x90,0x90; mov rcx, QWORD PTR [rbx+0x76543210]; pop rax; add rax, rcx; push rax # DA
.zero 16*3 # DB-DE
.byte 0x90,0x90,0x90,0x90; lea rax, QWORD PTR [rbx+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90; # DE
.byte 0x90,0x90,0x90,0x90; mov rax, [rbx+0x76543210]; call rax; push rax; .byte 0x90,0x90; # DF
.zero 16*1 # E0
.byte 0x90,0x90,0x90,0x90, 0x90,0x90;push 0x2a; .byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90; # E1 
.zero 16*3 # E2-E4
.byte 0x90,0x90,0x90,0x90, 0x90; pop rax; push 0x2a; pop rcx; add rax, rcx; mov rax, QWORD PTR[rax]; push rax; # E5
.byte 0x90,0x90; mov rax,QWORD PTR gs:0x60; push rax; .byte 0x90,0x90,0x90,0x90; # E6
.zero 16*3 # E7-E9
.byte 0x90,0x90,0x90,0x90, 0x90; pop rcx; push 42; pop rax; add rax, rcx; push rax; .byte 0x90,0x90,0x90; # EA
.zero 16*2 # EB-EC
.byte 0x90,0x90,0x90,0x90, 0x90; pop rax; push 0x2a; pop rcx; add rax, rcx; mov rax, QWORD PTR[rax]; push rax; # ED
.byte 0x90,0x90,0x90,0x90; mov rax, [rbp+42]; push rax; .byte 0x90,0x90,0x90, 0x90,0x90,0x90,0x90; # EE
.zero 16*1 # EF
.zero 16*16 # F0-FF

hrmc_bytecode:
## int main()
.byte 0x7E,0x9E, 0x5F,0x00
.byte 0xDF,0x92, 0xD5,0x3B
.byte 0xDE,0x05, 0xD1,0x3B, 0xDF,0x9A, 0xD5,0x90
.byte 0xDE,0x0A, 0xD1,0x3B, 0xDF,0x9A, 0xD5,0x9F
.byte 0xE1,0xF5, 0x61,0x00, 0xDF,0x90, 0xD5,0x07
.byte 0x6F,0x00, 0xE1,0x00, 0xDE,0x01, 0xE1,0x0C, 0xDE,0x05, 0xD1,0x07, 0x64,0x00, 0xDF,0x9F
.byte 0xB7,0x00

## void* get_kernel32()
.byte 0x7E,0x92, 0x5F,0x00
.byte 0xE6,0x60, 0xED,0x18, 0xED,0x20, 0xD5,0x2D
.byte 0xD1,0x2D, 0xED,0x20, 0xD5,0x3B
.byte 0x7E,0x71, 0xD1,0x3B, 0xB0,0x71
     .byte 0xD1,0x2D, 0xEA,0x38, 0x12,0x00, 0xD5,0x31
     .byte 0xD1,0x2D, 0xED,0x40, 0xD5,0x3A
     .byte 0xD1,0x31, 0xEA,0xFE, 0xD5,0x01
     .byte 0xE1,0x16, 0xD5,0x02
     .byte 0x7E,0x72, 0xD1,0x02, 0xE1,0x00, 0xC9,0x00, 0xB0,0x72
          .byte 0xD1,0x3A, 0xDA,0x01, 0x12,0x00, 0xD5,0x03
          .byte 0xDE,0x32, 0xDA,0x02, 0x12,0x00, 0xD5,0x04
          .byte 0x7E,0x73, 0xD1,0x03, 0xD1,0x04, 0xC1,0x00, 0xD1,0x03, 0xD1,0x04, 0xEA,0x20, 0xC1,0x00, 0xA7,0x00, 0xB0,0x73
               .byte 0xBF,0x72
          .byte 0x7F,0x73
          .byte 0xD1,0x01, 0xEA,0xFE, 0xD5,0x01
          .byte 0xD1,0x02, 0xEA,0xFE, 0xD5,0x02
          .byte 0xBB,0x72
     .byte 0x7F,0x72
     .byte 0x7E,0x72, 0xD1,0x02, 0xE1,0x00, 0xC4,0x00, 0xB0,0x72
          .byte 0xD1,0x3B, 0xB7,0x00
     .byte 0x7F,0x72
     .byte 0xD1,0x2D, 0x18,0x00, 0xD5,0x2D
     .byte 0xD1,0x2D, 0xED,0x20, 0xD5,0x3B
     .byte 0xBB,0x71
.byte 0x7F,0x71
.byte 0xE1,0x00, 0xB7,0x00

## int strcmp(char* s1, char* s2)
.byte 0x7E,0x9C, 0x5F,0x00, 0xEE,0x10, 0xD5,0x01, 0xEE,0x18, 0xD5,0x02
.byte 0x7E,0x71, 0xD1,0x01, 0x10,0x00, 0xB0,0x71, 0xD1,0x02, 0x10,0x00, 0xB0,0x71, 0xD1,0x01, 0x10,0x00, 0xD1,0x02, 0x10,0x00, 0xCE,0x00, 0xB0,0x71
     .byte 0xD1,0x01, 0xEA,0x01, 0xD5,0x01, 0xD1,0x02, 0xEA,0x01, 0xD5,0x02
     .byte 0xBB,0x71
.byte 0x7F,0x71
.byte 0x7E,0x71, 0xD1,0x01, 0x10,0x00, 0xD1,0x02, 0x10,0x00, 0xCE,0x00, 0xB0,0x71
     .byte 0xE1,0x00, 0xB7,0x00
.byte 0x7F,0x71
.byte 0xD1,0x01, 0x10,0x00, 0xD1,0x02, 0x10,0x00, 0xAB,0x00, 0xB7,0x00

## void* GetProcAddress(void* handle, char* proc_name)
.byte 0x7E,0x9A, 0x5F,0x00, 0xEE,0x10, 0xD5,0x3B, 0xEE,0x18, 0xD5,0x2A
.byte 0xD1,0x3B, 0xEA,0x3C, 0x14,0x00, 0xDA,0x3B, 0xD5,0x3D
.byte 0xD1,0x3D, 0xEA,0x18, 0xEA,0x70, 0x14,0x00, 0xDA,0x3B, 0xD5,0x37
.byte 0xD1,0x37, 0xEA,0x20, 0x14,0x00, 0xDA,0x3B, 0xD5,0x27
.byte 0xE1,0x00, 0xD5,0x30
.byte 0xD1,0x37, 0xEA,0x18, 0x15,0x00, 0xD5,0x3F
.byte 0xE1,0x00, 0xD5,0x31
.byte 0xE1,0x00, 0xD5,0x3C
.byte 0x7E,0x71
     .byte 0x7E,0x72, 0xD1,0x3C, 0xE1,0x00, 0xC7,0x00, 0xB0,0x72, 0xD1,0x31, 0xD5,0x30, 0x7F,0x72
     .byte 0x7E,0x72, 0xD1,0x3C, 0xE1,0x00, 0xC4,0x00, 0xB0,0x72, 0xD1,0x31, 0xD5,0x3F, 0x7F,0x72
     .byte 0xD1,0x3F, 0xDA,0x30, 0xE1,0x01, 0xA9,0x00, 0xD5,0x31
     .byte 0xD1,0x31, 0xE1,0x02, 0xA4,0x00, 0xDA,0x27, 0x14,0x00, 0xDA,0x3B, 0xD5,0x3A
     .byte 0xD1,0x3A, 0xD1,0x2A, 0xDF,0x9C, 0xD5,0x3C
.byte 0xD1,0x3C, 0xE1,0x00, 0xC1,0x00, 0xB0,0x71, 0xBB,0x71, 0x7F,0x71
.byte 0x7E,0x71, 0xD1,0x31, 0xE1,0x02, 0xA4,0x00, 0xDA,0x27, 0x14,0x00, 0xDA,0x3B, 0xD1,0x2A, 0xDF,0x9C, 0xE1,0x00, 0xC1,0x00, 0xB0,0x71, 0xE1,0x00, 0xB7,0x00, 0x7F,0x71
.byte 0xD1,0x37, 0xEA,0x24, 0x14,0x00, 0xDA,0x3B, 0xD5,0x07
.byte 0xD1,0x37, 0xEA,0x1C, 0x14,0x00, 0xDA,0x3B, 0xD5,0x38
.byte 0xD1,0x31, 0xE1,0x01, 0xA4,0x00, 0xDA,0x07, 0x12,0x00, 0xE1,0x02, 0xA4,0x00, 0xDA,0x38, 0x14,0x00, 0xDA,0x3B, 0xB7,0x00
.byte 0x00,0x00

.fill 1024 - ($ - hrmc_bytecode), 1, 0x90
hrmc_dest:
.zero (1024<<4)

/*

TODO: update code to have more locals on the stack
	make a way for other code to call into hrmc functions
		needed for callbacks
		maybe have it as a part of the stack frame setup op
TODO: make compile into a callable function
	ret instead of jmp hrmc_dest
	load rsi and rdi from stack +0x08 and +0x10
	jmp past the loading from stack for the entry point's first call to save on stack space
TODO: open window
	put pixels on the screen
	draw text
	make name table for core hrmc
	add meta data to hrmc functions
	make editor recognize the meta data and display the names of core hrmc and local names
can do array indexing in 1 func if move back patching from -9 to -11
TODO: fix these up and put in 5x
nop; pop rax; mov rcx, [rbx+0x76543210]; movzx rax, word ptr [rax*2+rcx]; push rax;
nop; pop rax; mov rcx, [rbx+0x76543210]; movsx rax, word ptr [rax*2+rcx]; push rax;
nop; pop rax; mov rcx, [rbx+0x76543210]; mov eax, [rax*4+rcx]; push rax;
nop; pop rax; mov rcx, [rbx+0x76543210]; movsx rax, dword ptr [rax*4+rcx]; push rax;
nop; pop rax; mov rcx, [rbx+0x76543210]; mov rax, [rax*8+rcx]; push rax;

range kinds of machine code contained within range - mnemonic reasoning
00-0F are for meta data and common global pointers - kinda like a kernel's vector table
10-1F are for local variables that you want a 0-F name for instead of using the stack
20-2F can be conversions
30-3F multiply-add funcs for easy array of struct access
40-4F 4-wide 32-bit floating point vector operations
50-5F instructions interacting with the Stack - 5 is like S
60-6F load instructions - '6' is like '(' with a loop-in, straighten the loop-in to (- and angle the bow <- now it's an arrow
70-7F tag/label instructions for fixing relative branch offsets - 7 is like T
80-8F 8-wide 32-bit floating point vector operations
90-9F store instructions - same mnemonic reasoning as 60-6F but -> as in -> mem for store to memory
A0-AF 64-bit integer Arithmetic and bitwise operations
B0-BF instructions related to Branching and complying with ABI calling convention stuff
C0-CF both integer and floating point Comparison operations
D0-DF instructions interacting with the lookup table - uses table entries as Data, hence D
E0-EF instructions that use immediate values - E for EEEEmmediate - or as a rotated m for 2 m's in immediate
F0-FF 32-bit Floating point operations

the 2nd hexadecimal digit of the first byte follows some patterns
0/1/2/3/4/5/8/F are for u8/i8/u16/i16/u32/i32/u64/f32 types
ABED are + - * and /     B for suB and E for mul cuz E is like a rotated m
D could be for double for f64 but idk do we really need doubles?
6 and 9 are load and store
7 for addr-of
C for call/misc

E6 for load imm
E9 for load gs reg with offset (cuz store to imm doesn't make sense)
E7 for return imm
EC for ?

ET for +@T
DT for *+@T
5T for *+@T

accessing local/global primitive type arrays is easy with DT and 5T
accessing struct members is easy with ET
acessing array of structs.. how do
	index * sizeof(S) + arr + member_offset @T
	maybe a whole 'nother category for *sizeof(S)+
	but only if S is a power of 2
	idx *sizeof(S)+arr +@Tmem_off
	like 380F would be *(1<<8)+table[0F]

B0 for branch if tos is 0
B1-8 can be the pop abi regs
B9 for adjust abi regs for syscall (cuz linux syscalls abi is diff than linux lib abis)
BA for adjust stack pre-call
BB for back branch aka continue
BC for call addr on tos
BD for stack frame fixup on start of function  (TODO: see if there's a better spot/mnemonic for this)
BE for ret
BF for branch forward aka break









to self-compile
	do load main file from creb.c
		implement file_read and file_write
		call GetCommandLineA
	read bytes, skip whitespace, skipline on #
	file_write as name given on command line with .exe instead of .dmp

to optimize
	remove nops
	remove push rax; pop rax; and pop rax; push rax;
	reg alloc rsi,rdi,r8-r10
		either reg alloc top 10 local vars or first 10 local vars
		can say if an index is written to before reading it's local
	stuff like mov rcx, [mem]; add rax, rcx; can be just add rax, [mem];
	after shortening instructions, adjust rel jmp offsets
		maybe just keep a list of splice meta-info
		and patch branches from top to bottom
			keeping track of the current splice meta-info index and cur accumulated addr diff

3 steps to get 2 high level languages
	1) mc -> hrbc (simplest implementation as possible)
	2) hrmc -> pseudo judo (as expressive as possible)
	3) pseudo judo -> hral (most control while staying expressive - basically just compile the asm comments from step 1)


## textual hrmc (hrmc with a names-to-nums table)
	entries < 256 are direct aliases for hrmc
	>= 256 are user defined with 256 being main

main
	get_kernel32 =kernel32
	"GetStdHandle" kernel32 GetProcAddress =GetStdHandle
	"WriteFile" kernel32 GetProcAddress =WriteFile
	( 0 &bytes_written 12 "Hello World" GetStdHandle ) WriteFile
	return

get_kernel32
	GS:0x60 +@0x18 +@0x20 =node
	node +@0x20 =module_base
	{ module_base ?
		node +0x38 @u16 =module_name_len
		node +@0x40 =module_name
		module_name_len -2 =i
		22 =j
		{ j 0 >= ?
			module_name +i @u16 =char1
			u"KERNEL32.DLL" +j @u16 =char2
			{ char1 char2 != char1 char2 +0x20 != & ?
				break2
			}
			i -2 =i
			j -2 =j
			continue
		}
		{ j 0 < ?
			module_base return
		}
		node @ =node
		node +@0x20 =module_name
		continue
	}
	0 return

strcmp s1 s2
	{ s1 @u8 ? s2 @u8 ? s1 @u8 s2 @u8 == ?
		s1 +1 =s1 s2 +1 =s2
		continue
	}
	{ s1 @u8 s2 @u8 == ?
		0 return
	}
	s1 @u8 s2 @u8 - return

GetProcAddress module_base proc_name
	module_base +0x3C @u32 +module_base =pe_hdr
	pe_hdr +0x18 +0x70 @u32 +moudle_base =exports_table
	exports_table +0x20 @u32 +module_base = names_table
	0 =low
	exports_table +0x18 @i32 =high
	0 =index
	0 =comparison
	{
		{ comparison 0 > ? index =low }
		{ comparison 0 < ? index =high }
		high low + 1 >> =index
		index 2 << +names_table @u32 +module_base =cur_proc_name
		cur_proc_name proc_name strcmp =comparison
	comparison 0 != ? continue }
	{ index 2 << +names_table @u32 +module_base proc_name strcmp 0 != ? 0 return }
	exports_table +0x24 @u32 +module_base =ordinal_table
	exports_table +0x1C @u32 +module_base =export_address_table
	index 1 << +ordinal_table @u16 2 << +export_address_table @u32 +module_base return

alternative if we had +@ and *+@
GetProcAddress module_base proc_name
	module_base +@u32:0x3C +module_base =pe_hdr
	pe_hdr +0x18 +@u32:0x70 +moudle_base =exports_table
	exports_table +@u32:0x20 +module_base = names_table
	0 =low
	exports_table +@i32:0x18 =high
	0 =index
	0 =comparison
	{
		{ comparison 0 > ? index =low }
		{ comparison 0 < ? index =high }
		high low + 1 >> =index
		index *+@u32:names_table +module_base =cur_proc_name
		cur_proc_name proc_name strcmp =comparison
	comparison 0 != ? continue }
	{ index *+@u32:names_table +module_base proc_name strcmp 0 != ? 0 return }
	exports_table +@u32:0x24 +module_base =ordinal_table
	exports_table +@u32:0x1C +module_base =export_address_table
	index *+@u16:ordinal_table *+@u32:export_address_table +module_base return


## PseudoJudo (C with optional syntax and hrmc stuff built in)
	or maybe it's what I wrote in the hrmc compiler comments




*/
codesize = . - start
filesize = . - mzhdr
