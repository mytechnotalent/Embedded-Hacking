# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Non-Credit Practice Exercise 4 Solution: Automate the Hack

#### Answers

##### GDB Command Definition

```gdb
(gdb) define hack
> set {char[14]} 0x20000000 = {'h','a','c','k','y',',',' ','w','o','r','l','d','\r','\0'}
> set $r0 = 0x20000000
> c
> end
```

##### Usage

```gdb
(gdb) b *0x1000023c
(gdb) c
(gdb) hack                             # Executes all three commands at once
```

##### Expected Serial Output

```
hello, world
hello, world
hello, world
hacky, world     <-- HACKED! (after hack command executed)
hacky, world
```

#### Reflection Answers

1. **How could you parameterize the command to accept different strings or addresses?**
   Standard GDB `define` blocks do not support function parameters directly. However, you can use GDB convenience variables (`set $myaddr = 0x20000000`) and reference them in the macro, or create multiple specific commands like `hack_addr1`, `hack_addr2`. For advanced parameterization, use GDB Python scripting.

2. **What happens if you define `hack` before setting the breakpoint - will it still work as expected?**
   The `define` command only creates a macro; it does not execute immediately. The breakpoint must be set and hit before invoking `hack`. The sequence matters: set breakpoint -> run/continue to hit breakpoint -> then call `hack`. Defining the macro before or after the breakpoint does not matter as long as you invoke it at the right time.

3. **How would you adapt this pattern for multi-step routines (e.g., patch, dump, continue)?**
   Extend the `define` block with additional commands:
   ```gdb
   (gdb) define hack_verbose
   > set {char[14]} 0x20000000 = {'h','a','c','k','y',',',' ','w','o','r','l','d','\r','\0'}
   > x/20b 0x20000000
   > set $r0 = 0x20000000
   > info registers r0
   > c
   > end
   ```
   This dumps memory and registers before continuing, providing verification at each step.
