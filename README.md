# hrmc
Human Readable Machine Code

HRMC is not really a machine code (yet) - it's more of a bytecode.
The bytecode is designed in such a way that the hexadecimal representation is a mnemonic for what it does.

HRMC is mostly inspired by [David Smith's videos](https://www.youtube.com/@davidsmith7791/videos)
and [Devine Lu Linvega's Strange Loop talk](https://www.youtube.com/watch?v=T3u7bGgVspM).
Watching those videos will really put into perspective what the heck this thing is.
The main thing that makes HRMC stand out is instead of going straight to parsing a textual language,
I want to stay at a machine code/bytecode level and see if I can make programming at that level more ergonomic
so I can bootstrap up to a [Dion Systems](https://www.youtube.com/watch?v=GB_oTjVVgDc) type editor
where the underlying structure is more AST-like than text-like.
