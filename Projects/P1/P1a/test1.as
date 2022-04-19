		lw		0	1	1000	comment
loop	beq		1	6	done
		jalr	4	7
		nor		1	3	1
		beq		0	0	loop
		noop	1	0	1
done	halt
