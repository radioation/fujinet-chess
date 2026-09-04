# Fujinet-Chess
Just a learning project for using the fujinet lobby. The better Chess
program for FujiNET is [RetroMate](https://github.com/radioation/fujinet-retromate). 
It connects to [FICS](https://freechess.org) where you can easily 
get a game going.


Built with [MekkoGX](https://github.com/fozzTexx/MekkoGX). MekkoGX is cross-platform
build template for retro and classic computers. 
platforms.


*NOTE* This is a Very EARLY WIP chess server for Fujinet (and other) clients. It uses
[Stockfish](https://stockfishchess.org) as the Chess engine for single player games. 
I'm also using [python-chess](https://github.com/niklasf/python-chess)
to manage the chess board.


Supported Gaming Platforms:
* Atari 8-bit Computers

Planned clients
* Sega Genesis (retrolink TCP over controller port)
* GameBoy Advance (TCP over UART, possibly fujinet if I can sort out how to do that on an ESP32)
* C64 (Meatloaf/FujiNet)
* Apple 2 (FujiNet)

