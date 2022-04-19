        lw      0       1       five    five should be 7
        lw      1       2       3
start   add     1       2       1
        beq     0       1       2
        beq     0       0       start   offset should be -3
        noop
done    halt                            end of program
five    .fill   5
neg1    .fill   -1
stAddr  .fill   start                   will contain the address of start (2)
