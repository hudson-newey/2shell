# 2sh Modules

2sh modules are the equivalent of in-built commands in other shells.

The difference is that 2sh modules are also standalone executables that you can
install on your system and use with any shell.

When building 2sh, I inline all of the module code into the 2sh executable
like a standalone binary.

I do this so that I can improve the performance of common programs and prevent
spawning an child process (which is computationally expensive).
