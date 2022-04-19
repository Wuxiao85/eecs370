		lw	0	6	neg1
		lw	0	5	count
		lw	0	1	one
loop	add	1	1	1
		beq	5	0	done
		add	6	5	5
		beq	0	0	loop
done	halt
one		.fill	1
neg1	.fill	-1
count	.fill	3
