##as -c -o hrmc.o hrmc.s && objcopy -O binary -j .text hrmc.o hrmc.exe && ./hrmc.exe; echo $?
##mv hrmc.out hrmc.exe && ./hrmc.exe
##xxd -p hrmc.exe
##xxd hrmc.exe
##xxd dump.bin
##rm *.exe *.o *.obj
##objdump -M intel -d -j .text hrmc.exe
##objdump -M intel -x hrmc.exe
/* hrmc.s is a compiler for hrmc to Windows x64 written in GNU assembly
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
	lea rcx, [rip+hrmc_table]
	lea rdx, [rip+hrmc_bytecode]
	lea r8, [rip+hrmc_dest]
	call compile
	jmp hrmc_dest

## hrmc compiler v4
compile:
	mov rbx, rcx                # rbx = &hrmc_table
	mov rsi, rdx                # rsi = &hrmc_bytecode
	mov rdi, r8                 # rdi = &hrmc_dest
compile_loop:
	movzx ecx, byte ptr [rsi]   # c = u8[rsi] (c for code)
	movzx edx, byte ptr [rsi+1] # d = u8[rsi + 1] (d for data)
	add rsi, 2                  # rsi += 2
	test eax, eax               # if c is 0 then finish compiling
	jne continue_compiling
	ret
continue_compiling:
	mov r8, rcx                 # r8 = c & 0xF0
	and r8, 0xF0
	mov r9, rdx                 # r9 = d << 4
	shl r9, 4
	mov eax, ecx                # u128[rdi] = u128[hrmc_table + (c << 4)]
	shl eax, 4
	movdqa xmm0, [rbx+rax]
	movdqa [rdi], xmm0
	add rdi, 16                 # rdi += 16

	cmp cl, 0xDE                # if c is 0xDE then u32[dest-4] = hrmc_table - dest
	jne end_of_table_restoration
	mov rax, rbx
	sub rax, rdi
	mov [rdi-4], eax
	jmp compile_loop
end_of_table_restoration:
	cmp cl, 0xBB                # if c is 0xBB then u32[dest-4] = u64[hrmc_table + (d << 4)] - dest
	jne end_of_branch_backwards_patching
	mov rax, [rbx + r9]
	sub rax, rdi
	mov [rdi-4], eax
	jmp compile_loop
end_of_branch_backwards_patching:
	cmp r8, 0xD0                # if c & 0xF0 is 0xD0 then u32[dest-9] = d << 4
	jne end_of_data_table_addr_patching
	mov [rdi-9], r9d
	jmp compile_loop
end_of_data_table_addr_patching:
	cmp r8, 0xE0                # if c & 0xF0 is 0xE0 then u8[dest-9] = d
	jne end_of_const_imm_patching
	mov [rdi-9], dl
	jmp compile_loop
end_of_const_imm_patching:
	cmp r8, 0x50                # if c & 0xF0 is 0x50 then u32[dest-9] = sign_ext(d) << 3
	jne end_of_stack_offset_patching
	movsx eax, dl
	shl eax, 3
	mov [rdi-9], eax
	jmp compile_loop
end_of_stack_offset_patching:

	cmp r8, 0x70                # if c & 0xF0 is not 0x70 then continue compile_loop
	jne compile_loop
	cmp cl, 0x7E                # if c is 0x7E then u64[hrmc_table + (d << 4)] = dest
	jne end_of_tag_enscribing
	mov [rbx + r9], rdi
end_of_tag_enscribing:
	cmp cl, 0x7F                # if c is not 0x7F then continue compile_loop
	jne compile_loop
	lea r11, [rsi-2] # src_iter
	mov r12, rdi # dest_iter
	mov r13, rdx # target_d
fixup_branch_forwards_loop:
	sub r11, 2                  # src_iter -= 2
	mov cl, [r11]               # c = [src_iter]
	mov dl, [r11+1]             # d = [src_iter+1]
	sub r12, 16                 # dest_iter -= 16
	cmp rdx, r13                # if d != target_d then continue fixup_branch_forwards_loop
	jne fixup_branch_forwards_loop
	cmp cl, 0xBF                # if c == 0xBF or c == 0xB0 then u32[dest_it-4] = dest - dest_it
	je fixup
	cmp cl, 0xB0
	jne end_of_fixup
fixup:
	mov rax, rdi
	sub rax, r12
	mov [r12-4], eax
end_of_fixup:
	cmp cl, 0x7E                # if c is not 7E then continue fixup_branch_forwards_loop
	jne fixup_branch_forwards_loop
	jmp compile_loop

.align 16, 0
hrmc_table:
.zero 16*1                                                                                                                 # 00
.asciz "CloseHandle"; .align 16,0                                                                                          # 01 "CloseHandle" then actual CloseHandle
.zero 16*1                                                                                                                 # 02 get_kernel32 then actual kernel32
.byte 'K', 0,   'E', 0,    'R', 0,   'N', 0,    'E', 0,   'L', 0,    '3', 0,   '2', 0                                      # 03 KERNEL32.DLL unicode string
.byte '.', 0,   'D', 0,    'L', 0,   'L', 0,    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00                                   # 04 KERNEL32.DLL unicode string continued
.zero 16*1                                                                                                                 # 05 strcmp
.asciz "GetStdHandle"; .align 16,0                                                                                         # 06 "GetStdHandle" then actual GetStdHandle
.asciz "VirtualAlloc"; .align 16,0                                                                                         # 07 "VirtualAlloc" then actual VirtualAlloc
.asciz "GetFileAttributesExA"; .align 16,0                                                                                 # 08 "GetFileAttributesExA" then actual GetFileAttributesExA
#.asciz "GetFileAttributesExA"; .align 16,0                                                                                # 09 "GetFileAttributesExA" continued
.zero 16*1                                                                                                                 # 0A GetProcAddress
.zero 16*1                                                                                                                 # 0B memset
.asciz "CreateFileA"; .align 16,0                                                                                          # 0C "CreateFileA" then actual CreateFileA
.asciz "hrmc.dmp"; .align 16,0                                                                                             # 0D "hrmc.dmp" then contents of hrmc.dmp
.asciz "ReadFile"; .align 16,0                                                                                             # 0E "ReadFile" then actual ReadFile
.asciz "WriteFile"; .align 16,0                                                                                            # 0F "WriteFile" then actual WriteFile
.asciz "LoadLibraryA"; .align 16,0                                                                                         # 10 "LoadLibraryA" then actual LoadLibraryA
.asciz "user32.dll"; .align 16,0                                                                                           # 11 "user32.dll" then actual user32
.asciz "RegisterClassA"; .align 16,0                                                                                       # 12 "RegisterClassA" then actual RegisterClassA
.asciz "CreateWindowExA"; .align 16,0                                                                                      # 13 "CreateWindowExA" then actual CreateWindowExA
.asciz "PeekMessageA"; .align 16,0                                                                                         # 14 "PeekMessageA" then actual PeekMessageA
.asciz "PostQuitMessage"; .align 16,0                                                                                      # 15 "PostQuitMessage" then actual PostQuitMessage
.asciz "DefWindowProcA"; .align 16,0                                                                                       # 16 "DefWindowProcA" then actual DefWindowProcA
.ascii "TranslateMessage";                                                                                                 # 17 "TranslateMessage" then actual TranslateMessage
.zero 16*1                                                                                                                 # 18
.long 0x10cf0000; .align 16,0                                                                                              # 19 WS_OVERLAPPEDWINDOW|WS_VISIBLE
.ascii "GetModuleHandleA";                                                                                                 # 1A "GetModuleHandleA" then actual GetModuleHandleA
.zero 16*1                                                                                                                 # 1B
.zero 16*1                                                                                                                 # 1C
.ascii "DispatchMessageA";                                                                                                 # 1D "DispatchMessageA" then actual DispatchMessageA
.zero 16*1                                                                                                                 # 1E
.zero 16*1                                                                                                                 # 1F
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
pop rcx; pop rax; mov BYTE PTR[rcx],al ;   .align 16, 0x90                                                                 # 90 =u8
pop rcx; pop rax; mov BYTE PTR[rcx],al ;   .align 16, 0x90                                                                 # 91 =i8
pop rcx; pop rax; mov WORD PTR[rcx],ax ;   .align 16, 0x90                                                                 # 92 =u16
pop rcx; pop rax; mov WORD PTR[rcx],ax ;   .align 16, 0x90                                                                 # 93 =i16
pop rcx; pop rax; mov DWORD PTR[rcx],eax ; .align 16, 0x90                                                                 # 94 =u32
pop rcx; pop rax; mov DWORD PTR[rcx],eax ; .align 16, 0x90                                                                 # 95 =i32
.zero 16*1                                                                                                                 # 96
.zero 16*1                                                                                                                 # 97
pop rcx; pop rax; mov QWORD PTR[rcx],rax ; .align 16, 0x90                                                                 # 98 =u64
pop rcx; pop rax; mov QWORD PTR[rcx],rax ; .align 16, 0x90                                                                 # 99 =i64
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
pop rcx;                          sub rsp, 0x20; .align 16, 0x90                                                           # B1 pop 1 abi reg
pop rcx; pop rdx;                 sub rsp, 0x20; .align 16, 0x90                                                           # B2 pop 2 abi regs
pop rcx; pop rdx; pop r8;         sub rsp, 0x20; .align 16, 0x90                                                           # B3 pop 3 abi regs
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
.byte 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90; lea rbx, [rip+0x76543210];                                             # DE restore table ptr
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
.byte 0x90,0x90,0x90; pop rax; shl rax, 42; push rax; .byte 0x90,0x90,0x90,0x90,0x90,0x90,0x90;                            # EC <<imm
.zero 16*1                                                                                                                 # ED
pop rax; shl rax, 8; .byte 0x48,0x0d,0x2a,0x0,0x0,0x0, 0x50,0x90,0x90,0x90,0x90;                                           # EE <<|imm
.zero 16*1                                                                                                                 # EF
.zero 16*16                                                                                                                # F0-FF

hrmc_bytecode:
## int main()
.byte 0xBD,0x00, 0x5E,0x10 # setup stack frame
.byte 0xDC,0x02, 0xD9,0x02 # get_kernel32() =02
.byte 0xD7,0x06, 0xD6,0x02, 0xDC,0x0A, 0xD9,0x06 # GetProcAddress(kernel32, "GetStdHandle") =06
.byte 0xD7,0x0F, 0xD6,0x02, 0xDC,0x0A, 0xD9,0x0F # GetProcAddress(kernel32, "WriteFile") =0F
.byte 0xD7,0x0E, 0xD6,0x02, 0xDC,0x0A, 0xD9,0x0E # GetProcAddress(kernel32, "ReadFile") =0E
.byte 0xD7,0x0C, 0xD6,0x02, 0xDC,0x0A, 0xD9,0x0C # GetProcAddress(kernel32, "CreateFileA") =0C
.byte 0xD7,0x07, 0xD6,0x02, 0xDC,0x0A, 0xD9,0x07 # GetProcAddress(kernel32, "VirtualAlloc") =07
.byte 0xE6,0xF5, 0xB1,0x00, 0xDC,0x06, 0x59,0xFF # GetStdHandle(STD_OUTPUT_HANDLE) =$FF
.byte 0xBA,0x00, 0xE6,0x40, 0xE6,0x30, 0xE6,0x08, 0xA4,0x00, 0xE6,0x40, 0xE6,0x10, 0xA4,0x00, 0xE6,0x00, 0xB4,0x00, 0xDC,0x07, 0x59,0xFB # VirtualAlloc(0, 0x400000, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE) =$FB
.byte 0xBA,0x00, 0xE6,0x00, 0xE6,0x00, 0xE6,0x03, 0xE6,0x00, 0xE6,0x01, 0xE6,0x08, 0xE6,0x1C, 0xA4,0x00, 0xD7,0x0D, 0xB4,0x00, 0xDC,0x0C, 0x59,0xFD # CreateFileA("hrmc.dmp", GENERIC_READ, FILE_READ_SHARE, 0, OPEN_EXISTING, 0, 0) =$FD
.byte 0xBA,0x00, 0xE6,0x00, 0x57,0xFC, 0xE6,0x40, 0xE6,0x10, 0xA4,0x00, 0x56,0xFB, 0x56,0xFD, 0xB4,0x00, 0xDC,0x0E # ReadFile(fin, inbuf, insize, &bytes_read, 0)
.byte 0xBA,0x00, 0xE6,0x40, 0xE6,0x30, 0xE6,0x08, 0xA4,0x00, 0xE6,0x40, 0xE6,0x10, 0xA4,0x00, 0xE6,0x00, 0xB4,0x00, 0xDC,0x07, 0x59,0xF0 # VirtualAlloc(0, 0x400000, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE) =$F0
.byte 0xE6,0x00, 0x59,0xF1 # i=0
.byte 0xE6,0x00, 0x59,0xF2 # j=0
.byte 0x7E,0x71, 0x56,0xF1, 0x56,0xFC, 0xC4,0x00, 0xB0,0x71 # while i < bytes_read
.byte      0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x23, 0xCE,0x00, 0xB0,0x73 # i *+@u8:inbuf 23 == ? (aka skip # comments)
.byte           0x7E,0x74, 0x56,0xF1, 0xEA,0x01, 0x59,0xF1, 0x56,0xF1, 0x50,0xFB, 0xE6,0x0A, 0xC1,0x00, 0xB0,0x74, 0xBB,0x74, 0x7F,0x74 # do { i += 1 } while( i *+@u8:inbuf 0x10 != )
.byte      0x7F,0x73
.byte      0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x30, 0xC4,0x00, 0xB0,0x73, 0x56,0xF1, 0xEA,0x01, 0x59,0xF1, 0xBB,0x71, 0x7F,0x73 # i *+@u8:inbuf 0x30 < ? i += 1 ; continue (aka skip whitespace)
.byte 	 0x7E,0x72
.byte           0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x61, 0xC9,0x00, 0xB0,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x57, 0xAB,0x00, 0xE6,0x04, 0xA4,0x00, 0x59,0xF4, 0xBF,0x72, 0x7F,0x73 # i *+@u8:inbuf 61 >= ? i *+@u8:inbuf 51 - 4 << =$F4 ; break
.byte           0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x41, 0xC9,0x00, 0xB0,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x37, 0xAB,0x00, 0xE6,0x04, 0xA4,0x00, 0x59,0xF4, 0xBF,0x72, 0x7F,0x73 # i *+@u8:inbuf 41 >= ? i *+@u8:inbuf 37 - 4 << =$F4 ; break
.byte           0x56,0xF1, 0x50,0xFB, 0xE6,0x30, 0xAB,0x00, 0xE6,0x04, 0xA4,0x00, 0x59,0xF4 # i *+@u8:inbuf 30 - 4 << =$f4
.byte 	 0x7F,0x72
.byte      0x56,0xF1, 0xEA,0x01, 0x59,0xF1 # i += 1
.byte 	 0x7E,0x72
.byte           0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x61, 0xC9,0x00, 0xB0,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x57, 0xAB,0x00, 0x56,0xF4, 0xA0,0x00, 0x56,0xF0, 0x5A,0xF2, 0x90,0x00, 0xBF,0x72, 0x7F,0x73 # i *+@u8:inbuf 61 >= ? i *+@u8:inbuf 57 - $F4 | outbuf +j =u8 ; break
.byte           0x7E,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x41, 0xC9,0x00, 0xB0,0x73, 0x56,0xF1, 0x50,0xFB, 0xE6,0x37, 0xAB,0x00, 0x56,0xF4, 0xA0,0x00, 0x56,0xF0, 0x5A,0xF2, 0x90,0x00, 0xBF,0x72, 0x7F,0x73 # i *+@u8:inbuf 41 >= ? i *+@u8:inbuf 37 - $F4 | outbuf +j =u8 ; break
.byte           0x56,0xF1, 0x50,0xFB, 0xE6,0x30, 0xAB,0x00, 0x56,0xF4, 0xA0,0x00, 0x56,0xF0, 0x5A,0xF2, 0x90,0x00 # i *+@u8:inbuf 30 - $F4 | outbuf +j =u8
.byte 	 0x7F,0x72
.byte      0x56,0xF2, 0xEA,0x01, 0x59,0xF2 # j += 1
.byte      0x56,0xF1, 0xEA,0x01, 0x59,0xF1 # i += 1
.byte 	 0xBB,0x71
.byte 0x7F,0x71
.byte 0xE6,0x6F, 0xD7,0x0D, 0xEA,0x05, 0x90,0x00, 0xE6,0x75, 0xD7,0x0D, 0xEA,0x06, 0x90,0x00, 0xE6,0x74, 0xD7,0x0D, 0xEA,0x07, 0x90,0x00 # change "hrmc.dmp" to "hrmc.out"
.byte 0xBA,0x00, 0xE6,0x00, 0xE6,0x00, 0xE6,0x02, 0xE6,0x00, 0xE6,0x00, 0xE6,0x04, 0xE6,0x1C, 0xA4,0x00, 0xD7,0x0D, 0xB4,0x00, 0xDC,0x0C, 0x59,0xF5 # CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0) =F5
.byte 0xBA,0x00, 0xE6,0x00, 0x57,0xFA, 0x56,0xF2, 0x56,0xF0, 0x56,0xF5, 0xB4,0x00, 0xDC,0x0F # WriteFile(fout,msg,size,&written,0)
.byte 0xE7,0x00 # return

## void* get_kernel32()
.byte 0x7E,0x02, 0xBD,0x00, 0x5E,0x10 # 02=&get_kernel32; setup stack frame; reserve 16 stack slots for locals
.byte 0xE9,0x60, 0xE8,0x18, 0xE8,0x20, 0x59,0xFD # @gs:60 +@u64:18 +@u64:20 =FD
.byte 0x56,0xFD, 0xE8,0x20, 0x59,0xFB # @FD +@u64:20 =FB
.byte 0x7E,0x71, 0x56,0xFB, 0xB0,0x71 # @FB ?
.byte      0x56,0xFD, 0xE2,0x38, 0x59,0xF1 # @FD +@u16:38 =F1
.byte      0x56,0xFD, 0xE8,0x40, 0x59,0xFA # @FD +@u64:40 =FA
.byte      0x56,0xF1, 0xEB,0x02, 0x59,0xF1 # @F1 -2 =F1
.byte      0xE6,0x16, 0x59,0xF2 # 22 =F2
.byte      0x7E,0x72, 0x56,0xF2, 0xE6,0x00, 0xC9,0x00, 0xB0,0x72 # @F2 0 >= ?
.byte           0x56,0xFA, 0x5A,0xF1, 0x62,0x00, 0x59,0xF3 # @FA +@F1 @u16 =F3
.byte           0xD7,0x03, 0x5A,0xF2, 0x62,0x00, 0x59,0xF4 # &03 +@F2 @u16 =F4
.byte           0x7E,0x73, 0x56,0xF3, 0x56,0xF4, 0xC1,0x00, 0x56,0xF3, 0x56,0xF4, 0xEA,0x20, 0xC1,0x00, 0xAF,0x00, 0xB0,0x73 # @F3 @F4 != @F3 @F4 +20 != & ?
.byte                0xBF,0x72 # break
.byte           0x7F,0x73
.byte           0x56,0xF1, 0xEB,0x02, 0x59,0xF1 # @F1 -2 =F1
.byte           0x56,0xF2, 0xEB,0x02, 0x59,0xF2 # @F2 -2 =F2
.byte           0xBB,0x72 # continue
.byte      0x7F,0x72
.byte      0x7E,0x72, 0x56,0xF2, 0xE6,0x00, 0xC4,0x00, 0xB0,0x72 # @F2 0 < ?
.byte           0x56,0xFB, 0xBE,0x00 # @FB return
.byte      0x7F,0x72
.byte      0x56,0xFD, 0x68,0x00, 0x59,0xFD # @FD @u64 =FD
.byte      0x56,0xFD, 0xE8,0x20, 0x59,0xFB # @FD +@u64:20 =FB
.byte      0xBB,0x71 # continue
.byte 0x7F,0x71
.byte 0xE7,0x00 # return0

## int strcmp(char* s1 char* s2)
.byte 0x7E,0x05, 0xBD,0x00 # 05=&strcmp; setup stack frame
.byte 0x7E,0x71, 0x56,0x02, 0x60,0x00, 0xB0,0x71, 0x56,0x03, 0x60,0x00, 0xB0,0x71, 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xCE,0x00, 0xB0,0x71 # $02 @u8 ? $03 @u8 ? $02 @u8 $03 @u8 == ?
.byte      0x56,0x02, 0xEA,0x01, 0x59,0x02, 0x56,0x03, 0xEA,0x01, 0x59,0x03 # $02 +1 =$02 $03 +1 =$03
.byte      0xBB,0x71 # continue
.byte 0x7F,0x71
.byte 0x7E,0x71, 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xCE,0x00, 0xB0,0x71 # $02 @u8 $03 @u8 == ?
.byte      0xE7,0x00 # return0
.byte 0x7F,0x71
.byte 0x56,0x02, 0x60,0x00, 0x56,0x03, 0x60,0x00, 0xAB,0x00, 0xBE,0x00 # $02 @u8 $03 @u8 - return

## void* GetProcAddress(void* handle char* proc_name)
.byte 0x7E,0x0A, 0xBD,0x00, 0x5E,0x10 # 0A=&GetProcAddress; setup stack frame; reserve 16 stack slots for locals
.byte 0x56,0x02, 0xE4,0x3C, 0x5A,0x02, 0x59,0xFD # $02 +@u32:3C +$02 =FD
.byte 0x56,0xFD, 0xEA,0x18, 0xE4,0x70, 0x5A,0x02, 0x59,0xFE # @FD +18 +@u32:70 +$02 =FE
.byte 0x56,0xFE, 0xE4,0x20, 0x5A,0x02, 0x59,0xFA # @FE +@u32:20 +$02 =FA
.byte 0xE6,0x00, 0x59,0xF0 # 0 =F0
.byte 0x56,0xFE, 0xE5,0x18, 0x59,0xFF # @FE +@i32:18 =FF
.byte 0xE6,0x00, 0x59,0xF1 # 0 =F1
.byte 0xE6,0x00, 0x59,0xFC # 0 =FC
.byte 0x7E,0x71 # do {
.byte      0x7E,0x72, 0x56,0xFC, 0xE6,0x00, 0xC7,0x00, 0xB0,0x72, 0x56,0xF1, 0x59,0xF0, 0x7F,0x72 # @FC 0 > ? @F1 =F0
.byte      0x7E,0x72, 0x56,0xFC, 0xE6,0x00, 0xC4,0x00, 0xB0,0x72, 0x56,0xF1, 0x59,0xFF, 0x7F,0x72 # @FC 0 < ? @F1 =FF
.byte      0x56,0xFF, 0x5A,0xF0, 0xE6,0x01, 0xA9,0x00, 0x59,0xF1 # @FF @F0 + 1 >> =F1
.byte      0x56,0xF1, 0x54,0xFA, 0x5A,0x02, 0x59,0xF2 # @F1 *+@u32:FA +$02 =F2
.byte      0x56,0xF2, 0x56,0x03, 0xDC,0x05, 0x59,0xFC # @F2 $03 strcmp =FC
.byte 0x56,0xFC, 0xE6,0x00, 0xC1,0x00, 0xB0,0x71, 0xBB,0x71, 0x7F,0x71 # @FC 0 != ? continue }
.byte 0x7E,0x71, 0x56,0xF1, 0x54,0xFA, 0x5A,0x02, 0x56,0x03, 0xDC,0x05, 0xE6,0x00, 0xC1,0x00, 0xB0,0x71, 0xE7,0x00, 0x7F,0x71 # index *+@u32:FA +$02 $03 strcmp 0 != ? return0
.byte 0x56,0xFE, 0xE4,0x24, 0x5A,0x02, 0x59,0xF7 # @FE +@u32:24 +$02 =F7
.byte 0x56,0xFE, 0xE4,0x1C, 0x5A,0x02, 0x59,0xF8 # @FE +@u32:1C +$02 =F8
.byte 0x56,0xF1, 0x52,0xF7, 0x54,0xF8, 0x5A,0x02, 0xBE,0x00 # @F1 *+@u16:F7 *+@u32:F8 +$02 return
.byte 0x00,0x00 # end compilation

.fill 4096 - ($ - hrmc_bytecode), 1, 0x90
hrmc_dest:
.zero (4096<<4) # fill so .text section becomes the size it needs to be in virtual memory, doesn't need to take this much on disk but w/e once it self-compiles it's all gucci

codesize = . - start
filesize = . - mzhdr
