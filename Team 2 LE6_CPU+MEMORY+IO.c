/*
Arnold Joseph C. Najera
Cris Villem P. Saniel
Korinne Margaret V. Sasil
Chrys Sean T. Sevilla
*/

#include <stdio.h>
#include <stdlib.h>

/* --- Global Memory Arrays --- */
unsigned char dataMemory[2048]; // Main Memory: 11-bit address space
unsigned char iOData[32];       // I/O Memory: 16 latches (0x00-0x0F) and 16 buffers (0x10-0x1F)

/* --- Global Buses and Control Signals --- */
unsigned int ADDR = 0x000;      // 11-bit Address Bus
unsigned char BUS = 0x00;       // 8-bit Data Bus
unsigned char CONTROL = 0x00;   // 5-bit Control Signals 
unsigned char IOM = 0;          // IO/M: 1 for Main Memory, 0 for I/O
unsigned char RW = 0;           // R/W: 0 for Read, 1 for Write
unsigned char OE = 0;           // Output Enable: 1 to enable bus operations
unsigned char FLAGS = 0x00;     // 8-bit FLAGS register (OF - - - - SF CF ZF)

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
    printf("Initializing Computer Architecture for Lab 6...\n\n");
    initMemory(); 

    if (CU() == 1) { 
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

void initMemory(void) {
    /* Machine Code for CountDown Program */
    dataMemory[0x000] = 0x30; dataMemory[0x001] = 0x09; // WB 0x09
    dataMemory[0x002] = 0x0C; dataMemory[0x003] = 0x00; // WM 0x400
    
    // Loop Start
    dataMemory[0x004] = 0x14; dataMemory[0x005] = 0x00; // RM 0x400
    dataMemory[0x006] = 0x70; dataMemory[0x007] = 0x00; // SWAP
    dataMemory[0x008] = 0x28; dataMemory[0x009] = 0x00; // WIO 0x000
    
    dataMemory[0x00A] = 0x14; dataMemory[0x00B] = 0x00; // RM 0x400
    dataMemory[0x00C] = 0x48; dataMemory[0x00D] = 0x00; // WACC
    dataMemory[0x00E] = 0x30; dataMemory[0x00F] = 0x00; // WB 0x00
    dataMemory[0x010] = 0xA0; dataMemory[0x011] = 0x20; // BRE 0x020
    
    dataMemory[0x012] = 0x14; dataMemory[0x013] = 0x00; // RM 0x400
    dataMemory[0x014] = 0x48; dataMemory[0x015] = 0x00; // WACC
    dataMemory[0x016] = 0x30; dataMemory[0x017] = 0x01; // WB 0x01
    dataMemory[0x018] = 0xE8; dataMemory[0x019] = 0x00; // SUB
    dataMemory[0x01A] = 0x58; dataMemory[0x01B] = 0x00; // RACC
    dataMemory[0x01C] = 0x0C; dataMemory[0x01D] = 0x00; // WM 0x400
    dataMemory[0x01E] = 0x18; dataMemory[0x01F] = 0x04; // BR 0x004
    
    // End
    dataMemory[0x020] = 0xF8; dataMemory[0x021] = 0x00; // EOP
}

void MainMemory(void) {
    if (IOM == 1) {
        if (RW == 0 && OE == 1) {
            BUS = dataMemory[ADDR]; // Read to BUS
        } else if (RW == 1 && OE == 1) {
            dataMemory[ADDR] = BUS; // Write from BUS
        }
    }
}

void IOMemory(void) {
    if (OE) {
        if (RW == 1 && IOM == 0) { // Write (Latches)
            if (ADDR >= 0x000 && ADDR <= 0x00F) {
                iOData[ADDR] = BUS;
            }
        } 
        else if (RW == 0 && IOM == 0) { // Read (Buffers)
            if (ADDR >= 0x010 && ADDR <= 0x01F) {
                BUS = iOData[ADDR];
            }
        }
    }
}

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

        ADDR = PC; 
        MainMemory();
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
                
                // Hardware Simulations triggered on output
                if (IOAR == 0x000) {
                    SevenSegment(); // Display latch 0 data
                    printf("\n");
                } else if (IOAR == 0x001) {
                    InputSim();     // Simulate input mapping
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

                if (inst_code == 0x14 && (FLAGS & 0x01)) PC = operand;      // BRE (ZF == 1)
                if (inst_code == 0x13 && !(FLAGS & 0x01)) PC = operand;     // BRNE (ZF == 0)
                if (inst_code == 0x12 && !(FLAGS & 0x04)) PC = operand;     // BRGT (SF == 0)
                if (inst_code == 0x11 && (FLAGS & 0x04)) PC = operand;      // BRLT (SF == 1)
                break;

            case 0x1F: // EOP
                printf("\n--- Program Execution Complete ---\n");
                return 1;
        }
    }
}