/*
Arnold Joseph C. Najera
Cris Villem P. Saniel
Korinne Margaret V. Sasil
Chrys Sean T. Sevilla
*/

#include <stdio.h>
#include <stdlib.h>

/* --- Global Memory Chips (32 rows by 32 columns per chip from LE5) --- */
unsigned long A1[32]={0}, A2[32]={0}, A3[32]={0}, A4[32]={0}, 
              A5[32]={0}, A6[32]={0}, A7[32]={0}, A8[32]={0};
unsigned long B1[32]={0}, B2[32]={0}, B3[32]={0}, B4[32]={0}, 
              B5[32]={0}, B6[32]={0}, B7[32]={0}, B8[32]={0};

/* --- Global I/O Memory (16 Latches and 16 Buffers from LE6) --- */
unsigned char iOData[32];     

/* --- Global Buses and Control Signals --- */
unsigned int ADDR = 0x000;      
unsigned char BUS = 0x00;       
unsigned char CONTROL = 0x00;   
unsigned char IOM = 0;          
unsigned char RW = 0;           
unsigned char OE = 0;           
unsigned char FLAGS = 0x00;     

/* --- Function Prototypes --- */
void initMemory(void);
void MainMemory(void);
void IOMemory(void);
void InputSim(void);
void SevenSegment(void);
int CU(void);
int ALU(void);
unsigned char twosComp(unsigned char data);
void setFlags(int temp_ACC);

int main(void) {
    printf("Initializing CPU + Hardware Memory + I/O Ports...\n\n");
    initMemory(); 

    if (CU() == 1) { 
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

/* --- LE5 Hardware Accurate Main Memory --- */
void MainMemory(void) {
    unsigned int col, row, cs;
    
    if (IOM == 1 && OE == 1) {
        col = ADDR & 0x001F;              
        row = (ADDR >> 5) & 0x001F;       
        cs = ADDR >> 10;                  

        if (RW == 1) { 
            if (cs == 0) {
                A1[row] = (A1[row] & ~(1UL << col)) | (((BUS >> 0) & 1UL) << col);
                A2[row] = (A2[row] & ~(1UL << col)) | (((BUS >> 1) & 1UL) << col);
                A3[row] = (A3[row] & ~(1UL << col)) | (((BUS >> 2) & 1UL) << col);
                A4[row] = (A4[row] & ~(1UL << col)) | (((BUS >> 3) & 1UL) << col);
                A5[row] = (A5[row] & ~(1UL << col)) | (((BUS >> 4) & 1UL) << col);
                A6[row] = (A6[row] & ~(1UL << col)) | (((BUS >> 5) & 1UL) << col);
                A7[row] = (A7[row] & ~(1UL << col)) | (((BUS >> 6) & 1UL) << col);
                A8[row] = (A8[row] & ~(1UL << col)) | (((BUS >> 7) & 1UL) << col);
            } else {
                B1[row] = (B1[row] & ~(1UL << col)) | (((BUS >> 0) & 1UL) << col);
                B2[row] = (B2[row] & ~(1UL << col)) | (((BUS >> 1) & 1UL) << col);
                B3[row] = (B3[row] & ~(1UL << col)) | (((BUS >> 2) & 1UL) << col);
                B4[row] = (B4[row] & ~(1UL << col)) | (((BUS >> 3) & 1UL) << col);
                B5[row] = (B5[row] & ~(1UL << col)) | (((BUS >> 4) & 1UL) << col);
                B6[row] = (B6[row] & ~(1UL << col)) | (((BUS >> 5) & 1UL) << col);
                B7[row] = (B7[row] & ~(1UL << col)) | (((BUS >> 6) & 1UL) << col);
                B8[row] = (B8[row] & ~(1UL << col)) | (((BUS >> 7) & 1UL) << col);
            }
        } 
        else if (RW == 0) { 
            if (cs == 0) {
                BUS = (((A1[row] >> col) & 1UL) << 0) |
                      (((A2[row] >> col) & 1UL) << 1) |
                      (((A3[row] >> col) & 1UL) << 2) |
                      (((A4[row] >> col) & 1UL) << 3) |
                      (((A5[row] >> col) & 1UL) << 4) |
                      (((A6[row] >> col) & 1UL) << 5) |
                      (((A7[row] >> col) & 1UL) << 6) |
                      (((A8[row] >> col) & 1UL) << 7);
            } else {
                BUS = (((B1[row] >> col) & 1UL) << 0) |
                      (((B2[row] >> col) & 1UL) << 1) |
                      (((B3[row] >> col) & 1UL) << 2) |
                      (((B4[row] >> col) & 1UL) << 3) |
                      (((B5[row] >> col) & 1UL) << 4) |
                      (((B6[row] >> col) & 1UL) << 5) |
                      (((B7[row] >> col) & 1UL) << 6) |
                      (((B8[row] >> col) & 1UL) << 7);
            }
        }
    }
}

void initMemory(void) {
    IOM = 1; RW = 1; OE = 1; 

    /* Loading CountDown Program using LE5 Hardware Decoding routing */
    ADDR=0x000; BUS=0x30; MainMemory(); ADDR=0x001; BUS=0x09; MainMemory(); // WB 0x09
    ADDR=0x002; BUS=0x0C; MainMemory(); ADDR=0x003; BUS=0x00; MainMemory(); // WM 0x400
    
    // Loop Start (0x004)
    ADDR=0x004; BUS=0x14; MainMemory(); ADDR=0x005; BUS=0x00; MainMemory(); // RM 0x400
    ADDR=0x006; BUS=0x70; MainMemory(); ADDR=0x007; BUS=0x00; MainMemory(); // SWAP
    ADDR=0x008; BUS=0x28; MainMemory(); ADDR=0x009; BUS=0x00; MainMemory(); // WIO 0x000
    
    ADDR=0x00A; BUS=0x14; MainMemory(); ADDR=0x00B; BUS=0x00; MainMemory(); // RM 0x400
    ADDR=0x00C; BUS=0x48; MainMemory(); ADDR=0x00D; BUS=0x00; MainMemory(); // WACC
    ADDR=0x00E; BUS=0x30; MainMemory(); ADDR=0x00F; BUS=0x00; MainMemory(); // WB 0x00
    ADDR=0x010; BUS=0xA0; MainMemory(); ADDR=0x011; BUS=0x20; MainMemory(); // BRE 0x020
    
    ADDR=0x012; BUS=0x14; MainMemory(); ADDR=0x013; BUS=0x00; MainMemory(); // RM 0x400
    ADDR=0x014; BUS=0x48; MainMemory(); ADDR=0x015; BUS=0x00; MainMemory(); // WACC
    ADDR=0x016; BUS=0x30; MainMemory(); ADDR=0x017; BUS=0x01; MainMemory(); // WB 0x01
    ADDR=0x018; BUS=0xE8; MainMemory(); ADDR=0x019; BUS=0x00; MainMemory(); // SUB
    ADDR=0x01A; BUS=0x58; MainMemory(); ADDR=0x01B; BUS=0x00; MainMemory(); // RACC
    ADDR=0x01C; BUS=0x0C; MainMemory(); ADDR=0x01D; BUS=0x00; MainMemory(); // WM 0x400
    ADDR=0x01E; BUS=0x18; MainMemory(); ADDR=0x01F; BUS=0x04; MainMemory(); // BR 0x004
    
    // End (0x020)
    ADDR=0x020; BUS=0xF8; MainMemory(); ADDR=0x021; BUS=0x00; MainMemory(); // EOP
}

/* --- LE6 I/O Memory (Latches and Buffers) --- */
void IOMemory(void) {
    if (OE) {
        if (RW == 1 && IOM == 0) { 
            if (ADDR >= 0x000 && ADDR <= 0x00F) {
                iOData[ADDR] = BUS;
            }
        } 
        else if (RW == 0 && IOM == 0) { 
            if (ADDR >= 0x010 && ADDR <= 0x01F) {
                BUS = iOData[ADDR];
            }
        }
    }
}

/* --- LE6 Input Simulation --- */
void InputSim(void) { 
    unsigned char data; 
    int i;
    iOData[0x01F] = 0x00; // Clear buffer before writing simulated bits
    
    for(i = 7; i >= 0; i--) { 
        data = iOData[0x001];    
        data = data >> i; 
        data = data & 0x01; 
        data = data << (7 - i); 
        iOData[0x01F] = iOData[0x01F] | data; 
    } 
}

/* --- LE6 7-Segment Decoder --- */
void SevenSegment()
{
    if(iOData[ADDR]==0x01)
    {
        printf("    X\n");
        printf("    X\n");
        printf("    X\n");
        printf("    X\n");
        printf("    X\n");
        printf("    X\n");
        printf("    X\n");
    }    
    else if(iOData[ADDR]==0x02)
    {
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf(" XXXXX\n");
        printf(" X    \n");
        printf(" X    \n");
        printf(" XXXXX\n");
    }    
    else if(iOData[ADDR]==0x03)
    {
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf(" XXXXX\n");
    }    
    else if(iOData[ADDR]==0x04)
    {
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf("     X\n");
    }
    else if(iOData[ADDR]==0x05)
    {
        printf(" XXXXX\n");
        printf(" X    \n");
        printf(" X    \n");
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf(" XXXXX\n");
    }    
    else if(iOData[ADDR]==0x06)
    {
        printf(" XXXXX\n");
        printf(" X    \n");
        printf(" X    \n");
        printf(" XXXXX\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
    }
    else if(iOData[ADDR]==0x07)
    {
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf("     X\n");
        printf("     X\n");
        printf("     X\n");
        printf("     X\n");
    }  
    else if(iOData[ADDR]==0x08)
    {
        printf(" XXXXX\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
    }
    else if(iOData[ADDR]==0x09)
    {
        printf(" XXXXX\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
        printf("     X\n");
        printf("     X\n");
        printf(" XXXXX\n");
    }
    else if(iOData[ADDR]==0x00)
    {
        printf(" XXXXX\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" X   X\n");
        printf(" XXXXX\n");
    }
   // getc(stdin);
}


unsigned char twosComp(unsigned char data) {
    return (~data) + 1;
}

void setFlags(int temp_ACC) {
    FLAGS = 0x00; 
    if ((temp_ACC & 0xFF) == 0) FLAGS |= 0x01;         
    if (temp_ACC > 0xFF || temp_ACC < 0) FLAGS |= 0x02;
    if ((temp_ACC & 0x80) != 0) FLAGS |= 0x04;         
}

int ALU(void) {
    static int ACC = 0; 
    int temp_ACC = 0;
    unsigned char temp_OP2;

    if (CONTROL == 0x1E || CONTROL == 0x1D || CONTROL == 0x14 || CONTROL == 0x13 || CONTROL == 0x12 || CONTROL == 0x11) {
        if (CONTROL == 0x1D || CONTROL == 0x14 || CONTROL == 0x13 || CONTROL == 0x12 || CONTROL == 0x11) {
            temp_OP2 = twosComp(BUS);
        } else {
            temp_OP2 = BUS;
        }

        temp_ACC = (int)ACC + temp_OP2;
        setFlags(temp_ACC);

        unsigned char sign_ACC = ACC & 0x80;
        unsigned char sign_BUS = BUS & 0x80;
        unsigned char sign_RES = temp_ACC & 0x80;
        
        if (CONTROL == 0x1E) { 
            if ((sign_ACC == sign_BUS) && (sign_ACC != sign_RES)) FLAGS |= 0x80; 
        } else { 
            if ((sign_ACC != sign_BUS) && (sign_ACC != sign_RES)) FLAGS |= 0x80; 
        }

        ACC = (unsigned char)temp_ACC;
    } 
    else if (CONTROL == 0x1B) { 
        temp_ACC = ACC * BUS;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
        if (temp_ACC > 0xFF) FLAGS |= 0x80; 
    }
    else if (CONTROL == 0x1A) { 
        ACC = ACC & BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x19) { 
        ACC = ACC | BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x18) { 
        ACC = (~ACC) & 0xFF;
        setFlags(ACC);
    }
    else if (CONTROL == 0x17) { 
        ACC = ACC ^ BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x16) { 
        temp_ACC = ACC << 1;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
    }
    else if (CONTROL == 0x15) { 
        unsigned char lsb = ACC & 0x01;
        ACC = ACC >> 1;
        setFlags(ACC);
        if (lsb) FLAGS |= 0x02; 
    }
    else if (CONTROL == 0x09) { 
        ACC = BUS; 
    }

    return ACC;
}

int CU(void) {
    unsigned int PC = 0x000;
    unsigned int IR = 0x0000;
    unsigned int MAR = 0x000;
    unsigned char MBR = 0x00;
    unsigned int IOAR = 0x000;
    unsigned char IOBR = 0x00;

    unsigned char Fetch_sig;
    unsigned char IO_sig;
    unsigned char Memory_sig;

    unsigned int inst_code;
    unsigned int operand;

    while (1) {
        /* 1. FETCH CYCLE */
        CONTROL = 0x00; IOM = 1; RW = 0; OE = 1;
        Fetch_sig = 1; IO_sig = 0; Memory_sig = 0;

        ADDR = PC; MainMemory();
        if (Fetch_sig == 1) {
            IR = (unsigned int)BUS; 
            IR = IR << 8;           
            PC++;                   
            ADDR = PC;              
        }

        MainMemory();
        if (Fetch_sig == 1) {
            IR = IR | BUS;          
            PC++;                   
        }

        /* 2. DECODE CYCLE */
        inst_code = IR >> 11;
        operand = IR & 0x07FF;

        /* 3. EXECUTE CYCLE */
        Fetch_sig = 0;

        switch (inst_code) {
            case 0x01: // WM
                MAR = operand; Memory_sig = 1; CONTROL = inst_code; IOM = 1; RW = 1; OE = 1; ADDR = MAR;
                if (Memory_sig) BUS = MBR;
                MainMemory();
                break;
            case 0x02: // RM
                MAR = operand; Memory_sig = 1; CONTROL = inst_code; IOM = 1; RW = 0; OE = 1; ADDR = MAR;
                MainMemory();
                if (Memory_sig) MBR = BUS;
                break;
            case 0x03: // BR
                PC = operand;
                break;
            case 0x04: // RIO
                IOAR = operand; IO_sig = 1; CONTROL = inst_code; IOM = 0; RW = 0; OE = 1; ADDR = IOAR;
                IOMemory();
                if (IO_sig) IOBR = BUS;
                break;
            case 0x05: // WIO
                IOAR = operand; IO_sig = 1; CONTROL = inst_code; IOM = 0; RW = 1; OE = 1; ADDR = IOAR;
                if (IO_sig) BUS = IOBR;
                IOMemory();
                
                // Hardware Simulation Hooks
                if (IOAR == 0x000) {
                    SevenSegment();
                    printf("--- Seven Segment Display Updated with Value: 0x%02X ---\n", IOBR); 
                } else if (IOAR == 0x001) {
                    InputSim();     
                }
                break;
            case 0x06: // WB
                MBR = (unsigned char)(operand & 0xFF);
                break;
            case 0x07: // WIB
                IOBR = (unsigned char)(operand & 0xFF);
                break;
            case 0x09: // WACC
                BUS = MBR; 
                CONTROL = inst_code; 
                ALU(); 
                break;
            case 0x0B: // RACC
                CONTROL = inst_code;
                MBR = ALU(); 
                break;
            case 0x0E: // SWAP
                {
                    unsigned char temp = MBR;
                    MBR = IOBR;
                    IOBR = temp;
                }
                break;

            case 0x1E: case 0x1D: case 0x1B: case 0x1A: 
            case 0x19: case 0x18: case 0x17: case 0x16: case 0x15:
                Memory_sig = 1; IO_sig = 0; 
                CONTROL = inst_code; IOM = 0; RW = 0; OE = 0;
                if (Memory_sig) BUS = MBR; 
                ALU();
                break;

            case 0x14: // BRE
            case 0x13: // BRNE
            case 0x12: // BRGT
            case 0x11: // BRLT
                Memory_sig = 1; CONTROL = inst_code; 
                if (Memory_sig) BUS = MBR;
                ALU(); 

                if (inst_code == 0x14 && (FLAGS & 0x01)) PC = operand;      
                if (inst_code == 0x13 && !(FLAGS & 0x01)) PC = operand;     
                if (inst_code == 0x12 && !(FLAGS & 0x04)) PC = operand;     
                if (inst_code == 0x11 && (FLAGS & 0x04)) PC = operand;      
                break;

            case 0x1F: // EOP
                printf("\n--- Zero Encountered. End of Program. ---\n");
                return 1;
        }
    }
}