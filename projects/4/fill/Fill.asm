// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

//// Replace this comment with your code.

(LOOP)

    @8192
    D = A
    @pixel
    M = D 
    @KBD
    D = M
    @CASE2
    D ; JEQ

(CASE1)

    @SCREEN
    D = A
    @address
    M = D

    @i
    M = 0

(LOOP1)

    @i
    D = M
    @pixel
    D = D - M
    @STOP
    D ; JGE

    @address
    D = M
    @i
    A = D + M
    M = -1
    
    @i
    M = M + 1

    @LOOP1
    0 ; JMP

(STOP)

    @LOOP
    0 ; JMP

(CASE2)

    @SCREEN
    D = A
    @address
    M = D

    @i
    M = 0

(LOOP2)

    @i
    D = M
    @pixel
    D = D - M
    @STOP
    D ; JGE

    @address
    D = M
    @i
    A = D + M
    M = 0

    @i
    M = M + 1

    @LOOP2
    0 ; JMP
