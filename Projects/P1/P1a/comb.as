        lw          0       1       n
        lw          0       2       r
        lw          0       4       Caddr        load combination function address
        jalr        4       7                    call function
        halt
comb	lw		0	3	zero	// r3 == 0
		beq		2	0	base	// r == 0
		beq		2	1	base	// r == n
		beq		0	0	store	// store halt, n, r, r3
call2	lw		0	4	Loadr
		lw		0	6	neg1	// r6 = -1
		add		2	6	2		// r - 1
		beq		2	6	load
		jalr	7	6			// store comb, n-1, r-1, r3
store	lw		0	6	one
		sw		5	7	Stack	// save return address
		add		5	6	5			// ++Stack ptr
		sw		5	1	Stack	// save n
		add		5	6	5			// ++Stack ptr
		sw		5	2	Stack	//	save r
		add		5	6	5			// ++Stack ptr
		sw		5	4	Stack	//	save r4
		add		5	6	5			// ++Stack ptr
		sw		5	3	Stack	//	save r3
		add		5	6	5			// ++Stack ptr
		lw		0	4	zero
		lw		0	7	Caddr
call1	lw		0	6	neg1	// r6 = -1
		add		1	6	1		// n - 1
		jalr	7	6			// store comb, n-1, r, r3
load	lw		0	6	neg1
		add		5	6	5			// --Stack ptr
		lw		5	3	Stack	// recover r3
		add		5	6	5			// --Stack ptr
		lw		5	4	Stack	// recover r4
		add		5	6	5			// --Stack ptr
		lw		5	2	Stack	// recover r
		add		5	6	5			// --Stack ptr
		lw		5	1	Stack	// recover n
		add		5	6	5			// --Stack ptr
		lw		5	7	Stack	// recover address
		beq		0	5	done
		beq		0	0	addr
base	lw		0	3	one		// If Stack ptr == 0, return 1
		beq		0	5	done
addr	lw		0	6	neg1
		add		5	6	5			// --Stack ptr
		add		0	3	6		// load r6 = r3 prev
		lw		5	3	Stack	// recover r3
		add		3	6	3		// r3 = r3 + r6(r3 prev)
		lw		0	6	one
		sw		5	3	Stack	//	save r3
		add		5	6	5			// ++Stack ptr
		beq		5	0	done
		beq		0	4	call2
		jalr	4	6
done	jalr	7	6
n       .fill       3
r       .fill       1
zero	.fill	0
one		.fill	1
neg1	.fill	-1
Callr2	.fill	call2
Addr	.fill	addr
Loadr	.fill	load
Caddr	.fill	comb
Stack	.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
		.fill	0
