# CPU Cache Simulator

A C++ project I made for my CS 230 class at LAVC.

## What it does

This program simulates a CPU cache. You can read and write values, check what is stored in cache and memory, and remove cache entries.

It uses:
- 32 KB of main memory
- 2 KB of cache
- Two-way set associative caching
- LRU to replace the least recently used entry
- Write-back to save changed values to memory when an entry is removed

## Commands

- A address R — Read an address
- A address W value — Write a value
- B address — Show cache and memory information
- C — Show cache entries with nonzero values
- D — Show memory blocks with nonzero values
- E address — Remove an entry from cache
- B -1 — Exit

## Example

A 100 W 25
B 100
E 100
B 100
B -1

This writes 25 to the cache at address 100. Removing the entry saves the value to main memory.
