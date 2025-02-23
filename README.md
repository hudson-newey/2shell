# 2Shell

A command line shell focused on speed.

## Why did I need a new shell

- I was sick of my terminal window opening and having to wait another half a second for the shell to become interactive.
- Doing any input including an empty space on zsh (my favorite shell) had a noticable delay.
- Shells are doubled as a scripting language, when people typically just use bash as the "shell scripting language". Meaning that shells like zsh and fish are filled with complexity related to scripting, when no one uses these features for command line shell usage.

## Why 2sh is better

- cold start time to interactivity is significantly faster
- 2sh is multi-threaded
- 2sh caches executable locations in a hash map, meaning that executable location lookups are an `O(1)` operation instead of `O(n)` (does not occur when using command line argument invokation, because creating a hash map would slow down execution in this use case)
- no scripting language nonsense (improving performance). 2sh is a command line shell, and should not be used in scripting contexts
