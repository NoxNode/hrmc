# HRMC
Human Readable Machine Code

## Motivation

HRMC is mostly inspired by [David Smith's SmithForth](https://www.youtube.com/watch?v=9MSJGzYELBA)
and [Devine Lu Linvega's Strange Loop talk](https://www.youtube.com/watch?v=T3u7bGgVspM).
What differentiates HRMC from those 2 systems is I don't embrace the Forth way of doing things.
Coming from C, I found stack manipulation stuff like dup, drop, swap to be painful to work with.
I just want to make C-like statements in an ergonomic RPN bytecode and render it however I want.
My goal is to use HRMC to bootstrap up to an editor with ideas from
[Learnable Programming](https://worrydream.com/LearnableProgramming/),
[Moldable Development](https://youtu.be/Pot9GnHFOVU?t=706),
[Acme](https://youtu.be/dP1xVpMPn8M?t=285),
[Cedar](https://youtu.be/z_dt7NG38V4?t=2409),
[The Mother of All Demos](https://youtu.be/UhpTiWyVa6k?t=1292),
[Tomorrow Corp Tech](https://youtu.be/72y2EC5fkcE?t=453),
and [Dion Systems](https://youtu.be/GB_oTjVVgDc?t=1709).

## Implementation

hrmc.s and hrmc.dmp are 2 implementations of the HRMC compiler done in different ways (GNU assembly and hex dump).
They start with making a simple executable file header with a read-write-execute .text section big enough for all the code and data needed.
Then comes the 256 bytes of machine code that compiles the bytecode and jumps to the generated machine code.
Then comes the 256x16 byte HRMC lookup table of machine code.
Then comes the HRMC bytecode.
At the top of the files are shell commands to compile/inspect the HRMC compiler executables.
My vim config has shortcuts to run a command at line 1-9 and put the output in the prev/new window.

`ref_code` contains some C code that were old attemps at a simple C compiler that I often used as references.
`link_win` in `min_common_win.c` is probably most helpful for understanding the whole `get_kernel32` `GetProcAddress` stuff.

## Design

HRMC is not really a machine code (yet) - it's more of a bytecode, and it always has a parameter byte even if ignored.
The bytecode is designed in such a way that the hexadecimal representation is a mnemonic for what it does.
It's also an index into a lookup table of machine code that does the operation.
So compiling is mostly just copying over the machine code at that index and patching immediate values.
The following is an explantion of the design.

| op |  kinds of data/machine code in op range   |               mnemonic/reasoning               | param  |
| -- | ----------------------------------------- | ---------------------------------------------- | ------ |
| 0x | meta data and common global pointers      | kinda like a kernel's vector table             |    -   |
| 1x | local variables - not needed anymore      | 1 like l as in local                           |    -   |
| 2x | [TODO] conversions                        | 2 like to as in convert to                     |    -   |
| 3x | [TODO] mul-add ops for easy AoS access    | 3 like a rotated m as in mul                   | index  |
| 4x | [TODO] 4-wide f32 vector ops              | 4 like 4-wide floats                           |    -   |
| 5x | ops using the Stack Frame                 | 5 like S as in Stack                           | index  |
| 6x | load ops                                  | 6 looks like (- which is like <- in reg <- mem |    -   |
| 7x | tag/label ops for fixing relative offsets | 7 like T as in Tag                             | tag_id |
| 8x | [TODO] 8-wide f32 vector ops              | 8 like 8-wide floats                           |    -   |
| 9x | store ops                                 | 9 looks like -) which is like -> in reg -> mem |    -   |
| Ax | 64-bit integer Arithmetic and bitwise ops | A for Arithmetic or for boolean Algebra        |    -   |
| Bx | ops for Branching/calling and ABI stuff   | B for Branch                                   |[tag_id]|
| Cx | Comparison operations                     | C for Compare                                  |    -   |
| Dx | ops using the lookup table as Data        | D for Data as in it uses table entries as Data | index  |
| Ex | ops using immediate values                | E for EEEEmmediate or a rotated M in iMMediate | i8     |
| Fx | [TODO] Floating point ops                 | F for float                                    |    -   |

- 5x/6x/9x/Dx/Ex use the same x for dealing with primitive types like so:
	- 0/1/2/3/4/5/8/D/F for u8/i8/u16/i16/u32/i32/u64/f64/f32
	- what they do with the data is different, though
		- 6x/9x just load or store that type (signed and unsigned store is the same)
		- 5x/Dx do a `*+@` which is useful for subscripting primitive arrays
		- Ex just does a `+@` which is useful for accessing primitive struct members
- 5x/Dx/Ex also use x like so:
	- A/B for + and - (A for Add, B for suB)
	- 6/9 for load and store as u64 (storing to an imm makes no sense so E9 is load from gs reg
	cuz 9 looks like g in some fonts - gs reg is used for getting kernel32 in order to call Win32 API funcs)
	- 7 for addr-of (& is on the 7 key - E7 is return imm as in rET)
	- C for call (I'm thinking of making EC be imm shift left for easy large imm creation)

- So `5502` does `*+@i32` with stack slot 2 aka `i32_array_passed_as_first_param[index_at_top_of_stack]`
	- Stack slot 2 is the first param because 0 is rbp and 1 is the return address. -1 would be the first local variable.
	- ... except if want a function callable from external things using the C FFI.

		In this case, stack slot -1 is actually the first param, -2 is 2nd, -3 is 3rd, -4 is 4th, 6 is the 5th param, 7 is the 6th, etc.

	- Why the negative stack slots?

		The negatives are usually for local variables, but after setting up the stack frame I just push the ABI regs
		so that I can easily reference them with the 5x ops instead of having a whole set of ops working with registers.

	- Why does 5th+ params start at slot 6? I thought slot 2 was the first param after rbp and return address?

		Windows x64 calling convention requires 0x20 bytes of shadow space, whatever that is.
		So 0x20 + 0x10 for the first 2 slots = rbp+0x30 aka slot 6

The mnemonic/reasoning for all the entries in the remaining op groups (Ax, Bx, Cx, 7x) is as follows:

| op |    what it does    |                    mnemonic/reasoning                  |
| -- |  ----------------- | ------------------------------------------------------ |
| A0 | or                 | 0 like O as in OR                                      |
| A1 | not                | 1 like ! but bitwise not logical                       |
| A2 | udiv               | 2 like a rotated u as in udiv                          |
| A3 | umul upper 64 bits | 3 like a rotated m as in mul                           |
| A4 | shift left         | 4 is the number that most looks like < as in <<        |
| A5 | negate             | 5 like S as in Sign as in change Sign                  |
| A6 | umod               | 6 looks like C, signed modulo is C                     |
| A7 | shift right        | 7 like a skewed > as in >>                             |
| A8 | xor                | 8 ignore top and bottom and its an x                   |
| A9 | arith shift right  | 9 like -> (see above) and the - is like sign aka arith |
| AA | add                | A for Add                                              |
| AB | sub                | B for suB                                              |
| AC | modulo             | C looks like a loop, modulo loops circular buffers     |
| AD | div                | D for Div                                              |
| AE | mul                | E like a rotated M as in Mul                           |
| AF | and                | F like a bitmask for an and                            |
| B0 | branch if 0        | self explanatory                                       |
| B1 | pop 1 abi reg      | self explanatory                                       |
| B2 | pop 2 abi regs     | self explanatory                                       |
| B3 | pop 3 abi regs     | self explanatory                                       |
| B4 | pop 4 abi regs     | self explanatory                                       |
| B5 | pop 5 abi regs     | self explanatory                                       |
| B6 | pop 6 abi regs     | self explanatory                                       |
| B7 | pop 7 abi regs     | self explanatory                                       |
| B8 | pop 8 abi regs     | self explanatory                                       |
| B9 | non-syscall abi    | 9 like plan9 -> unix -> linux which needs this         |
| BA | align stack        | self explanatory                                       |
| BB | branch back        | self explanatory                                       |
| BC | call proc          | self explanatory                                       |
| BD | deploy stack frame | D for Deploy/setup stack frame and push abi regs       |
| BE | return             | E for rEturn                                           |
| BF | branch forward     | self explanatory                                       |
| C0 | unassigned         |                                                        |
| C1 | not equal          | 1 like ! as in !=                                      |
| C2 | lt or eq unsigned  | 2 like u as in unsigned (TODO: better mnemonic)        |
| C3 | gt or eq unsigned  | 3 is gt 2 and 2 is unsigned                            |
| C4 | lt signed          | 4 is the number that most looks like <                 |
| C5 | unassigned         |                                                        |
| C6 | lt or eq signed    | 6 like <- or <=                                        |
| C7 | gt signed          | 7 like a skewed >                                      |
| C8 | unassigned         |                                                        |
| C9 | gt or eq signed    | 9 like -> or => or >=                                  |
| CA | gt unsigned        | A for Above aka unsigned >                             |
| CB | lt unsigned        | B for Below aka unsigned <                             |
| CC | unassigned         |                                                        |
| CD | unassigned         |                                                        |
| CE | equal to           | E for Equal to                                         |
| CF | unassigned         |                                                        |
| 7E | tag enscribe       | enscribe as in write addr into table                   |
| 7F | tag fixup          | fixup as in fixup rel offsets of branches to this      |

## Why this design?

- Why isn't load and store on 1x and 5x?

	To simplify the compiler, ops that use the param byte the same way are grouped by the 1st hex digit of the op byte.
	So there's only the 2nd hex digit of the op byte to distinguish between ops that use the param byte in the same way.
	So instructions that load and store need to not collide with typed instructions.
	So either 1 and 5 can't mean i8 and i32 or they can't mean load and store.
	I like the idea of using the size in bytes as the type and +1 to mean signed more than 1 and 5 for load and store.
	So I moved load and store to 6 and 9 and was able to still find a neat mnemonic for them.

- Why not group by 2nd hex digit?

	I also explored using the 2nd hexadecimal digit for the grouping.
	This also led/pushed me down the path of making mnemonics that are more like AD and D1 for add and div
		- aka just use the first 2 letters of the word(s) you would use if not restricted to hexadecimal.
	But I found it difficult to remember what instructions were what when coding with that format.
	I found it much easier to remember what instructions were what when grouping by the first hexadecimal digit.
	It's like the first hexadecimal digit is the category then the 2nd is the specific operation.
	It lets me hone in on just the 16 entries within a group instead of thinking about the whole 256 entries.

- Why 2 byte pairs instead of one byte per opcode with an optional byte for immediate value like the 6502?

	I want all instructions to be a fixed size for easy search/navigation/vertical alignment.
	Having everything be the same size makes programming in this thing more like a spreadsheet, which is good imo.

- Why 16 byte output even for instructions that map to 2 bytes of actual machine code?

	Similar to the previous point, having things fixed size makes lots of things easier.
	I also want to easily decompile - and with each outupt as a fixed size, that becomes much easier.
	Also it's very easy to later eliminate the nops.

- Why stack based?

	I explored register based with like 4 bytes per hrmc, but it a lot more verbose, less composable, and harder to follow.
	I did think about how I could do both stack and register based in the same encoding
		- like have a different scope-start op for stack vs register code, but I think that'd be too complicated.
	A lot of design decisions started becoming clearer when I made the following ordered list of priorities:

		1. Simplicity of implementation
		2. Expressivity/Ergonomics
		3. Control
	
	As someone who really dislikes not having control over everything, it felt wrong putting control last.
	However, it felt right again when I instead thought: defer control and don't put up any barriers to getting full control later.
	While bootstrapping, I think simplicity and ergonomics are just too important.

- Why bootstrap?

	It's fun.

