comb	lw		0	3	zero	// r3 == 0
		beq		2	0	base2	// r == 0
		beq		2	1	base1	// r == n
		beq		0	0	load
call1	lw		0	7	call2	// comb(n-1,r)
		lw		0	6	neg1	// r6 = -1
		add		1	6	1		// n - 1
		beq		2	1	base1	// if n == r, else...
		beq		0	0	load	// load call2, n-1, r
call2	lw		0	7	call2	// comb(n-1,r-1)
		lw		0	6	neg1	// r6 = -1
		add		2	6	2		// r - 1
		beq		2	0	base2	// if r == 0, else...
		beq		0	0	call1
base1	lw		0	6	one
		add		3	6	3
		beq		5	0	done
		beq		0	0	call2
base2	lw		0	6	one
		add		3	6	3
		beq		5	0	done
		beq		0	0	unload
load	lw		0	6	one
		sw		5	7	Stack	// save return address
		add		5	6	5		// ++Stack ptr
		sw		5	1	Stack	// save n
		add		5	6	5		// ++Stack ptr
		sw		5	2	Stack	//	save r
		add		5	6	5		// ++Stack ptr
		beq		0	0	call1
unload	lw		0	6	neg1	// else
		add		5	6	5		// --Stack ptr
		lw		5	2	Stack	// recover r
		add		5	6	5		// --Stack ptr
		lw		5	1	Stack	// recover n
		add		5	6	5		// --Stack ptr
		lw		5	7	Stack	// recover address
done	jalr	7	0
zero	.fill	0
one		.fill	1
neg1	.fill	-1
Caddr	.fill	comb
