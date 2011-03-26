//GameShark functions exported by gsuploader.

//Initialize GameShark utilities and communication.
void (*gscomms_init)(void)=(void *)0xA02FFED0;

//Check GameShark button and return state
//0 = not pressed
//1 = pressed
int (*check_gsbutton)(void)=(void *)0xA02FFFE4;