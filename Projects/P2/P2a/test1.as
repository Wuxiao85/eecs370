Def1	lw	0	1	Undef2
		lw	0	1	Def2
Def2	lw	0	1	Def1
		lw	0	1	Undef1
Def3	sw	0	1	Undef4
		sw	0	1	Def4
Def4	sw	0	1	Def3
		sw	0	1	Undef3
		beq	0	0	Def1
		beq	0	0	Def2
		beq	0	0	Def3
		beq	0	0	Def4
Def5	.fill	Def6
		.fill	Undef6
Def6	.fill	Undef5
		.fill	Def5
