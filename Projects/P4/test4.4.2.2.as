		lw  	0   	1   	16		// 4 2 2	00 0 00
		lw  	0   	2   	20
		lw  	0   	3   	24
		lw  	0   	4   	28
		add		1		2		1		// 00 1 00
		add		2		3		2
		nor		3		3		3
		nor		4		4		4
		sw		0		1		17		// 01 0 00
		sw		0		2		21
		sw		0		3		25
		sw		0		4		29
		noop							// 01 1 00
		noop
		noop
		halt
data	.fill	1		// 10 0 00
		.fill	2
		.fill	3
		.fill	4
data2	.fill	5		// 10 1 00
		.fill	6
		.fill	7
		.fill	8
data3	.fill	9		// 11 0 00
		.fill	10
		.fill	11
		.fill	12
data4	.fill	13		// 11 1 00
		.fill	14
		.fill	15
		.fill	16
