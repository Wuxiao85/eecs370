		lw		0	1	one
		noop
		noop
		noop
		add		1	1	2
		nor		0	0	3
		lw		0	4	four
		sw		0	4	x
		halt
one		.fill	1
four	.fill	4
x		.fill	0
