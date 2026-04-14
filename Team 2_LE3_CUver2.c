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

/* --- Function Prototypes --- */
void initMemory(void);
void MainMemory(void);
void IOMemory(void);
int CU(void);

int main(void) {
    initMemory(); 

    if (CU() == 1) { 
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

void initMemory(void) {
    /* WB 0xFF */
    dataMemory[0x000] = 0x30;
    dataMemory[0x001] = 0xFF;

    /* WM 0x400 */
    dataMemory[0x002] = 0x0C;
    dataMemory[0x003] = 0x00;

    /* RM 0x400 */
    dataMemory[0x004] = 0x14;
    dataMemory[0x005] = 0x00;

    /* BR 0x12A */
    dataMemory[0x006] = 0x19;
    dataMemory[0x007] = 0x2A;

    /* WIB 0x05 */
    dataMemory[0x12A] = 0x38;
    dataMemory[0x12B] = 0x05;

    /* WIO 0x0A */
    dataMemory[0x12C] = 0x28;
    dataMemory[0x12D] = 0x0A;

    /* EOP */
    dataMemory[0x12E] = 0xF8;
    dataMemory[0x12F] = 0x00;
}

void MainMemory(void) {
    if (IOM == 1) {
        if (RW == 0 && OE == 1) {
            // Memory Read
            BUS = dataMemory[ADDR];
        } else if (RW == 1 && OE == 1) {
            // Memory Write
            dataMemory[ADDR] = BUS;
        }
    }
}

void IOMemory(void) {
    if (IOM == 0) {
        if (RW == 0 && OE == 1) {
            // I/O Read
            BUS = ioBuffer[ADDR];
        } else if (RW == 1 && OE == 1) {
            // I/O Write
            ioBuffer[ADDR] = BUS;
        }
    }
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
        printf("****************************\n");
        
        /* --------------------------------------
           1. FETCH CYCLE 
        --------------------------------------- */
        /* Set external control signals for fetching */
        CONTROL = 0x00; 
        IOM = 1;        // Main Memory access
        RW = 0;         // Read operation
        OE = 1;         // Allow data movement

        /* Set local control signals */
        Fetch_sig = 1;  
        IO_sig = 0;
        Memory_sig = 0;

        /* Fetch Upper Byte */
        ADDR = PC;      // PC has ownership of Address Bus
        MainMemory();
        if (Fetch_sig == 1) {
            IR = (unsigned int)BUS; 
            IR = IR << 8;           
            PC++;                   
            ADDR = PC;              
        }

        /* Fetch Lower Byte */
        MainMemory();
        if (Fetch_sig == 1) {
            IR = IR | BUS;          
            PC++;                   
        }

        /* --------------------------------------
           2. DECODE CYCLE
        --------------------------------------- */
        inst_code = IR >> 11;
        operand = IR & 0x07FF;

        /* --------------------------------------
           3. EXECUTE CYCLE
        --------------------------------------- */
        Fetch_sig = 0; // Turn off fetch signal during execution

        switch (inst_code) {
            case 0x01: // WM - Write to Memory
                MAR = operand;
                
                Memory_sig = 1;
                IO_sig = 0;
                
                CONTROL = inst_code;
                IOM = 1;
                RW = 1;
                OE = 1;
                
                ADDR = MAR;
                if (Memory_sig == 1) {
                    BUS = MBR; // MBR owns the data bus
                }
                MainMemory();
                break;

            case 0x02: // RM - Read from Memory
                MAR = operand;
                
                Memory_sig = 1;
                IO_sig = 0;
                
                CONTROL = inst_code;
                IOM = 1;
                RW = 0;
                OE = 1;
                
                ADDR = MAR;
                MainMemory();
                if (Memory_sig == 1) {
                    MBR = BUS; // Data bus loaded into MBR
                }
                break;

            case 0x03: // BR - Branch
                CONTROL = inst_code;
                PC = operand; // Internal operation, no bus activity needed
                break;

            case 0x04: // RIO - Read from I/O Buffer
                IOAR = operand;
                
                Memory_sig = 0;
                IO_sig = 1;
                
                CONTROL = inst_code;
                IOM = 0;
                RW = 0;
                OE = 1;
                
                ADDR = IOAR;
                IOMemory();
                if (IO_sig == 1) {
                    IOBR = BUS; // Data bus loaded into IOBR
                }
                break;

            case 0x05: // WIO - Write to I/O Buffer
                IOAR = operand;
                
                Memory_sig = 0;
                IO_sig = 1;
                
                CONTROL = inst_code;
                IOM = 0;
                RW = 1;
                OE = 1;
                
                ADDR = IOAR;
                if (IO_sig == 1) {
                    BUS = IOBR; // IOBR owns the data bus
                }
                IOMemory();
                break;

            case 0x06: // WB - Write Data to MBR
                CONTROL = inst_code;
                MBR = (unsigned char)(operand & 0xFF);
                break;

            case 0x07: // WIB - Write Data to IOBR
                CONTROL = inst_code;
                IOBR = (unsigned char)(operand & 0xFF);
                break;

            case 0x1F: // EOP - End of Program
                CONTROL = inst_code;
                printf("Instruction: EOP (End of Program)\n");
                printf("--- Final Register & Bus States ---\n");
                printf("PC      : 0x%03X\n", PC);
                printf("IR      : 0x%04X\n", IR);
                printf("MAR     : 0x%03X\n", MAR);
                printf("MBR     : 0x%02X\n", MBR);
                printf("IOAR    : 0x%03X\n", IOAR);
                printf("IOBR    : 0x%02X\n", IOBR);
                printf("ADDR Bus: 0x%03X\n", ADDR);
                printf("DATA Bus: 0x%02X\n", BUS);
                printf("CONTROL : 0x%02X\n", CONTROL);
                return 1;

            default:
                printf("Unknown instruction code encountered.\n");
                return 0;
        }

        /* Echo the state of the registers and buses */
        printf("--- Register & Bus States ---\n");
        printf("PC      : 0x%03X\n", PC);
        printf("IR      : 0x%04X\n", IR);
        printf("MAR     : 0x%03X\n", MAR);
        printf("MBR     : 0x%02X\n", MBR);
        printf("IOAR    : 0x%03X\n", IOAR);
        printf("IOBR    : 0x%02X\n", IOBR);
        printf("ADDR Bus: 0x%03X\n", ADDR);
        printf("DATA Bus: 0x%02X\n", BUS);
        printf("CONTROL : 0x%02X\n", CONTROL);
        printf("----------------------------\n");
        
        // Uncomment to run step-by-step
        // getchar();
    }
}