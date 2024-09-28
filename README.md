# hrmc
Human Readable Machine Code

HRMC is mostly inspired by [David Smith's videos](https://www.youtube.com/@davidsmith7791/videos)
and [Devine Lu Linvega's Strange Loop talk](https://www.youtube.com/watch?v=T3u7bGgVspM).
Watching those videos will really put into perspective what the heck this thing is.
The main thing that makes HRMC stand out is instead of going straight to parsing a textual language,
I want to stay at a machine code/bytecode level and see if I can make programming at that level more ergonomic
so I can bootstrap up to a [Dion Systems](https://www.youtube.com/watch?v=GB_oTjVVgDc) type editor
where the underlying structure is more AST-like than text-like.

HRMC is not really a machine code (yet) - it's more of a bytecode.
The bytecode is designed in such a way that the hexadecimal representation is a mnemonic for what it does.
It's also an index into a lookup table of machine code that does the operation.
So compiling is mostly just copying over the machine code at that index and patching immediate values.
The following is an explantion of the design.

| byte1 | kinds of data/machine code contained within range |                     mnemonic/reasoning                     | byte2  |
| ----- | ------------------------------------------------- | ---------------------------------------------------------- | -----  |
| 00-0F | meta data and common global pointers              | kinda like a kernel's vector table                         | ignored|
| 10-1F | local variables - not needed anymore              | 1 like l as in local                                       | ignored|
| 20-2F | [TODO] conversions                                | 2 like to as in convert to                                 | ignored|
| 30-3F | [TODO] mul-add ops for easy AoS access            | 3 like a rotated m as in mul                               | index  |
| 40-4F | [TODO] 4-wide f32 vector ops                      | 4 like 4-wide floats                                       | ignored|
| 50-5F | ops using the Stack Frame                         | 5 like S as in Stack                                       | index  |
| 60-6F | load ops                                          | 6 looks kinda like (- which looks like <- as in reg <- mem | ignored|
| 70-7F | tag/label ops for fixing rel branch offsets       | 7 like T as in Tag                                         | tag id |
| 80-8F | [TODO] 8-wide f32 vector ops                      | 8 like 8-wide floats                                       | ignored|
| 90-9F | store ops                                         | 9 looks kinda like -) which looks like -> as in reg -> mem | ignored|
| A0-AF | 64-bit integer Arithmetic and bitwise ops         | A for Arithmetic or for boolean Algebra                    | ignored|
| B0-BF | ops for Branching/calling and ABI stuff           | B for Branch                                               |[tag id]|
| C0-CF | Comparison operations                             | C for Compare                                              | ignored|
| D0-DF | ops using the lookup table as Data                | D for Data as in it uses table entries as Data             | index  |
| E0-EF | ops using immediate values                        | E for EEEEmmediate or as a rotated M as in iMMediate       | i8     |
| F0-FF | [TODO] Floating point ops                         | F for float                                                | ignored|

- 5x/6x/9x/Dx/Ex use the same x for dealing with primitive types like so:
	- 0/1/2/3/4/5/8/D/F for u8/i8/u16/i16/u32/i32/u64/f64/f32
	- what they do with the data is different, though
		- 6x/9x just load or store that type (signed and unsigned store is the same)
		- 5x/Dx do a `*+@` which is useful for subscripting primitive arrays
		- Ex just does a `+@` which is useful for accessing primitive struct members
- 5x/Dx/Ex also use x like:
	- A/B for + and -
	- 6/9 for load and store as u64
- 5x/Dx both use x like:
	- 7 for addr-of
	- C for call

- So `5502` does `*+@i32` with stack slot 2 aka `i32_array_passed_as_first_param[index_at_top_of_stack]`
	- Stack slot 2 is the first param because 0 is rbp and 1 is the return address. -1 would be the first local variable.
	- ... except when we want a function callable from external things using the C FFI.

		In this case, stack slot -1 is actually the first param, -2 is 2nd, -3 is 3rd, -4 is 4th, 6 is the 5th param, 7 is the 6th, etc.

	- Why the negative stack slots?

		The negatives are usually for local variables, but after setting up the stack frame I just push the ABI regs
		so that I can easily reference them with the 5x ops instead of having a whole set of ops working with registers.

	- Why does 5th+ params start at slot 6? I thought slot 2 was the first param after rbp and return address?

		Windows x64 calling convention requires 0x20 bytes of shadow space, whatever that is.
		So 0x20 + 0x10 for the first 2 slots = rbp+0x30 aka slot 6

- Why isn't load and store on 1x and 5x?

	To simplify the compiler, ops that use the 2nd byte in a certain way are grouped by the 1st hex digit of the 1st byte.
	So we only have the 2nd hex digit of the 1st byte to distinguish ops that use the 2nd byte in the same way.
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
	Also it's very easy to later eliminate the nops (especially if 90 isn't anywhere in valid code).

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

