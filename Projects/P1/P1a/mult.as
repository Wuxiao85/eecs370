		lw		0	2	Mcand
		lw		0	3	Mplier
		lw		0	4	BITi
		lw		0	7	Num
getBit	nor		3	3	5		// r3 nor r3 = r5
		nor		4	4	6		// r4 nor r4 = r6
		nor		5	6	5		// r5 nor r6 = r3 and r4 = r5
		beq		5	0	lftSHF	// if r5 == 0, times0 else times1
times1	add		1	2	1		// r1 = r1 + r2
lftSHF	add		2	2	2		// r5 = r2 + r2 = r2 * 2 = r2 << 1  shift mcand left 1 for next sum
		lw		0	6	NegOne
		add		7	6	7		// Decrement Num
		beq		7	0	done	// If Num == 0, then Done else continue
		add		4	4	4		// Left shift bit finder
		beq		0	0	-11
done	halt
Mcand	.fill	32766	// shift after every addition
Mplier	.fill	10383	// const
NegOne	.fill	-1		// const
Num		.fill	15
BITi	.fill	1
