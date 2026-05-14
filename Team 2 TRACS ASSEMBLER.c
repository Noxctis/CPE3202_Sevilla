/*
Arnold Joseph C. Najera
Cris Villem P. Saniel
Korinne Margaret V. Sasil
Chrys Sean T. Sevilla
*/

/*
.\assembler.exe CountDown.txt machine_code.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LABELS 100
#define MAX_LINE 256

/* --- Symbol Table for Labels --- */
typedef struct {
    char name[32];
    int address;
} Label;

Label symbolTable[MAX_LABELS];
int labelCount = 0;

/* --- Instruction Set Dictionary --- */
typedef struct {
    char mnemonic[10];
    unsigned int opcode; // Pre-shifted 16-bit base opcode
    int type;            // 0: No operand, 1: Data Mem, 2: IO Mem, 3: Literal, 4: Branch/Label
} Instruction;

Instruction instrSet[] = {
    {"WM",   0x0800, 1}, {"RM",   0x1000, 1}, {"RIO",  0x2000, 2}, {"WIO",  0x2800, 2},
    {"WB",   0x3000, 3}, {"WIB",  0x3800, 3}, {"WACC", 0x4800, 0}, {"RACC", 0x5800, 0},
    {"SWAP", 0x7000, 0}, {"BR",   0x1800, 4}, {"BRE",  0xA000, 4}, {"BRNE", 0x9800, 4},
    {"BRGT", 0x9000, 4}, {"BRLT", 0x8800, 4}, {"ADD",  0xF000, 0}, {"SUB",  0xE800, 0},
    {"MUL",  0xD800, 0}, {"AND",  0xD000, 0}, {"OR",   0xC800, 0}, {"NOT",  0xC000, 0},
    {"XOR",  0xB800, 0}, {"SHL",  0xB000, 0}, {"SHR",  0xA800, 0}, {"EOP",  0xF800, 0}
};
int numInstr = 24;

/* --- Helper Functions --- */
void trim(char *str) {
    char *p = str;
    int l = strlen(p);
    while(isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;
    memmove(str, p, l + 1);
}

int getInstruction(char *mnemonic) {
    for (int i = 0; i < numInstr; i++) {
        if (strcmp(instrSet[i].mnemonic, mnemonic) == 0) return i;
    }
    return -1;
}

void addLabel(char *name, int address) {
    strcpy(symbolTable[labelCount].name, name);
    symbolTable[labelCount].address = address;
    labelCount++;
}

int getLabelAddress(char *name) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) return symbolTable[i].address;
    }
    return -1;
}

/* --- Main Assembler --- */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.asm> <output.txt>\n", argv[0]);
        return 1;
    }

    FILE *inFile = fopen(argv[1], "r");
    if (!inFile) {
        printf("Error: Cannot open input file %s\n", argv[1]);
        return 1;
    }

    char line[MAX_LINE];
    int PC = 0x000;
    int hasEOP = 0;
    int errorCount = 0;

    printf("--- TRACS Assembler ---\n");
    printf("Starting PASS 1 (Resolving Labels)...\n");

    /* =========================================
       PASS 1: Resolve Labels and ORG directive
       ========================================= */
    while (fgets(line, sizeof(line), inFile)) {
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0'; // Strip comments
        trim(line);
        if (strlen(line) == 0) continue;

        char token1[32] = "", token2[32] = "", token3[32] = "";
        sscanf(line, "%s %s %s", token1, token2, token3);

        if (strcmp(token1, "ORG") == 0) {
            PC = (int)strtol(token2, NULL, 16);
            continue;
        }

        int instrIndex = getInstruction(token1);
        if (instrIndex == -1) {
            // Token 1 is not an instruction, so it must be a label!
            addLabel(token1, PC);
            if (strlen(token2) > 0) {
                if (strcmp(token2, "EOP") == 0) hasEOP = 1;
                PC += 2; // Every instruction takes 2 bytes
            }
        } else {
            // Token 1 is an instruction
            if (strcmp(token1, "EOP") == 0) hasEOP = 1;
            PC += 2;
        }
    }

    if (!hasEOP) {
        printf("ERROR: No EOP - Program does not have an End of Program instruction.\n");
        errorCount++;
    }

    /* =========================================
       PASS 2: Generate Machine Code
       ========================================= */
    rewind(inFile);
    FILE *outFile = fopen(argv[2], "w");
    PC = 0x000;
    int lineNum = 0;

    printf("Starting PASS 2 (Generating Machine Code)...\n");

    while (fgets(line, sizeof(line), inFile)) {
        lineNum++;
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';
        trim(line);
        if (strlen(line) == 0) continue;

        char token1[32] = "", token2[32] = "", token3[32] = "";
        sscanf(line, "%s %s %s", token1, token2, token3);

        if (strcmp(token1, "ORG") == 0) {
            PC = (int)strtol(token2, NULL, 16);
            continue;
        }

        char *mnemonic = token1;
        char *operandStr = token2;

        int instrIndex = getInstruction(token1);
        if (instrIndex == -1) {
            // Skip the label we already resolved in pass 1
            mnemonic = token2;
            operandStr = token3;
            instrIndex = getInstruction(mnemonic);
            
            if (instrIndex == -1 && strlen(mnemonic) > 0) {
                printf("ERROR Line %d: Unknown instruction '%s'\n", lineNum, mnemonic);
                errorCount++;
                continue;
            }
        }

        if (strlen(mnemonic) == 0) continue;

        Instruction instr = instrSet[instrIndex];
        unsigned int machine_code = instr.opcode;
        int operandVal = 0;

        // Parse Operand based on type
        if (instr.type == 0) { // No Operand
            if (strlen(operandStr) > 0) {
                printf("ERROR Line %d: Illegal operand - '%s' should not have an operand.\n", lineNum, mnemonic);
                errorCount++;
            }
        } else {
            if (strlen(operandStr) == 0) {
                printf("ERROR Line %d: Missing operand for '%s'.\n", lineNum, mnemonic);
                errorCount++;
                continue;
            }

            if (instr.type == 4) { // Branch/Label
                operandVal = getLabelAddress(operandStr);
                if (operandVal == -1) {
                    operandVal = (int)strtol(operandStr, NULL, 16); // Try parsing as raw hex if not a label
                }
            } else {
                operandVal = (int)strtol(operandStr, NULL, 16);
            }

            // Error Checking for Illegal Addresses / Out of Range
            if (instr.type == 1 && (operandVal < 0x400 || operandVal > 0x7FF)) {
                printf("ERROR Line %d: Illegal address - Data memory (0x400-0x7FF) expected.\n", lineNum);
                errorCount++;
            } else if (instr.type == 2 && (operandVal < 0x000 || operandVal > 0x01F)) {
                printf("ERROR Line %d: Illegal address - I/O memory (0x000-0x01F) expected.\n", lineNum);
                errorCount++;
            } else if (instr.type == 3 && (operandVal < 0x00 || operandVal > 0xFF)) {
                printf("ERROR Line %d: Illegal operand - 8-bit literal (0x00-0xFF) expected.\n", lineNum);
                errorCount++;
            }

            machine_code |= (operandVal & 0x07FF); // Merge 11-bit operand into instruction
        }

        // Output formatting required by TRACS
        fprintf(outFile, "ADDR=0x%03X; BUS=0x%02X; MainMemory();\n", PC, (machine_code >> 8) & 0xFF);
        fprintf(outFile, "ADDR=0x%03X; BUS=0x%02X; MainMemory();\n", PC + 1, machine_code & 0xFF);
        PC += 2;
    }

    fclose(inFile);
    fclose(outFile);

    if (errorCount > 0) {
        printf("\nAssembly failed with %d errors.\n", errorCount);
        remove(argv[2]); // Delete the corrupt output file
    } else {
        printf("\nBuild Success! Machine code written to '%s'.\n", argv[2]);
    }

    return 0;
}
