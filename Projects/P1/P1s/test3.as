		lw	0	1	ones	// Test lw and nor
		lw	0	2	stuf
		lw	0	3	stuf
		lw	0	4	stuf
		lw	0	5	stuf
		lw	0	6	stuf
		lw	0	7	stuf
		nor	0	0	2
		nor 0	1	3
		nor	1	0	4
		nor	1	1	5
		halt
ones	.fill -1
stuf	.fill 1234
