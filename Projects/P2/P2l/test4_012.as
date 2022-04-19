		add	0	1	0
local	nor	0	1	0
		lw	0	1	one
		add	0	2	0
two		noop
		nor	0	2	0
		sw	0	2	two
		nor	2	0	2
		noop
lo		lw	0	3	three
		nor	0	3	0
		add	0	3	0
		nor	3	0	3
		halt
		.fill	1
one		.fill	local
		.fill	1
		.fill	2
low		.fill	22
		.fill	low
three	.fill	lo
		.fill	3
		.fill	33
