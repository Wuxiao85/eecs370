		add		0	0	0
		nor		0	0	1
		lw		0	0	-32768
		sw		0	0	32767
		beq		0	1	0
		noop
		jalr	7	7
done	halt
