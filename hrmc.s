##as -c -o hrmc.o hrmc.s && objcopy -O binary -j .text hrmc.o hrmc.exe && ./hrmc.exe; echo $?
##xxd -p hrmc.exe
##xxd hrmc.exe
##xxd dump.bin
##rm *.exe *.o *.obj
##objdump -M intel -d -j .text hrmc.exe
##objdump -M intel -x hrmc.exe
/* Human Readable Machine Code compiler for Windows x64
hrmc.s is a compiler for hrmc written in gnu assembler
TODOs:
make initial compile machine code callable
	ret instead of jmp right away to hrmc_dest
	load rsi and rdi from rsp+0x08 and rsp+0x10
self compile
	get file name from command line
	get file size
	infilebuf = alloc buffer of that size +1 for null
	outfilebuf = alloc buffer of that size +1 for null
	read file into inbuffer
	iterate through inbuffer
	if space, skip
	if #, line skip
	if hex pair, convert to byte and output to outbuffer
	write outbuffer to file with .exe at end
optimize
	remove nops
	remove redundant push/pops
	reg alloc
	adjust rel jmp offsets after or as we go
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
# if c & 0xF0 is 0x50 then u32[dest-9] = sign_ext(d) << 3
	cmp r10, 0x50
	jne end_of_stack_offset_patching
	movsx eax, dl
	shl eax, 3
	mov [rdi-9], eax
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
.zero 16*1                                                                                                                 # 00
.zero 16*1                                                                                                                 # 01
.zero 16*1                                                                                                                 # 02 get_kernel32
.byte 'K', 0,   'E', 0,    'R', 0,   'N', 0,    'E', 0,   'L', 0,    '3', 0,   '2', 0                                      # 03 kernel32 unicode string
.byte '.', 0,   'D', 0,    'L', 0,   'L', 0,    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00                                   # 04 kernel32 unicode string continued
.zero 16*1                                                                                                                 # 05
.asciz "GetStdHandle"; .align 16,0                                                                                         # 06 "GetStdHandle"
.zero 16*1                                                                                                                 # 07
.zero 16*1                                                                                                                 # 08
.zero 16*1                                                                                                                 # 09
.zero 16*1                                                                                                                 # 0A GetProcAddress
.zero 16*1                                                                                                                 # 0B
.zero 16*1                                                                                                                 # 0C strcmp
.zero 16*1                                                                                                                 # 0D
.zero 16*1                                                                                                                 # 0E main
.asciz "WriteFile"; .align 16,0                                                                                            # 0F "WriteFile"
.zero 16*16                                                                                                                # 10-1F
.zero 16*16                                                                                                                # 20-2F
.zero 16*16                                                                                                                # 30-3F
.zero 16*16                                                                                                                # 40-4F
.byte 0x90,0x90,0x90;pop rax; add rax, [rbp+0x76543210]; movzx rax, byte ptr [rax]; push rax;                              # 50 *+@u8:local
.byte 0x90,0x90,0x90;pop rax; add rax, [rbp+0x76543210]; movsx rax, byte ptr [rax]; push rax;                              # 51 *+@i8:local
pop rax; shl rax, 1; add rax, [rbp+0x76543210]; movzx rax, word ptr [rax]; push rax;                                       # 52 *+@u16:local
pop rax; shl rax, 1; add rax, [rbp+0x76543210]; movsx rax, word ptr [rax]; push rax;                                       # 53 *+@i16:local
pop rax; shl eax, 2; add rax, [rbp+0x76543210]; mov eax,   dword ptr [rax]; push rax;.byte 0x90,0x90;                      # 54 *+@u32:local
pop rax; shl eax, 2; add rax, [rbp+0x76543210]; movsx rax, dword ptr [rax]; push rax;.byte 0x90;                           # 55 *+@i32:local
.byte 0x90,0x90,0x90,0x90; mov rax, QWORD PTR [rbp+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90;                       # 56 @local
.byte 0x90,0x90,0x90,0x90;lea rax, qword ptr [rbp+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90;                        # 57 &local
pop rax; shl eax, 3; add rax, [rbp+0x76543210]; mov rax,   qword ptr [rax]; push rax;.byte 0x90;                           # 58 *+@u64:local
.byte 0x90,0x90,0x90; pop rax; mov QWORD PTR[rbp+0x76543210],rax; .byte 0x90, 0x90,0x90,0x90,0x90;                         # 59 =local
.byte 0x90,0x90,0x90,0x90; mov rcx, QWORD PTR [rbp+0x76543210]; pop rax; add rax, rcx; push rax                            # 5A +local
.byte 0x90,0x90,0x90,0x90; mov rcx, QWORD PTR [rbp+0x76543210]; pop rax; sub rax, rcx; push rax                            # 5B -local
.byte 0x90,0x90,0x90,0x90; mov rax, [rbp+0x76543210]; call rax; push rax; .byte 0x90,0x90;                                 # 5C call local
.zero 16*1                                                                                                                 # 5D
.byte 0x90,0x90,0x90,0x90;sub rsp, 0x76543210; .byte 0x90,0x90,0x90,0x90,0x90;                                             # 5E reserve stack space
.zero 16*1                                                                                                                 # 5F
pop rax; movzx rax, BYTE PTR [rax]; push rax   ; .align 16, 0x90                                                           # 60 @u8
pop rax; movsx rax, BYTE PTR [rax]; push rax   ; .align 16, 0x90                                                           # 61 @i8
pop rax; movzx rax, WORD PTR [rax]; push rax   ; .align 16, 0x90                                                           # 62 @u16
pop rax; movsx rax, WORD PTR [rax]; push rax   ; .align 16, 0x90                                                           # 63 @i16
pop rax; mov eax, DWORD PTR [rax]; push rax    ; .align 16, 0x90                                                           # 64 @u32
pop rax; movsxd rax, DWORD PTR [rax]; push rax ; .align 16, 0x90                                                           # 65 @i32
.zero 16*1                                                                                                                 # 66
.zero 16*1                                                                                                                 # 67
pop rax; mov rax, QWORD PTR [rax]; push rax ; .align 16, 0x90                                                              # 68 @u64
pop rax; mov rax, QWORD PTR [rax]; push rax ; .align 16, 0x90                                                              # 69 @i64
.zero 16*1                                                                                                                 # 6A
.zero 16*1                                                                                                                 # 6B
.zero 16*1                                                                                                                 # 6C
.zero 16*1                                                                                                                 # 6D
.zero 16*1                                                                                                                 # 6E
.zero 16*1                                                                                                                 # 6F
.zero 16*14                                                                                                                # 70-7D
or al,al ; .align 16, 0x90                                                                                                 # 7E {
or rax,rax ; .align 16, 0x90                                                                                               # 7F }
.zero 16*16                                                                                                                # 80-8F
pop rax; mov BYTE PTR[rax],al ;   .align 16, 0x90                                                                          # 90 =u8
pop rax; mov BYTE PTR[rax],al ;   .align 16, 0x90                                                                          # 91 =i8
pop rax; mov WORD PTR[rax],ax ;   .align 16, 0x90                                                                          # 92 =u16
pop rax; mov WORD PTR[rax],ax ;   .align 16, 0x90                                                                          # 93 =i16
pop rax; mov DWORD PTR[rax],eax ; .align 16, 0x90                                                                          # 94 =u32
pop rax; mov DWORD PTR[rax],eax ; .align 16, 0x90                                                                          # 95 =i32
.zero 16*1                                                                                                                 # 96
.zero 16*1                                                                                                                 # 97
pop rax; mov QWORD PTR[rax],rax ; .align 16, 0x90                                                                          # 98 =u64
pop rax; mov QWORD PTR[rax],rax ; .align 16, 0x90                                                                          # 99 =i64
.zero 16*1                                                                                                                 # 9A
.zero 16*1                                                                                                                 # 9B
.zero 16*1                                                                                                                 # 9C
.zero 16*1                                                                                                                 # 9D
.zero 16*1                                                                                                                 # 9E
.zero 16*1                                                                                                                 # 9F
pop rcx; pop rax;      or rax, rcx; push rax ; .align 16, 0x90                                                             # A0 or
pop rax;               not rax; push rax; ; .align 16, 0x90                                                                # A1 not
cqo; pop rcx; pop rax; div rax, rcx; push rax ; .align 16, 0x90                                                            # A2 udiv
pop rcx; pop rax;      mul rcx; push rdx ; .align 16, 0x90                                                                 # A3 upper umul
pop rcx; pop rax;      shl rax, rcx; push rax ; .align 16, 0x90                                                            # A4 shl
pop rax;               neg rax; push rax; ; .align 16, 0x90                                                                # A5 neg
cqo; pop rcx; pop rax; div rax, rcx; push rdx ; .align 16, 0x90                                                            # A6 umod
pop rcx; pop rax;      shr rax, rcx; push rax ; .align 16, 0x90                                                            # A7 shr
pop rcx; pop rax;      xor rax, rcx; push rax ; .align 16, 0x90                                                            # A8 xor
pop rcx; pop rax;      sar rax, rcx; push rax ; .align 16, 0x90                                                            # A9 sar
pop rcx; pop rax;      add rax, rcx; push rax ; .align 16, 0x90                                                            # AA add
pop rcx; pop rax;      sub rax, rcx; push rax ; .align 16, 0x90                                                            # AB sub
cqo; pop rcx; pop rax; idiv rax, rcx; push rdx ; .align 16, 0x90                                                           # AC mod
cqo; pop rcx; pop rax; idiv rax, rcx; push rax ; .align 16, 0x90                                                           # AD div
pop rcx; pop rax;      imul rcx; push rax ; .align 16, 0x90                                                                # AE mul
pop rcx; pop rax;      and rax, rcx; push rax ; .align 16, 0x90                                                            # AF and
.byte 0x90,0x90, 0x90,0x90,0x90,0x90;pop rax; test rax, rax; je mzhdr;                                                     # B0 ?
pop rcx;                          .align 16, 0x90                                                                          # B1 pop 1 abi reg
pop rcx; pop rdx;                 .align 16, 0x90                                                                          # B2 pop 2 abi regs
pop rcx; pop rdx; pop r8;         .align 16, 0x90                                                                          # B3 pop 3 abi regs
pop rcx; pop rdx; pop r8; pop r9; sub rsp, 0x20; .align 16, 0x90                                                           # B4 pop 4 abi regs; rsp-20 is for shadow space
.zero 16*1                                                                                                                 # B5 unused in windows, used in linux
.zero 16*1                                                                                                                 # B6 unused in windows, used in linux
.zero 16*1                                                                                                                 # B7 unused in windows, used in linux
.zero 16*1                                                                                                                 # B8 unused in windows, used in linux
.zero 16*1                                                                                                                 # B9 adjust for non-syscall abi - unused in windows, used in linux
and rsp, -16; .align 16, 0x90                                                                                              # BA stack must be 16 byte aligned on function entry
.byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90, 0x90,0x90,0x90;jmp mzhdr;                                                  # BB backward (continue)
pop rax; call rax; push rax; .align 16, 0x90                                                                               # BC call procedure
push rbp; mov rbp, rsp; push rcx; push rdx; push r8; push r9; .align 16, 0x90                                              # BD deploy stack frame
pop rax; mov rsp, rbp; pop rbp; ret ; .align 16, 0x90                                                                      # BE return
.byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90, 0x90,0x90,0x90;jmp mzhdr;                                                  # BF forward (break)
.zero 16*1                                                                                                                 # C0
pop rcx; pop rax; cmp rax,rcx; setne al; movsx rax, al ; push rax; .align 16, 0x90                                         # C1 !=
pop rcx; pop rax; cmp rax,rcx; setbe al; movsx rax, al ; push rax; .align 16, 0x90                                         # C2 <=u
pop rcx; pop rax; cmp rax,rcx; setae al; movsx rax, al ; push rax; .align 16, 0x90                                         # C3 >=u
pop rcx; pop rax; cmp rax,rcx; setl al; movsx rax, al ; push rax; .align 16, 0x90                                          # C4 <
.zero 16*1                                                                                                                 # C5
pop rcx; pop rax; cmp rax,rcx; setle al; movsx rax, al ; push rax; .align 16, 0x90                                         # C6 <=
pop rcx; pop rax; cmp rax,rcx; setg al; movsx rax, al ; push rax; .align 16, 0x90                                          # C7 >
.zero 16*1                                                                                                                 # C8
pop rcx; pop rax; cmp rax,rcx; setge al; movsx rax, al ; push rax; .align 16, 0x90                                         # C9 >=
pop rcx; pop rax; cmp rax,rcx; seta al; movsx rax, al ; push rax; .align 16, 0x90                                          # CA >u
pop rcx; pop rax; cmp rax,rcx; setb al; movsx rax, al ; push rax; .align 16, 0x90                                          # CB <u
.zero 16*1                                                                                                                 # CC
.zero 16*1                                                                                                                 # CD
pop rcx; pop rax; cmp rax,rcx; sete al; movsx rax, al ; push rax; .align 16, 0x90                                          # CE ==
.zero 16*1                                                                                                                 # CF
.byte 0x90,0x90,0x90;pop rax; add rax, [rbx+0x76543210]; movzx rax, byte ptr [rax]; push rax;                              # D0 *+@u8:global
.byte 0x90,0x90,0x90;pop rax; add rax, [rbx+0x76543210]; movsx rax, byte ptr [rax]; push rax;                              # D1 *+@i8:global
pop rax; shl rax, 1; add rax, [rbx+0x76543210]; movzx rax, word ptr [rax]; push rax;                                       # D2 *+@u16:global
pop rax; shl rax, 1; add rax, [rbx+0x76543210]; movsx rax, word ptr [rax]; push rax;                                       # D3 *+@i16:global
pop rax; shl eax, 2; add rax, [rbx+0x76543210]; mov eax,   dword ptr [rax]; push rax;.byte 0x90,0x90;                      # D4 *+@u32:global
pop rax; shl eax, 2; add rax, [rbx+0x76543210]; movsx rax, dword ptr [rax]; push rax;.byte 0x90;                           # D5 *+@i32:global
.byte 0x90,0x90,0x90,0x90; mov rax, QWORD PTR [rbx+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90;                       # D6 @global
.byte 0x90,0x90,0x90,0x90;lea rax, qword ptr [rbx+0x76543210]; push rax; .byte 0x90,0x90,0x90,0x90;                        # D7 &global
pop rax; shl eax, 3; add rax, [rbx+0x76543210]; mov rax,   qword ptr [rax]; push rax;.byte 0x90;                           # D8 *+@u64:global
.byte 0x90,0x90,0x90; pop rax; mov QWORD PTR[rbx+0x76543210],rax; .byte 0x90, 0x90,0x90,0x90,0x90;                         # D9 =global
.byte 0x90,0x90,0x90,0x90; mov rcx, QWORD PTR [rbx+0x76543210]; pop rax; add rax, rcx; push rax                            # DA +global
.byte 0x90,0x90,0x90,0x90; mov rcx, QWORD PTR [rbx+0x76543210]; pop rax; sub rax, rcx; push rax                            # DB -global
.byte 0x90,0x90,0x90,0x90; mov rax, [rbx+0x76543210]; call rax; push rax; .byte 0x90,0x90;                                 # DC call global
.zero 16*1                                                                                                                 # DD
.zero 16*1                                                                                                                 # DE
.zero 16*1                                                                                                                 # DF
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; movzx rax, byte ptr [rax]; push rax;                         # E0 +@u8:imm  
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; movsx rax, byte ptr [rax]; push rax;                         # E1 +@i8:imm  
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; movzx rax, word ptr [rax]; push rax;                         # E2 +@u16:imm 
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; movsx rax, word ptr [rax]; push rax;                         # E3 +@i16:imm 
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; mov   eax, dword ptr [rax]; push rax;.byte 0x90,0x90;        # E4 +@u32:imm 
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; movsx rax, dword ptr [rax]; push rax;.byte 0x90;             # E5 +@i32:imm 
.byte 0x90,0x90,0x90,0x90,0x90,0x90;push 0x2a; .byte 0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90;                             # E6 load imm
.byte 0x90,0x90,0x90,0x90,0x90,0x90;push 42; pop rax; mov rsp, rbp; pop rbp; ret;.byte 0x90,0x90;                          # E7 ret imm
.byte 0x90,0x90,0x90,0x90,0x58,0x48,0x05,0x2a,0x00,0x00,0x00; mov   rax, qword ptr [rax]; push rax;.byte 0x90;             # E8 +@u64:imm
.byte 0x90,0x90; mov rax,QWORD PTR gs:0x60; push rax; .byte 0x90,0x90,0x90,0x90;                                           # E9 gs:imm - used in get_kernel32()
.byte 0x90,0x90,0x90; pop rax; add rax, 42; push rax; .byte 0x90,0x90,0x90,0x90,0x90,0x90,0x90;                            # EA +imm
.byte 0x90,0x90,0x90; pop rax; sub rax, 42; push rax; .byte 0x90,0x90,0x90,0x90,0x90,0x90,0x90;                            # EB -imm
.zero 16*1                                                                                                                 # EC
.zero 16*1                                                                                                                 # ED
.byte 0x90,0x90,0x90,0x90; mov rax, [rbp+42]; push rax; .byte 0x90,0x90,0x90, 0x90,0x90,0x90,0x90;                         # EE
.zero 16*1                                                                                                                 # EF
.zero 16*16                                                                                                                # F0-FF

hrmc_bytecode:
## int main()
.byte 0x7E,0x0E, 0xBD,0x00 # 0E=&main; setup stack frame
.byte 0xDC,0x02, 0x59,0xFF #, 0xD9,0x0B # get_kernel32() =$FF
.byte 0xD7,0x06, 0x56,0xFF, 0xDC,0x0A, 0x59,0xFE # GetProcAddress(kernel32, "GetStdHandle") =$FE
.byte 0xD7,0x0F, 0x56,0xFF, 0xDC,0x0A, 0x59,0xFD # GetProcAddress(kernel32, "WriteFile") =$FD
.byte 0xE6,0xF5, 0xB1,0x00, 0x5C,0xFE, 0x59,0xFC # GetStdHandle(STD_OUTPUT_HANDLE) =$FC
.byte 0xBA,0x00, 0xE6,0x00, 0xD7,0x01, 0xE6,0x0C, 0xD7,0x06, 0x56,0xFC, 0xB4,0x00, 0x5C,0xFD # WriteFile(fout,msg,size,&written,0)
.byte 0xBE,0x00 # return

## void* get_kernel32()
.byte 0x7E,0x02, 0xBD,0x00 # 02=&get_kernel32; setup stack frame
.byte 0xE9,0x60, 0xE8,0x18, 0xE8,0x20, 0xD9,0x1D # @gs:60 +@u64:0x18 +@u64:0x20 =1D
.byte 0xD6,0x1D, 0xE8,0x20, 0xD9,0x1B # @1D +@u64:0x20 =1B
.byte 0x7E,0x71, 0xD6,0x1B, 0xB0,0x71 # @1B ?
     .byte 0xD6,0x1D, 0xE2,0x38, 0xD9,0x11 # @1D +@u16:0x38 =11
     .byte 0xD6,0x1D, 0xE8,0x40, 0xD9,0x1A # @1D +@u64:0x40 =1A
     .byte 0xD6,0x11, 0xEB,0x02, 0xD9,0x11 # @11 -2 =11
     .byte 0xE6,0x16, 0xD9,0x12 # 22 =12
     .byte 0x7E,0x72, 0xD6,0x12, 0xE6,0x00, 0xC9,0x00, 0xB0,0x72 # @12 0 >= ?
          .byte 0xD6,0x1A, 0xDA,0x11, 0x62,0x00, 0xD9,0x13 # @1A +@11 @u16 =13
          .byte 0xD7,0x03, 0xDA,0x12, 0x62,0x00, 0xD9,0x14 # &03 +@12 @u16 =14
          .byte 0x7E,0x73, 0xD6,0x13, 0xD6,0x14, 0xC1,0x00, 0xD6,0x13, 0xD6,0x14, 0xEA,0x20, 0xC1,0x00, 0xAF,0x00, 0xB0,0x73 # @13 @14 != @13 @14 +0x20 != & ?
               .byte 0xBF,0x72 # break
          .byte 0x7F,0x73
          .byte 0xD6,0x11, 0xEB,0x02, 0xD9,0x11 # @11 -2 =11
          .byte 0xD6,0x12, 0xEB,0x02, 0xD9,0x12 # @12 -2 =12
          .byte 0xBB,0x72 # continue
     .byte 0x7F,0x72
     .byte 0x7E,0x72, 0xD6,0x12, 0xE6,0x00, 0xC4,0x00, 0xB0,0x72 # @12 0 < ?
          .byte 0xD6,0x1B, 0xBE,0x00 # @1B return
     .byte 0x7F,0x72
     .byte 0xD6,0x1D, 0x68,0x00, 0xD9,0x1D # @1D @u64 =1D
     .byte 0xD6,0x1D, 0xE8,0x20, 0xD9,0x1B # @1D +@u64:0x20 =1B
     .byte 0xBB,0x71 # continue
.byte 0x7F,0x71
.byte 0xE7,0x00 # return0

## int strcmp(char* s1, char* s2)
.byte 0x7E,0x0C, 0xBD,0x00 # 0C=&strcmp; setup stack frame
.byte 0x7E,0x71, 0x56,0x02, 0x60,0x00, 0xB0,0x71, 0x56,0x03, 0x60,0x00, 0xB0,0x71, 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xCE,0x00, 0xB0,0x71 # $02 @u8 ? $03 @u8 ? $02 @u8 $03 @u8 == ?
     .byte 0x56,0x02, 0xEA,0x01, 0x59,0x02, 0x56,0x03, 0xEA,0x01, 0x59,0x03 # $02 +1 =$02 $03 +1 =$03
     .byte 0xBB,0x71 # continue
.byte 0x7F,0x71
.byte 0x7E,0x71, 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xCE,0x00, 0xB0,0x71 # $02 @u8 $03 @u8 == ?
     .byte 0xE7,0x00 # return0
.byte 0x7F,0x71
.byte 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xAB,0x00, 0xBE,0x00 # $02 @u8 $03 @u8 - return

## void* GetProcAddress(void* handle, char* proc_name)
.byte 0x7E,0x0A, 0xBD,0x00 # 0A=&GetProcAddress; setup stack frame
.byte 0x56,0x02, 0xE4,0x3C, 0x5A,0x02, 0xD9,0x1D # $02 +@u32:0x3C +$02 =1D
.byte 0xD6,0x1D, 0xEA,0x18, 0xE4,0x70, 0x5A,0x02, 0xD9,0x1E # @1D +0x18 +@u32:0x70 +$02 =1E
.byte 0xD6,0x1E, 0xE4,0x20, 0x5A,0x02, 0xD9,0x1A # @1E +@u32:0x20 +$02 =1A
.byte 0xE6,0x00, 0xD9,0x10 # 0 =10
.byte 0xD6,0x1E, 0xE5,0x18, 0xD9,0x1F # @1E +@i32:0x18 =1F
.byte 0xE6,0x00, 0xD9,0x11 # 0 =11
.byte 0xE6,0x00, 0xD9,0x1C # 0 =1C
.byte 0x7E,0x71 # do {
     .byte 0x7E,0x72, 0xD6,0x1C, 0xE6,0x00, 0xC7,0x00, 0xB0,0x72, 0xD6,0x11, 0xD9,0x10, 0x7F,0x72 # @1C 0 > ? @11 =10
     .byte 0x7E,0x72, 0xD6,0x1C, 0xE6,0x00, 0xC4,0x00, 0xB0,0x72, 0xD6,0x11, 0xD9,0x1F, 0x7F,0x72 # @1C 0 < ? @11 =1F
     .byte 0xD6,0x1F, 0xDA,0x10, 0xE6,0x01, 0xA9,0x00, 0xD9,0x11 # @1F @10 + 1 >> =11
     .byte 0xD6,0x11, 0xD4,0x1A, 0x5A,0x02, 0xD9,0x12 # @11 *+@u32:1A +$02 =12
     .byte 0xD6,0x12, 0x56,0x03, 0xDC,0x0C, 0xD9,0x1C # @12 $03 strcmp =1C
.byte 0xD6,0x1C, 0xE6,0x00, 0xC1,0x00, 0xB0,0x71, 0xBB,0x71, 0x7F,0x71 # @1C 0 != ? continue }
.byte 0x7E,0x71, 0xD6,0x11, 0xD4,0x1A, 0x5A,0x02, 0x56,0x03, 0xDC,0x0C, 0xE6,0x00, 0xC1,0x00, 0xB0,0x71, 0xE7,0x00, 0x7F,0x71 # index *+@u32:1A +$02 $03 strcmp 0 != ? return0
.byte 0xD6,0x1E, 0xE4,0x24, 0x5A,0x02, 0xD9,0x07 # @1E +@u32:0x24 +$02 =17
.byte 0xD6,0x1E, 0xE4,0x1C, 0x5A,0x02, 0xD9,0x18 # @1E +@u32:0x1C +$02 =18
.byte 0xD6,0x11, 0xD2,0x07, 0xD4,0x18, 0x5A,0x02, 0xBE,0x00 # @11 *+@u16:17 *+@u32:18 +$02 return
.byte 0x00,0x00 # end compilation

.fill 1024 - ($ - hrmc_bytecode), 1, 0x90
hrmc_dest:
.zero (1024<<4)

codesize = . - start
filesize = . - mzhdr
