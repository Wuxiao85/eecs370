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
