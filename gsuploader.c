/*
Modified version of HCS GSUpload - ppcasm
(Compiled with Dev-Cpp 4.9.9.2)
*/
 
#include <stdio.h>
#include <conio.h>
#define LPT1 0x378
#define Out32(PortAddress, data) _outp(PortAddress, data)
#define Inp32(PortAddress) _inp(PortAddress)
#define TMOUT 0x4000 //This can probably be lower, it's set this high just for testing.
#define UPLOAD_ADDR 0x80300000 //Where to upload your code, and also where execution starts after embedded code runs.
#define EMBED_ADDR UPLOAD_ADDR-80 //Where to upload the embedded setup code, this runs first.
#define INSN_PATCH_ADDR 0xA07C5C00 //GS Code Handler(uncached)
#define makejump(address) ((unsigned long )(address)<<2>>4|0x08000000)
 
int InitGSComms(void);
int CloseGSComms(void);
int Upload(const unsigned char * buffer, unsigned long size, unsigned long address);
int UploadFile(FILE * infile, unsigned long address);
int Read(unsigned char * buffer, unsigned long size, unsigned long address);
int GSFcn1(int);
int SendNibble(int);
int ReadWriteNibble(int);
int ReadWriteByte(int);
unsigned long ReadWrite32(unsigned long);
int CheckGSPresence(void);
long GetFileSizez(FILE *input);
 
int main(int argc, char ** argv)
{
 
    Out32(LPT1, 0);
       
    unsigned long instruction=makejump(EMBED_ADDR);
    unsigned char previnsn[4];
    unsigned char insn[4];
    insn[0]=instruction>>24&0xff;
    insn[1]=instruction>>16&0xff;
    insn[2]=instruction>>8&0xff;
    insn[3]=instruction>>0&0xff;
     
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
   
    printf("File Size: %d bytes...\n", GetFileSizez(infile));  
   
    unsigned char codebuf[]=
    {0x3c, 0x1a, 0x00, 0x00,  //lui $k0, 0x0000 (will be modified later)
     0x37, 0x5a, 0x00, 0x00,  //ori $k0, $k0, 0x0000 (will be modified later)
     0x00, 0x00, 0x00, 0x00,  //nop
     0x40, 0x9a, 0x70, 0x00,  //mtc0 $k0, $14
     0x00, 0x00, 0x00, 0x00,  //nop
     0x3c, 0x1b, 0x3c, 0x1a,  //lui $k1, 0x3c1a
     0x37, 0x7b, 0x80, 0x00,  //ori $k1, $k1, 0x8000
     0x00, 0x00, 0x00, 0x00,  //nop
     0x3c, 0x1a, 0xa0, 0x7c,  //lui $k0, 0xa07c
     0x37, 0x5a, 0x5c, 0x00,  //ori $k0, $k0, 0x5c00
     0x00, 0x00, 0x00, 0x00,  //nop
     0xaf, 0x5b, 0x00, 0x00,  //sw $k1, 0($k0)
     0x00, 0x00, 0x00, 0x00,  //nop
     0x3c, 0x1a, 0x80, 0x00,  //lui $k0, 0x8000
     0x37, 0x5a, 0x01, 0x20,  //ori $k0, $k0, 0x120
     0x03, 0x40, 0x00, 0x08,  //jr $k0
     0x00, 0x00, 0x00, 0x00}; //nop
 
    codebuf[2]=UPLOAD_ADDR>>24&0xff;
    codebuf[3]=UPLOAD_ADDR>>16&0xff;
    codebuf[6]=UPLOAD_ADDR>>8&0xff;
    codebuf[7]=UPLOAD_ADDR>>0&0xff;
 
    if(UploadFile(infile,UPLOAD_ADDR))
    {
       printf("Uploading file failed...\n");
       Out32(LPT1, 0);
       return 1;
    }
 
    if(Upload(codebuf, sizeof(codebuf), EMBED_ADDR))
    {
       printf("Failed to upload embedded code...\n");
       Out32(LPT1, 0);
       return 1;
    }
   
    printf("Uploaded embedded code to: 0x%08x.\n", EMBED_ADDR);
                       
    if(Upload(insn,4,INSN_PATCH_ADDR))
    {  
       printf("Instruction patch failed...\n");
       Out32(LPT1, 0);
       return 1;    
    }
   
    printf("Done.\n");
   
    Out32(LPT1, 0);
    close(infile);
        return 0;
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
       
    printf("Uploading File to %x | 00%%", UPLOAD_ADDR);
 
    ReadWriteByte(2);
        ReadWrite32(address);
        ReadWrite32(size);
       
        for (c=0; c < size; c++) {
                fread(&buf,1,1,infile);
                if (c & 0x400) printf("\b\b\b%02d%%",c*100/size);
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
       
        Out32(LPT1,x&0x0f|0x10);
 
        while (timeout && Inp32(LPT1+1)&8) timeout--;
       
        retval=(Inp32(LPT1+1)&0xf0^0x80)>>4;
 
        Out32(LPT1,0);
 
        timeout=TMOUT;
        while (timeout && Inp32(LPT1+1)&8) timeout--;
 
        if (!timeout) {/*printf("SendNibble timed out2\n");*/ return 0;}
        else return retval;
}
 
int ReadWriteNibble(int x) {
        int retval;
        Out32(LPT1,x&0x0f|0x10);
       
        while ((~Inp32(LPT1+1))&8);
        while ((~Inp32(LPT1+1))&8);
 
        retval=(Inp32(LPT1+1)&0xf0^0x80)>>4;
 
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
