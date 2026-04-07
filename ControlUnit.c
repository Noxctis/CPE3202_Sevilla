#include <stdio.h>
#include <stdlib.h>

/* Global Memory and I/O Arrays */
unsigned char dataMemory[2048]; // 8-bits wide, 2^11 cells (n=11)
unsigned char ioBuffer[32];     // 8-bits wide, 2^5 cells (n=5)

/* Function Prototypes */
void initMemory(void);
int CU(void);

int main(void) {
    printf("Initializing Main Memory...\n");
    initMemory(); // This function populates the memory

    if (CU() == 1) { // Check the return value
        printf("\nProgram run successfully!\n");
    } else {
        printf("\nError encountered, program terminated!\n");
    }

    return 0;
}

void initMemory(void) {
    /* WB 0xFF (assigns data 0xFF to MBR) */
    dataMemory[0x000] = 0x30;
    dataMemory[0x001] = 0xFF;

    /* WM 0x400 (writes data from MBR to memory address 0x400) */
    dataMemory[0x002] = 0x0C;
    dataMemory[0x003] = 0x00;

    /* RM 0x400 (reads data at address 0x400 and stores to MBR) */
    dataMemory[0x004] = 0x14;
    dataMemory[0x005] = 0x00;

    /* BR 0x12A (branch to memory address 0x12A) */
    dataMemory[0x006] = 0x19;
    dataMemory[0x007] = 0x2A;

    /* WIB 0x05 (assigns data 0x05 to IOBR) */
    dataMemory[0x12A] = 0x38;
    dataMemory[0x12B] = 0x05;

    /* WIO 0x0A (writes data from IOBR to IO buffer address 0x0A) */
    dataMemory[0x12C] = 0x28;
    dataMemory[0x12D] = 0x0A;

    /* EOP (end of program) */
    dataMemory[0x12E] = 0xF8;
    dataMemory[0x12F] = 0x00;
}

int CU(void) {
    /* Local Register Variables (Initialized to NULL / 0) */
    unsigned int PC = 0x000;   // 11-bit
    unsigned int IR = 0x0000;  // 16-bit
    unsigned int MAR = 0x000;  // 11-bit
    unsigned char MBR = 0x00;  // 8-bit
    unsigned int IOAR = 0x000; // 11-bit
    unsigned char IOBR = 0x00; // 8-bit

    unsigned int inst_code;
    unsigned int operand;

    while (1) {
        printf("****************************\n");
        printf("PC               : 0x%03X\n", PC);
        printf("Fetching instruction...\n");

        /* 1. FETCH CYCLE */
        IR = dataMemory[PC];         // Get upper byte
        IR = IR << 8;                // Move upper byte to the correct position
        PC++;                        // Point to the address of the lower byte
        IR = IR | dataMemory[PC];    // Get lower byte (16-bit instruction is now fetched)
        PC++;                        // Point to the next instruction

        /* 2. DECODE CYCLE */
        inst_code = IR >> 11;        // Extract the 5-bit instruction code
        operand = IR & 0x07FF;       // Extract the 11-bit operand

        printf("IR               : 0x%04X\n", IR);
        printf("Instruction Code : 0x%02X\n", inst_code);
        printf("Operand          : 0x%03X\n", operand);

        /* 3. EXECUTE CYCLE */
        switch (inst_code) {
            case 0x01: // WM - Write to memory
                printf("Instruction      : WM\n");
                printf("Writing data to memory...\n");
                MAR = operand;
                dataMemory[MAR] = MBR;
                break;

            case 0x02: // RM - Read from memory
                printf("Instruction      : RM\n");
                printf("Reading data from memory...\n");
                MAR = operand;
                MBR = dataMemory[MAR];
                printf("MBR              : 0x%02X\n", MBR);
                break;

            case 0x03: // BR - Branch
                printf("Instruction      : BR\n");
                printf("Branch to 0x%03X on next cycle.\n", operand);
                PC = operand;
                break;

            case 0x04: // RIO - Read from IO buffer
                printf("Instruction      : RIO\n");
                IOAR = operand;
                IOBR = ioBuffer[IOAR];
                printf("Reading from IO buffer...\n");
                break;

            case 0x05: // WIO - Write to IO buffer
                printf("Instruction      : WIO\n");
                printf("Writing to IO buffer...\n");
                IOAR = operand;
                ioBuffer[IOAR] = IOBR;
                break;

            case 0x06: // WB - Write data to MBR
                printf("Instruction      : WB\n");
                printf("Loading data to MBR...\n");
                MBR = (unsigned char)(operand & 0xFF);
                printf("MBR              : 0x%02X\n", MBR);
                break;

            case 0x07: // WIB - Write data to IOBR
                printf("Instruction      : WIB\n");
                printf("Loading data to IOBR...\n");
                IOBR = (unsigned char)(operand & 0xFF);
                printf("IOBR             : 0x%02X\n", IOBR);
                break;

            case 0x1F: // EOP - End of Program (11111 binary)
                printf("Instruction      : EOP\n");
                printf("End of program.\n");
                return 1; // Return 1 on successful EOP

            default: // Unknown instruction
                printf("Unknown instruction code encountered.\n");
                return 0; // Return 0 on error
        }
        
        /* Tip #2: Add a wait/pause before next cycle to evaluate one at a time.
           Uncomment the line below if you want the console to wait for your "Enter" key press.
        */
        // getchar(); 
    }
}