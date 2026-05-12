/*
Arnold Joseph C. Najera
Cris Villem P. Saniel
Korinne Margaret V. Sasil
Chrys Sean T. Sevilla
*/

#include <stdio.h>
#include <stdlib.h>

/* --- Global Memory Chips (32 rows by 32 columns per chip) --- */
/* Chip Group A (cs = 0) */
unsigned long A1[32]={0}, A2[32]={0}, A3[32]={0}, A4[32]={0}, 
              A5[32]={0}, A6[32]={0}, A7[32]={0}, A8[32]={0};
/* Chip Group B (cs = 1) */
unsigned long B1[32]={0}, B2[32]={0}, B3[32]={0}, B4[32]={0}, 
              B5[32]={0}, B6[32]={0}, B7[32]={0}, B8[32]={0};

unsigned char ioBuffer[32];     // 5-bit address space for I/O

/* --- Global Buses and Control Signals --- */
unsigned int ADDR = 0x000;      // 11-bit Address Bus
unsigned char BUS = 0x00;       // 8-bit Data Bus
unsigned char CONTROL = 0x00;   // 5-bit Control Signals (Instruction Code)
unsigned char IOM = 0;          // IO/M: 1 for Main Memory, 0 for I/O
unsigned char RW = 0;           // R/W: 0 for Read, 1 for Write
unsigned char OE = 0;           // Output Enable: 1 to enable bus operations
unsigned char FLAGS = 0x00;     // 8-bit FLAGS register (OF - - - - SF CF ZF)

/* --- Function Prototypes --- */
void initMemory(void);
void MainMemory(void);
void IOMemory(void);
int CU(void);
int ALU(void);
unsigned char twosComp(unsigned char data);
void setFlags(int temp_ACC);

int main(void) {
    printf("Initializing Main Memory (Hardware Decoding)...\n");
    initMemory(); 

    if (CU() == 1) { 
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

/* --- Hardware Accurate Main Memory --- */
void MainMemory(void) {
    unsigned int col, row, cs;
    
    if (IOM == 1 && OE == 1) {
        /* Decoding address data */
        col = ADDR & 0x001F;              // Extract lower 5 bits
        row = (ADDR >> 5) & 0x001F;       // Extract middle 5 bits
        cs = ADDR >> 10;                  // Extract the 11th bit (Chip Select)

        if (RW == 1) { 
            // Write Operation: Store 1 bit in each of the 8 chips in the active group
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
            // Read Operation: Extract 1 bit from each of the 8 chips and reconstruct the BUS data
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
    /* Setting the global control signals for direct memory access */
    IOM = 1; RW = 1; OE = 1; 

    /* Full LE4 Appendix A Assembly Program initialized via hardware routing */
    
    ADDR=0x000; BUS=0x30; MainMemory(); ADDR=0x001; BUS=0x15; MainMemory(); // WB 0x15
    ADDR=0x002; BUS=0x0C; MainMemory(); ADDR=0x003; BUS=0x00; MainMemory(); // WM 0x400
    ADDR=0x004; BUS=0x30; MainMemory(); ADDR=0x005; BUS=0x05; MainMemory(); // WB 0x05
    ADDR=0x006; BUS=0x48; MainMemory(); ADDR=0x007; BUS=0x00; MainMemory(); // WACC
    ADDR=0x008; BUS=0x30; MainMemory(); ADDR=0x009; BUS=0x08; MainMemory(); // WB 0x08
    ADDR=0x00A; BUS=0xF0; MainMemory(); ADDR=0x00B; BUS=0x00; MainMemory(); // ADD
    ADDR=0x00C; BUS=0x14; MainMemory(); ADDR=0x00D; BUS=0x00; MainMemory(); // RM 0x400
    ADDR=0x00E; BUS=0xD8; MainMemory(); ADDR=0x00F; BUS=0x00; MainMemory(); // MUL
    ADDR=0x010; BUS=0x58; MainMemory(); ADDR=0x011; BUS=0x00; MainMemory(); // RACC
    ADDR=0x012; BUS=0x0C; MainMemory(); ADDR=0x013; BUS=0x01; MainMemory(); // WM 0x401
    ADDR=0x014; BUS=0x38; MainMemory(); ADDR=0x015; BUS=0x0B; MainMemory(); // WIB 0x0B
    ADDR=0x016; BUS=0x28; MainMemory(); ADDR=0x017; BUS=0x00; MainMemory(); // WIO 0x000
    ADDR=0x018; BUS=0x30; MainMemory(); ADDR=0x019; BUS=0x10; MainMemory(); // WB 0x10
    ADDR=0x01A; BUS=0xE8; MainMemory(); ADDR=0x01B; BUS=0x00; MainMemory(); // SUB
    ADDR=0x01C; BUS=0x58; MainMemory(); ADDR=0x01D; BUS=0x00; MainMemory(); // RACC
    ADDR=0x01E; BUS=0x28; MainMemory(); ADDR=0x01F; BUS=0x01; MainMemory(); // WIO 0x001
    ADDR=0x020; BUS=0xB0; MainMemory(); ADDR=0x021; BUS=0x00; MainMemory(); // SHL
    ADDR=0x022; BUS=0xB0; MainMemory(); ADDR=0x023; BUS=0x00; MainMemory(); // SHL
    ADDR=0x024; BUS=0x14; MainMemory(); ADDR=0x025; BUS=0x01; MainMemory(); // RM 0x401
    ADDR=0x026; BUS=0xA8; MainMemory(); ADDR=0x027; BUS=0x00; MainMemory(); // SHR
    ADDR=0x028; BUS=0xC8; MainMemory(); ADDR=0x029; BUS=0x00; MainMemory(); // OR
    ADDR=0x02A; BUS=0xC0; MainMemory(); ADDR=0x02B; BUS=0x00; MainMemory(); // NOT
    ADDR=0x02C; BUS=0x20; MainMemory(); ADDR=0x02D; BUS=0x01; MainMemory(); // RIO 0x001
    ADDR=0x02E; BUS=0x70; MainMemory(); ADDR=0x02F; BUS=0x00; MainMemory(); // SWAP
    ADDR=0x030; BUS=0xB8; MainMemory(); ADDR=0x031; BUS=0x00; MainMemory(); // XOR
    ADDR=0x032; BUS=0x30; MainMemory(); ADDR=0x033; BUS=0xFF; MainMemory(); // WB 0xFF
    ADDR=0x034; BUS=0xD0; MainMemory(); ADDR=0x035; BUS=0x00; MainMemory(); // AND
    ADDR=0x036; BUS=0x14; MainMemory(); ADDR=0x037; BUS=0x01; MainMemory(); // RM 0x401
    ADDR=0x038; BUS=0xA0; MainMemory(); ADDR=0x039; BUS=0x3C; MainMemory(); // BRE 0x03C
    ADDR=0x03A; BUS=0x0C; MainMemory(); ADDR=0x03B; BUS=0xF0; MainMemory(); // WM 0xF0
    ADDR=0x03C; BUS=0x90; MainMemory(); ADDR=0x03D; BUS=0x40; MainMemory(); // BRGT 0x040
    ADDR=0x03E; BUS=0x88; MainMemory(); ADDR=0x03F; BUS=0x44; MainMemory(); // BRLT 0x044
    ADDR=0x040; BUS=0x30; MainMemory(); ADDR=0x041; BUS=0x00; MainMemory(); // WB 0x00
    ADDR=0x042; BUS=0x48; MainMemory(); ADDR=0x043; BUS=0x00; MainMemory(); // WACC
    ADDR=0x044; BUS=0x30; MainMemory(); ADDR=0x045; BUS=0x03; MainMemory(); // WB 0x03
    ADDR=0x046; BUS=0x48; MainMemory(); ADDR=0x047; BUS=0x00; MainMemory(); // WACC
    
    // Controlled Loop Block
    ADDR=0x048; BUS=0x30; MainMemory(); ADDR=0x049; BUS=0x00; MainMemory(); // WB 0x00
    ADDR=0x04A; BUS=0xA0; MainMemory(); ADDR=0x04B; BUS=0x52; MainMemory(); // BRE 0x052
    ADDR=0x04C; BUS=0x30; MainMemory(); ADDR=0x04D; BUS=0x01; MainMemory(); // WB 0x01
    ADDR=0x04E; BUS=0xE8; MainMemory(); ADDR=0x04F; BUS=0x00; MainMemory(); // SUB
    ADDR=0x050; BUS=0x18; MainMemory(); ADDR=0x051; BUS=0x48; MainMemory(); // BR 0x048
    ADDR=0x052; BUS=0xF8; MainMemory(); ADDR=0x053; BUS=0x00; MainMemory(); // EOP
}

void IOMemory(void) {
    if (IOM == 0) {
        if (RW == 0 && OE == 1) BUS = ioBuffer[ADDR];
        else if (RW == 1 && OE == 1) ioBuffer[ADDR] = BUS;
    }
}

unsigned char twosComp(unsigned char data) {
    return (~data) + 1;
}

void setFlags(int temp_ACC) {
    FLAGS = 0x00; // Clear flags
    if ((temp_ACC & 0xFF) == 0) FLAGS |= 0x01;         // Zero Flag (ZF)
    if (temp_ACC > 0xFF || temp_ACC < 0) FLAGS |= 0x02;// Carry Flag (CF)
    if ((temp_ACC & 0x80) != 0) FLAGS |= 0x04;         // Sign Flag (SF)
}

int ALU(void) {
    static int ACC = 0; 
    int temp_ACC = 0;
    unsigned char temp_OP2;
    unsigned char original_ACC = ACC; 

    // ADD or SUB or BRE/BRNE/BRGT/BRLT
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
    else if (CONTROL == 0x1B) { // MUL
        temp_ACC = ACC * BUS;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
        if (temp_ACC > 0xFF) FLAGS |= 0x80; 
    }
    else if (CONTROL == 0x1A) { // AND
        ACC = ACC & BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x19) { // OR
        ACC = ACC | BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x18) { // NOT
        ACC = (~ACC) & 0xFF;
        setFlags(ACC);
    }
    else if (CONTROL == 0x17) { // XOR
        ACC = ACC ^ BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x16) { // SHL
        temp_ACC = ACC << 1;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
    }
    else if (CONTROL == 0x15) { // SHR
        unsigned char lsb = ACC & 0x01;
        ACC = ACC >> 1;
        setFlags(ACC);
        if (lsb) FLAGS |= 0x02; 
    }
    else if (CONTROL == 0x09) { // WACC
        ACC = BUS; 
    }

    return ACC;
}

int CU(void) {
    /* --- Local Registers --- */
    unsigned int PC = 0x000;
    unsigned int IR = 0x0000;
    unsigned int MAR = 0x000;
    unsigned char MBR = 0x00;
    unsigned int IOAR = 0x000;
    unsigned char IOBR = 0x00;

    /* --- Local Control Lines --- */
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
                printf("\n--- End of Program Encountered ---\n");
                return 1;
        }

        /* Echo the state of the registers and buses */
        printf("--- Register & Bus States ---\n");
        printf("PC      : 0x%03X | IR      : 0x%04X\n", PC, IR);
        printf("MAR     : 0x%03X | MBR     : 0x%02X\n", MAR, MBR);
        printf("IOAR    : 0x%03X | IOBR    : 0x%02X\n", IOAR, IOBR);
        printf("ADDR Bus: 0x%03X | DATA Bus: 0x%02X\n", ADDR, BUS);
        printf("CONTROL : 0x%02X  | FLAGS   : 0x%02X\n", CONTROL, FLAGS);
        printf("----------------------------\n");
    }
}