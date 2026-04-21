/*
Arnold Joseph C. Najera
Cris Villem P. Saniel
Korinne Margaret V. Sasil
Chrys Sean T. Sevilla
*/

#include <stdio.h>
#include <stdlib.h>

/* --- Global Memory Arrays --- */
unsigned char dataMemory[2048]; // 11-bit address space
unsigned char ioBuffer[32];     // 5-bit address space

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
    printf("Initializing Main Memory for ALU-CU Integration...\n");
    initMemory(); 

    if (CU() == 1) { 
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

void initMemory(void) {
    /* Test Assembly Program (Appendix A) */
    dataMemory[0x000] = 0x30; dataMemory[0x001] = 0x15; // WB 0x15
    dataMemory[0x002] = 0x0C; dataMemory[0x003] = 0x00; // WM 0x400
    dataMemory[0x004] = 0x30; dataMemory[0x005] = 0x05; // WB 0x05
    dataMemory[0x006] = 0x48; dataMemory[0x007] = 0x00; // WACC
    dataMemory[0x008] = 0x30; dataMemory[0x009] = 0x08; // WB 0x08
    dataMemory[0x00A] = 0xF0; dataMemory[0x00B] = 0x00; // ADD
    dataMemory[0x00C] = 0x14; dataMemory[0x00D] = 0x00; // RM 0x400
    dataMemory[0x00E] = 0xD8; dataMemory[0x00F] = 0x00; // MUL
    dataMemory[0x010] = 0x58; dataMemory[0x011] = 0x00; // RACC
    dataMemory[0x012] = 0x0C; dataMemory[0x013] = 0x01; // WM 0x401
    dataMemory[0x014] = 0x38; dataMemory[0x015] = 0x0B; // WIB 0x0B
    dataMemory[0x016] = 0x28; dataMemory[0x017] = 0x00; // WIO 0x000
    dataMemory[0x018] = 0x30; dataMemory[0x019] = 0x10; // WB 0x10
    dataMemory[0x01A] = 0xE8; dataMemory[0x01B] = 0x00; // SUB
    dataMemory[0x01C] = 0x58; dataMemory[0x01D] = 0x00; // RACC
    dataMemory[0x01E] = 0x28; dataMemory[0x01F] = 0x01; // WIO 0x001
    dataMemory[0x020] = 0xB0; dataMemory[0x021] = 0x00; // SHL
    dataMemory[0x022] = 0xB0; dataMemory[0x023] = 0x00; // SHL
    dataMemory[0x024] = 0x14; dataMemory[0x025] = 0x01; // RM 0x401
    dataMemory[0x026] = 0xA8; dataMemory[0x027] = 0x00; // SHR
    dataMemory[0x028] = 0xC8; dataMemory[0x029] = 0x00; // OR
    dataMemory[0x02A] = 0xC0; dataMemory[0x02B] = 0x00; // NOT
    dataMemory[0x02C] = 0x20; dataMemory[0x02D] = 0x01; // RIO 0x001
    dataMemory[0x02E] = 0x70; dataMemory[0x02F] = 0x00; // SWAP
    dataMemory[0x030] = 0xB8; dataMemory[0x031] = 0x00; // XOR
    dataMemory[0x032] = 0x30; dataMemory[0x033] = 0xFF; // WB 0xFF
    dataMemory[0x034] = 0xD0; dataMemory[0x035] = 0x00; // AND
    dataMemory[0x036] = 0x14; dataMemory[0x037] = 0x01; // RM 0x401
    dataMemory[0x038] = 0xA0; dataMemory[0x039] = 0x3C; // BRE 0x03C
    dataMemory[0x03A] = 0x0C; dataMemory[0x03B] = 0xF0; // WM 0xF0
    dataMemory[0x03C] = 0x90; dataMemory[0x03D] = 0x40; // BRGT 0x040
    dataMemory[0x03E] = 0x88; dataMemory[0x03F] = 0x44; // BRLT 0x044
    dataMemory[0x040] = 0x30; dataMemory[0x041] = 0x00; // WB 0x00
    dataMemory[0x042] = 0x48; dataMemory[0x043] = 0x00; // WACC
    dataMemory[0x044] = 0x30; dataMemory[0x045] = 0x03; // WB 0x03
    dataMemory[0x046] = 0x48; dataMemory[0x047] = 0x00; // WACC
    
    // Loop
    dataMemory[0x048] = 0x30; dataMemory[0x049] = 0x00; // WB 0x00
    dataMemory[0x04A] = 0xA0; dataMemory[0x04B] = 0x52; // BRE 0x052
    dataMemory[0x04C] = 0x30; dataMemory[0x04D] = 0x01; // WB 0x01
    dataMemory[0x04E] = 0xE8; dataMemory[0x04F] = 0x00; // SUB
    dataMemory[0x050] = 0x18; dataMemory[0x051] = 0x48; // BR 0x048
    dataMemory[0x052] = 0xF8; dataMemory[0x053] = 0x00; // EOP
}

void MainMemory(void) {
    if (IOM == 1) {
        if (RW == 0 && OE == 1) BUS = dataMemory[ADDR];
        else if (RW == 1 && OE == 1) dataMemory[ADDR] = BUS;
    }
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
    if ((temp_ACC & 0xFF) == 0) FLAGS |= 0x01;         // Zero Flag (ZF) - Bit 0
    if (temp_ACC > 0xFF || temp_ACC < 0) FLAGS |= 0x02;// Carry Flag (CF) - Bit 1
    if ((temp_ACC & 0x80) != 0) FLAGS |= 0x04;         // Sign Flag (SF) - Bit 2
    // OF is set manually in the ALU for Add/Sub operations based on sign crossover
}

int ALU(void) {
    static int ACC = 0; // Local static accumulator
    int temp_ACC = 0;
    unsigned char temp_OP2;
    unsigned char original_ACC = ACC; // Save for printing

    printf("  [ALU] Executing Operation: ");

    // ADD or SUB or BRE/BRNE/BRGT/BRLT (all perform subtraction logic)
    if (CONTROL == 0x1E || CONTROL == 0x1D || CONTROL == 0x14 || CONTROL == 0x13 || CONTROL == 0x12 || CONTROL == 0x11) {
        
        if (CONTROL == 0x1E) printf("ADD\n");
        else if (CONTROL == 0x1D) printf("SUB\n");
        else printf("COMPARE (SUB Logic for Branching)\n");

        if (CONTROL == 0x1D || CONTROL == 0x14 || CONTROL == 0x13 || CONTROL == 0x12 || CONTROL == 0x11) {
            temp_OP2 = twosComp(BUS);
        } else {
            temp_OP2 = BUS;
        }

        temp_ACC = (int)ACC + temp_OP2;
        setFlags(temp_ACC);

        // Overflow logic for ADD/SUB
        unsigned char sign_ACC = ACC & 0x80;
        unsigned char sign_BUS = BUS & 0x80;
        unsigned char sign_RES = temp_ACC & 0x80;
        
        if (CONTROL == 0x1E) { // ADD
            if ((sign_ACC == sign_BUS) && (sign_ACC != sign_RES)) FLAGS |= 0x80; 
        } else { // SUB / Compares
            if ((sign_ACC != sign_BUS) && (sign_ACC != sign_RES)) FLAGS |= 0x80; 
        }

        // ACC is updated for ALL of these operations in this architecture!
        ACC = (unsigned char)temp_ACC;
    } 
    else if (CONTROL == 0x1B) { // MUL
        printf("MUL\n");
        temp_ACC = ACC * BUS;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
        if (temp_ACC > 0xFF) FLAGS |= 0x80; // Set OF for multiplication
    }
    else if (CONTROL == 0x1A) { // AND
        printf("AND\n");
        ACC = ACC & BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x19) { // OR
        printf("OR\n");
        ACC = ACC | BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x18) { // NOT
        printf("NOT\n");
        ACC = (~ACC) & 0xFF;
        setFlags(ACC);
    }
    else if (CONTROL == 0x17) { // XOR
        printf("XOR\n");
        ACC = ACC ^ BUS;
        setFlags(ACC);
    }
    else if (CONTROL == 0x16) { // SHL
        printf("SHL\n");
        temp_ACC = ACC << 1;
        ACC = temp_ACC & 0xFF;
        setFlags(temp_ACC);
    }
    else if (CONTROL == 0x15) { // SHR
        printf("SHR\n");
        unsigned char lsb = ACC & 0x01;
        ACC = ACC >> 1;
        setFlags(ACC);
        if (lsb) FLAGS |= 0x02; // Set Carry flag explicitly to shifted-out bit
    }
    else if (CONTROL == 0x09) { // WACC
        printf("WACC (Load BUS to ACC)\n");
        ACC = BUS; 
    }
    else if (CONTROL == 0x0B) { // RACC
        printf("RACC (Read ACC to BUS)\n");
    }
    else {
        printf("UNKNOWN (0x%02X)\n", CONTROL);
    }

    // Print the math operation results nicely
    printf("  [ALU] Operand 1 (ACC) : 0x%02X\n", original_ACC);
    
    // NOT, SHL, SHR, RACC don't use the BUS as an active math operand
    if (CONTROL != 0x18 && CONTROL != 0x16 && CONTROL != 0x15 && CONTROL != 0x0B) {
        printf("  [ALU] Operand 2 (BUS) : 0x%02X\n", BUS);
    }
    
    printf("  [ALU] Result (ACC)    : 0x%02X\n", ACC);
    printf("  [ALU] Updated FLAGS   : 0x%02X\n", FLAGS);

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

        // Data Movements & Controls
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
                BUS = MBR; // Route through MBR to BUS
                CONTROL = inst_code; 
                ALU(); // Execute WACC ALU routine to load BUS into static ACC
                break;
            case 0x0B: // RACC
                CONTROL = inst_code;
                MBR = ALU(); // Read local static ACC back via ALU function return
                break;
            case 0x0E: // SWAP
                {
                    unsigned char temp = MBR;
                    MBR = IOBR;
                    IOBR = temp;
                }
                break;

            // ALU Arithmetic / Logical Operations (All route MBR to BUS first)
            case 0x1E: case 0x1D: case 0x1B: case 0x1A: 
            case 0x19: case 0x18: case 0x17: case 0x16: case 0x15:
                Memory_sig = 1; IO_sig = 0; 
                CONTROL = inst_code; IOM = 0; RW = 0; OE = 0;
                if (Memory_sig) BUS = MBR; 
                ALU();
                break;

            // Branching / Compare Operations
            case 0x14: // BRE
            case 0x13: // BRNE
            case 0x12: // BRGT
            case 0x11: // BRLT
                Memory_sig = 1; CONTROL = inst_code; 
                if (Memory_sig) BUS = MBR;
                ALU(); // Performs compare, sets flags

                if (inst_code == 0x14 && (FLAGS & 0x01)) PC = operand;      // BRE (ZF == 1)
                if (inst_code == 0x13 && !(FLAGS & 0x01)) PC = operand;     // BRNE (ZF == 0)
                if (inst_code == 0x12 && !(FLAGS & 0x04)) PC = operand;     // BRGT (SF == 0)
                if (inst_code == 0x11 && (FLAGS & 0x04)) PC = operand;      // BRLT (SF == 1)
                break;

            case 0x1F: // EOP
                printf("--- Final Register & Bus States ---\n");
                printf("PC      : 0x%03X\n", PC);
                printf("IR      : 0x%04X\n", IR);
                printf("MBR     : 0x%02X\n", MBR);
                printf("IOBR    : 0x%02X\n", IOBR);
                printf("FLAGS   : 0x%02X\n", FLAGS);
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