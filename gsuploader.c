#include <stdio.h>
#include "mips.h"
#define LPT1 0xc400
#define UPLOAD_ADDR 0xA0300000 //Where to upload your code, and also where execution starts after embedded code runs.
#define GLOBAL_OFFSET_TABLE 0xA0000200 //Where to store exported function GOT.
#define TMOUT 0x4000 //This can probably be lower, it's set this high just for testing.
unsigned char GAMESHARK_COMMS_INIT_GOT[4];
unsigned char GAMESHARK_CHECK_GSBUTTON_GOT[4];

#ifdef LINUX
#include <sys/io.h>
#define _outp(x,y) outb(y,x)
#define _inp(x) inb(x)
#else
//Dev-Cpp conio.h does not define these.
void _outp(unsigned short address, unsigned char data);
unsigned char _inp(unsigned short address);
#endif


/* Embedded pre-setup code
   This is a lazy way of doing things, but it works. This code gets executed and sets
   up the environment prior to loading your usercode.
*/
    unsigned long codebuf_pre[]=
    {
     
    /* Stop GameShark traps */
         MTC0(MIPS_R0, 18),
         NOP,
         MTC0(MIPS_R0, 19),
         NOP,
    
    /* Disable Interrupts */
        MFC0(MIPS_T0, 12),
      	0x2409fffe, 
        0x01094024, 
      	MTC0(MIPS_T0, 12),
         
    /* Modify EPC */
         LUI(MIPS_K0, UPLOAD_ADDR>>16), 
         ORI(MIPS_K0, MIPS_K0, UPLOAD_ADDR), 
         NOP,
         MTC0(MIPS_K0, 14),
         NOP,
    
    /* Patch back modified code handler */
         LUI(MIPS_K1, 0x3c1a),
         ORI(MIPS_K1, MIPS_K1, 0x8000),
         NOP,
         LUI(MIPS_K0, 0xa07c),
         ORI(MIPS_K0, MIPS_K0, 0x5c00),
         NOP,
         SW(MIPS_K1, 0, MIPS_K0),
         NOP,
         SYNC,
         NOP,
         
    /* Halt RSP */
         LUI(MIPS_T1, 2),
         LUI(MIPS_T0, 0xa404),
         ORI(MIPS_T0, MIPS_T0, 0x0010),
         NOP,
         SW(MIPS_T1, 0, MIPS_T0),
         NOP,
         
    /* Halt RDP */
         LUI(MIPS_T1, 1|4|0x10|0x40|0x80|0x100|0x200),
         LUI(MIPS_T0, 0xa410),
         ORI(MIPS_T0, MIPS_T0, 0x000c),
         NOP,
         SW(MIPS_T1, 0, MIPS_T0),
         NOP,
         
    /* Return from interrupt - execute code */
         ERET,
         NOP,
         
    /* Callback function: Start GScomms */
        GAMESHARK_COMMS_INIT, //GScomms Marker
        NOP,
        MFC0(MIPS_T0, 12),
      	0x2409fffe, 
        0x01094024, 
      	MTC0(MIPS_T0, 12),
       	LUI(MIPS_V0,0xa000),
        ORI(MIPS_A2,MIPS_V0,0x180),
        LW(MIPS_A0,0,MIPS_A2),
     	ORI(MIPS_V1,MIPS_V0,0x120),
        SW(MIPS_A0,0, MIPS_V1),
    	ORI(MIPS_A1,MIPS_V0,0x184),
     	LW(MIPS_A0,0,MIPS_A1),
     	ORI(MIPS_V1,MIPS_V0,0x124),
     	SW(MIPS_A0,0,MIPS_V1),
        ORI(MIPS_A0,MIPS_V0,0x188),
    	LW(MIPS_A3,0,MIPS_A0),
    	ORI(MIPS_V1,MIPS_V0,0x128),
    	SW(MIPS_A3,0,MIPS_V1),
      	ORI(MIPS_V1,MIPS_V0,0x18c),
     	LW(MIPS_A3,0,MIPS_V1),
     	ORI(MIPS_V0,MIPS_V0,0x12c),
    	SW(MIPS_A3,0,MIPS_V0),
    	LUI(MIPS_V0,0x3c1a),
     	ORI(MIPS_V0,MIPS_V0,0xa079),
     	SW(MIPS_V0,0,MIPS_A2),
     	LUI(MIPS_V0,0x275a),
     	ORI(MIPS_V0,MIPS_V0,0x4aec),
    	SW(	MIPS_V0,0,MIPS_A1),
     	LUI(MIPS_V0,0x340),
    	ORI(MIPS_V0,MIPS_V0,0x8),
    	SW(MIPS_V0,0,MIPS_A0),
    	SW(MIPS_R0,0,MIPS_V1),
    	LUI(MIPS_T1,0xa000),
    	ORI(MIPS_T1,MIPS_T1,0x120),
        CACHE(MIPS_T1, 0x10, 0),
    	NOP,
     	LUI(MIPS_T1,0xa000),
    	ORI(MIPS_T1,MIPS_T1,0x124),
     	CACHE(MIPS_T1, 0x10, 0),
     	NOP,
      	LUI(MIPS_T1,0xa000),
      	ORI(MIPS_T1,MIPS_T1,0x128),
      	CACHE(MIPS_T1, 0x10, 0),
      	NOP,
     	LUI(MIPS_T1,0xa000),
      	ORI(MIPS_T1,MIPS_T1,0x12c),
      	CACHE(MIPS_T1, 0x10, 0),
      	NOP,
      	LUI(MIPS_T1,0xa000),
      	ORI(MIPS_T1,MIPS_T1,0x180),
      	CACHE(MIPS_T1, 0x10, 0),
      	NOP,
      	LUI(MIPS_T1,0xa000),
     	ORI(MIPS_T1,MIPS_T1,0x184),
      	CACHE(MIPS_T1, 0x10, 0),
      	NOP,
      	LUI(MIPS_T1,0xa000),
      	ORI(MIPS_T1,MIPS_T1,0x188),
      	CACHE(MIPS_T1, 0x10, 0),
      	NOP,
      	LUI(MIPS_T1,0xa000),
     	ORI(MIPS_T1,MIPS_T1,0x18c),
    	CACHE(MIPS_T1, 0x10, 0),
    	MFC0(MIPS_T0, 12),
    	ORI(MIPS_T0, MIPS_T0, 1),
    	MTC0(MIPS_T0, 12),
        JR(MIPS_RA),    
        NOP,
        GAMESHARK_CHECK_GSBUTTON,
        LUI(MIPS_V0, 0xbe40),
        LBU(MIPS_V0, 0, MIPS_V0),
        SRA(MIPS_V0, MIPS_V0, 0x2),
        XORI(MIPS_V0, MIPS_V0, 0x1),
        ANDI(MIPS_V0, MIPS_V0, 0x1),
        JR(MIPS_RA),
        NOP,
   };
   
int main(int argc, char ** argv)
{

  export_funcs(); //Build GOT export list for built in functions.
  
  int i = 0;
  unsigned char *pointprebuf=(unsigned char *)&codebuf_pre;
  unsigned char codebuf[sizeof(codebuf_pre)];  
  
#ifdef LINUX
    int ret = ioperm(LPT1, 10, 1);
    if (ret < 0)
        err(1, "ioperm");
#endif

    /*Pull parport line low.*/
    Out32(LPT1, 0);
     
    printf("N64 HomeBrew Loader - ppcasm\n(Based on HCS GSUpload - http://here.is/halleyscomet)\n\n");
   
    if(argc!=2)
    {
       printf("Wrong Usage: %s <binary>\n", argv[0]);
       return 1;
    }
   
    FILE* infile=fopen(argv[1], "rb");
    if(!infile)
    {
       printf("error opening %s\n", argv[1]);
       return 1;
    }
   
    printf("File Size: %d bytes...\n", (int)GetFileSizez(infile));  
    
       
    for(i=0;i<=sizeof(codebuf_pre)/sizeof(unsigned long);i++)
    {
         codebuf_pre[i] = (codebuf_pre[i]<<24&0xff000000)|(codebuf_pre[i]<<8&0x00ff0000)|(codebuf_pre[i]>>8&0x0000ff00)|(codebuf_pre[i]>>24&0xff); 
    }
    
    for(i=0;i<=sizeof(codebuf);i++)
    {
         codebuf[i]=pointprebuf[i];                                  
    }
   
    /*Upload binary to specified address.*/
    if(UploadFile(infile,UPLOAD_ADDR))
    {
       printf("Uploading file failed...\n");
       Out32(LPT1, 0);
       return 1;
    }
    
    /*Set up embedded code to right before usercode.*/
    unsigned long EMBED_ADDR = UPLOAD_ADDR-sizeof(codebuf);

    /*Upload embedded code right before usercode.*/
    if(Upload(codebuf, sizeof(codebuf), EMBED_ADDR))
    {
       printf("Failed to upload embedded code...\n");
       Out32(LPT1, 0);
       return 1;
    }
   
    printf("Uploaded embedded code to: 0x%08x.\n", (unsigned int)EMBED_ADDR);
    
    /*Make synthetic jump instruction based on address.*/
    unsigned long instruction=makejump(EMBED_ADDR);
    
    /*Unload jump instruction into byte buffer for easy transfer.*/
    unsigned char insn[4];
    insn[0]=instruction>>24&0xff;
    insn[1]=instruction>>16&0xff;
    insn[2]=instruction>>8&0xff;
    insn[3]=instruction>>0&0xff;
    
    /*Inject synthetic jump instruction into code handler to insure it runs.*/                   
    if(Upload(insn,4,INSN_PATCH_ADDR))
    {  
       printf("Instruction patch failed...\n");
       Out32(LPT1, 0);
       return 1;    
    }

    printf("Done.\n");
   
    Out32(LPT1, 0);
    fclose(infile);
  return 0;
}

void export_funcs(void)
{
        printf("\nDbg: Function Callbacks:\n");

        int i = 0;
        for(i=0;i<=sizeof(codebuf_pre)/4;i++)
        {
             if(codebuf_pre[i]==GAMESHARK_COMMS_INIT)
             {
                  int address = UPLOAD_ADDR-sizeof(codebuf_pre)+i*4+4;
                  unsigned long make_jump_got = J(address);
                  GAMESHARK_COMMS_INIT_GOT[0]=make_jump_got>>24&0xff;
                  GAMESHARK_COMMS_INIT_GOT[1]=make_jump_got>>16&0xff;
                  GAMESHARK_COMMS_INIT_GOT[2]=make_jump_got>>8&0xff;
                  GAMESHARK_COMMS_INIT_GOT[3]=make_jump_got>>0&0xff;
            
                  if(Upload(GAMESHARK_COMMS_INIT_GOT,4,GLOBAL_OFFSET_TABLE+GOT_OFFSET))
                  {  
                       printf("GOT patch failed...\n");
                       Out32(LPT1, 0);
                       return;    
                  }
                  
                printf("0x%08x: void gscomms_init(void) - GOT: %x\n", address, GLOBAL_OFFSET_TABLE+GOT_OFFSET);
                GOT_OFFSET+=4;
             }
             
             if(codebuf_pre[i]==GAMESHARK_CHECK_GSBUTTON)
             {
                 int address = UPLOAD_ADDR-sizeof(codebuf_pre)+i*4+4;
                 unsigned long make_jump_got = J(address);
                 GAMESHARK_CHECK_GSBUTTON_GOT[0]=make_jump_got>>24&0xff;
                 GAMESHARK_CHECK_GSBUTTON_GOT[1]=make_jump_got>>16&0xff;
                 GAMESHARK_CHECK_GSBUTTON_GOT[2]=make_jump_got>>8&0xff;
                 GAMESHARK_CHECK_GSBUTTON_GOT[3]=make_jump_got>>0&0xff;
    
                 if(Upload(GAMESHARK_CHECK_GSBUTTON_GOT,4,GLOBAL_OFFSET_TABLE+GOT_OFFSET))
                 {  
                     printf("GOT patch failed...\n");
                     Out32(LPT1, 0);
                     return;    
                 }
               printf("0x%08x: int check_gsbutton(void) - GOT: %x\n", address, GLOBAL_OFFSET_TABLE+GOT_OFFSET);
               GOT_OFFSET+=4;
             }
        }
     printf("\n");
   return;
}
     
long GetFileSizez(FILE *input)
{
        long current, end;
        current = ftell(input);
        fseek(input, 0, 2);
        end = ftell(input);
        fseek(input, current, 0);
        return end;
}
 
int InitGSComms(void) {
        int timeout=TMOUT;
        while (GSFcn1(3) && timeout) {
                timeout--;
        }
        if (!timeout) {printf("init failed\n"); return 1;}
        if (!CheckGSPresence()) {printf("init failed2\n"); return 1;}
 
    return 0;
}
 
int CloseGSComms(void) {
        if (GSFcn1(3)) return 1;
        if (!CheckGSPresence()) return 1;
        ReadWriteByte(0x64);
 
        return 0;
}
 
int Read(unsigned char * buffer, unsigned long size, unsigned long address) {
        unsigned long c=0;
       
        if (InitGSComms()) return 1;
       
        ReadWriteByte(1);
        ReadWrite32(address);
        ReadWrite32(size);
       
        for (c=0; c < size; c++) buffer[c]=ReadWriteByte(0);
 
        for (c=0; c < 8; c++) ReadWriteByte(0);
 
        if (CloseGSComms()) return 1;
       
        return 0;
}
 
 
int Upload(const unsigned char * buffer, unsigned long size, unsigned long address) {
        unsigned long c=0;
       
        if (InitGSComms()) return 1;
 
        ReadWriteByte(2);
        ReadWrite32(address);
        ReadWrite32(size);
       
        for (c=0; c < size; c++) ReadWriteByte(buffer[c]);
 
        for (c=0; c < 8; c++) ReadWriteByte(0);
 
        if (CloseGSComms()) return 1;
       
        return 0;
}
 
int UploadFile(FILE * infile, unsigned long address) {
        unsigned long c=0;
        unsigned long size;
        char buf;
       
        if (fseek(infile,0,SEEK_END)) return 1;
        size=ftell(infile);
        rewind(infile);
 
        if (InitGSComms()) return 1;
       
    printf("Uploading File to %x | 00%%", (int)address);
 
    ReadWriteByte(2);
        ReadWrite32(address);
        ReadWrite32(size);
       
        for (c=0; c < size; c++) {
                fread(&buf,1,1,infile);
                if (c & 0x400) printf("\b\b\b%02lu%%", c*100/size);
                ReadWriteByte(buf);
        }
 
        for (c=0; c < 8; c++) ReadWriteByte(0);
 
        if (CloseGSComms()) return 1;
   
        printf("\b\b\bDone.\n");
       
        return 0;
}
 
/* my guess: used to get the GS and PC sync'd */
int GSFcn1(int x) {
        int timeout=TMOUT;
        unsigned char result;
       
        while (timeout) {
 
                result<<=4;
                result|=SendNibble(x);
 
                /* when we recieve a 6, then a 7, we're sync'd */
                if (result==0x67) break;
 
                SendNibble(x);
                SendNibble(x>>4);
 
                timeout--;
        }
 
        if (!timeout) return 1;
        return 0;
}
 
int SendNibble(int x) {
        int timeout=TMOUT,retval;
       
        Out32(LPT1,(x&0x0f)|0x10);
 
        while (timeout && Inp32(LPT1+1)&8) timeout--;
       
        retval=(Inp32(LPT1+1)&(0xf0^0x80))>>4;
 
        Out32(LPT1,0);
 
        timeout=TMOUT;
        while (timeout && Inp32(LPT1+1)&8) timeout--;
 
        if (!timeout) {/*printf("SendNibble timed out2\n");*/ return 0;}
        else return retval;
}
 
int ReadWriteNibble(int x) {
        int retval;
        Out32(LPT1,(x&0x0f)|0x10);
       
        while ((~Inp32(LPT1+1))&8);
        while ((~Inp32(LPT1+1))&8);
 
        retval=(Inp32(LPT1+1)&(0xf0^0x80))>>4;
 
        Out32(LPT1,0);
 
        while (Inp32(LPT1+1)&8);
        while (Inp32(LPT1+1)&8);
 
        return retval;
}
 
int ReadWriteByte(int x) {
        return (ReadWriteNibble(x>>4)<<4)|ReadWriteNibble(x);
}
 
unsigned long ReadWrite32(unsigned long x) {
        return ((unsigned long)ReadWriteNibble(x>>28)<<28)|
                ((unsigned long)ReadWriteNibble(x>>24)<<24)|
                ((unsigned long)ReadWriteNibble(x>>20)<<20)|
                ((unsigned long)ReadWriteNibble(x>>16)<<16)|
                ((unsigned long)ReadWriteNibble(x>>12)<<12)|
                ((unsigned long)ReadWriteNibble(x>>8)<<8)|
                ((unsigned long)ReadWriteNibble(x>>4)<<4)|
                (unsigned long)ReadWriteNibble(x);
}
 
/* perhaps this is what actually freezes operation? */
int CheckGSPresence(void) {
        int timeout=TMOUT,result;
       
        while (timeout) {
                result=(ReadWriteNibble(0x47>>4)<<4)|ReadWriteNibble(0x47);
                timeout--;
                if (result != 0x67) continue;
                result=(ReadWriteNibble(0x54>>4)<<4)|ReadWriteNibble(0x54);
                if (result == 0x74) break;
        }
 
        if (timeout) return 1;
        return 0;
}
