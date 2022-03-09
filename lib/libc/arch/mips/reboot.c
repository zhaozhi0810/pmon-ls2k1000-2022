void
tgt_reboot()
{

	void (*longreach) (void);
	
	longreach = (void *)0xbfc00000;
	(*longreach)();

	printf("2022-03-01 reboot\n");

	while(1);

}
