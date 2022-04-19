		lw		0	1	one
		add		1	1	2
		lw		0	3	neg5
		nor		3	3	4
		lw		0	5	x
		sw		0	5	result
		lw		0	1	zero
		beq		0	1	done
		lw		0	0	result
		lw		0	1	result
		lw		0	2	result
done	halt
zero	.fill	0
one		.fill	1
neg5	.fill	-5
x		.fill	100
result	.fill	-100
