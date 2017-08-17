//board.c add

	printf("Hit any key to update uboot: 1 ");
	for (i=0; i<100; ++i) {
		if (pl011_tstc()) {	/* we got a key press	*/
			serial_getc();  /* consume input	*/
			update_uboot = 1;
			break;
		}
		udelay(10000);
	}
	printf("\b\b\b 0 \n");

	if(update_uboot == 1)
	{
		ymodem();
		udelay(1000000);
	}
	printf("\n");
	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {

	    //printf("jiaxiyang: hello, world!\n");
		main_loop ();
	}
