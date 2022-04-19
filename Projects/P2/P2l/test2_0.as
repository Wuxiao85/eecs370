		lw	 0	1	G4
		add	 0	1	2
		beq	 0	3	G1
		add	 2	1	3
		sw	 0	5	G2
		sw	 0	5	G5
		lw	 5	3	G7
		beq	 1	4	G6
G1		.fill	G2
G4		.fill	G3
G6		.fill	G5
G8		.fill	G7
