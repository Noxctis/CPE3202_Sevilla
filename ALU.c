/*******************************************************************************
 * CpE 3202 - Computer Organization and Architecture
 * Laboratory Exercise #1: The Arithmetic and Logic Unit (ALU)
 *
 * Description: 8-bit ALU implementation in C that performs arithmetic and
 *              logic operations based on control signals from the control unit.
 ******************************************************************************/

#include <stdio.h>

/* ========================== Global Variables ============================== */
unsigned int  ACC;      /* 16-bit Accumulator (8-bit wide, 16-bit for carry)  */
unsigned char C;        /* Carry Flag                                         */
unsigned char Z;        /* Zero Flag                                          */
unsigned char OF;       /* Overflow Flag                                      */
unsigned char SF;       /* Sign Flag                                          */

/* ========================= Function Prototypes ============================ */
int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signals);
unsigned char twosComp(unsigned data);
unsigned char setFlags(unsigned int ACC);
void printBin(int data, unsigned char data_width);

/*******************************************************************************
 * twosComp - Returns the 2's complement of an 8-bit number
 * Parameter: data - the unsigned value to complement
 * Returns:   8-bit 2's complement result
 ******************************************************************************/
unsigned char twosComp(unsigned data) {
    return (unsigned char)((~data) + 1);
}

/*******************************************************************************
 * setFlags - Sets the Zero, Sign, Carry, and Overflow flags based on ACC
 * Parameter: acc - the 16-bit accumulator value
 * Returns:   combined flag byte
 ******************************************************************************/
unsigned char setFlags(unsigned int acc) {
    Z  = ((acc & 0xFF) == 0) ? 1 : 0;   /* Zero flag: result is zero        */
    SF = (acc >> 7) & 1;                  /* Sign flag: bit 7 of result       */
    C  = (acc >> 8) & 1;                  /* Carry flag: bit 8 (carry out)    */
    /* Note: OF (Overflow) is set by individual arithmetic operations         */
    return (C << 3) | (OF << 2) | (Z << 1) | SF;
}

/*******************************************************************************
 * printBin - Prints binary representation of data with specified width
 * Parameters: data       - the integer value to print
 *             data_width - number of bits to display
 ******************************************************************************/
void printBin(int data, unsigned char data_width) {
    int i;
    for (i = data_width - 1; i >= 0; i--) {
        printf("%d", (data >> i) & 1);
    }
}

/*******************************************************************************
 * ALU - Arithmetic Logic Unit
 * Performs arithmetic and logic operations based on control signals.
 *
 * Control Signals:
 *   0x01 - Addition              0x05 - OR
 *   0x02 - Subtraction           0x06 - NOT
 *   0x03 - Multiplication        0x07 - XOR
 *   0x04 - AND                   0x08 - Shift Right (logical)
 *                                0x09 - Shift Left (logical)
 *
 * Parameters: operand1        - first 8-bit operand
 *             operand2        - second 8-bit operand
 *             control_signals - operation selector
 * Returns:    result stored in ACC
 ******************************************************************************/
int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signals) {
    /* Reset flags and ACC */
    C = 0; Z = 0; OF = 0; SF = 0;
    ACC = 0;

    switch (control_signals) {

        /* ==================== ADDITION (0x01) ==================== */
        case 0x01:
        {
            printf("\n========================================\n");
            printf("            ADDITION\n");
            printf("========================================\n");
            printf("  Operand 1:   "); printBin(operand1, 8);
            printf("  (%d)\n", (signed char)operand1);
            printf("  Operand 2: + "); printBin(operand2, 8);
            printf("  (%d)\n", (signed char)operand2);
            printf("               --------\n");

            ACC = (unsigned int)operand1 + (unsigned int)operand2;

            /* Overflow: both operands same sign, result different sign */
            OF = ((~(operand1 ^ operand2)) & (operand1 ^ (unsigned char)(ACC & 0xFF)) & 0x80) ? 1 : 0;
            setFlags(ACC);

            printf("  ACC:         "); printBin(ACC & 0xFF, 8);
            printf("  (%d)\n", (signed char)(ACC & 0xFF));
            printf("\n  %d + %d = %d\n",
                   (signed char)operand1, (signed char)operand2, (signed char)(ACC & 0xFF));
            printf("  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== SUBTRACTION (0x02) ==================== */
        case 0x02:
        {
            unsigned char comp;

            printf("\n========================================\n");
            printf("          SUBTRACTION\n");
            printf("   (Addition via 2's Complement)\n");
            printf("========================================\n");
            printf("  Operand 1:   "); printBin(operand1, 8);
            printf("  (%d)\n", (signed char)operand1);
            printf("  Operand 2: - "); printBin(operand2, 8);
            printf("  (%d)\n", (signed char)operand2);

            /* Step 1: 2's complement of subtrahend */
            printf("\n  Step 1: Take 2's complement of Operand 2\n");
            printf("    Operand 2:      "); printBin(operand2, 8); printf("\n");
            printf("    1's complement: "); printBin((unsigned char)(~operand2), 8); printf("\n");
            printf("                  +        1\n");
            printf("                    --------\n");
            comp = twosComp(operand2);
            printf("    2's complement: "); printBin(comp, 8); printf("\n");

            /* Step 2: Add operand1 + 2's complement */
            printf("\n  Step 2: Add Operand 1 and 2's complement\n");
            printf("    "); printBin(operand1, 8); printf("\n");
            printf("  + "); printBin(comp, 8); printf("\n");
            printf("    --------\n");

            ACC = (unsigned int)operand1 + (unsigned int)comp;

            /* Overflow: operands different sign, result sign differs from operand1 */
            OF = (((operand1 ^ operand2) & (operand1 ^ (unsigned char)(ACC & 0xFF))) & 0x80) ? 1 : 0;
            setFlags(ACC);

            printf("    "); printBin(ACC & 0xFF, 8);
            printf("  (%d)\n", (signed char)(ACC & 0xFF));
            printf("\n  %d - %d = %d\n",
                   (signed char)operand1, (signed char)operand2, (signed char)(ACC & 0xFF));
            printf("  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== MULTIPLICATION - Booth's Algorithm (0x03) ==================== */
        case 0x03:
        {
            signed char M = (signed char)operand1;  /* Multiplicand           */
            unsigned char A = 0x00;                  /* Booth accumulator      */
            unsigned char Q = operand2;              /* Multiplier             */
            unsigned char Q_1 = 0;                   /* Q-1 bit               */
            unsigned char Q0, sign_bit;
            short product;
            int i;

            printf("\n========================================================\n");
            printf("       MULTIPLICATION (Booth's Algorithm)\n");
            printf("========================================================\n");
            printf("  Multiplicand (M): "); printBin(operand1, 8);
            printf("  (%d)\n", M);
            printf("  Multiplier   (Q): "); printBin(operand2, 8);
            printf("  (%d)\n", (signed char)operand2);

            /* Table header */
            printf("\n  %-5s | %-10s | %-10s | %-3s | %s\n",
                   "Step", "    A     ", "    Q     ", "Q-1", "Operation");
            printf("  ------|------------|------------|-----|------------------\n");

            /* Initial state */
            printf("  Init  | "); printBin(A, 8); printf("   | ");
            printBin(Q, 8); printf("   |  %d  | ", Q_1);
            printf("Initialize\n");

            /* Booth's Algorithm - 8 iterations */
            for (i = 1; i <= 8; i++) {
                Q0 = Q & 1;

                if (Q0 == 1 && Q_1 == 0) {
                    /* Q0 Q-1 = 10: Subtract multiplicand */
                    A = (unsigned char)((signed char)A - M);
                    printf("  %-5d | ", i);
                    printBin(A, 8); printf("   | ");
                    printBin(Q, 8); printf("   |  %d  | ", Q_1);
                    printf("A = A - M\n");
                } else if (Q0 == 0 && Q_1 == 1) {
                    /* Q0 Q-1 = 01: Add multiplicand */
                    A = (unsigned char)((signed char)A + M);
                    printf("  %-5d | ", i);
                    printBin(A, 8); printf("   | ");
                    printBin(Q, 8); printf("   |  %d  | ", Q_1);
                    printf("A = A + M\n");
                } else {
                    /* Q0 Q-1 = 00 or 11: No arithmetic operation */
                    printf("  %-5d | ", i);
                    printBin(A, 8); printf("   | ");
                    printBin(Q, 8); printf("   |  %d  | ", Q_1);
                    printf("No operation\n");
                }

                /* Arithmetic Shift Right {A, Q, Q-1} */
                Q_1 = Q & 1;
                Q = (Q >> 1) | ((A & 1) << 7);
                sign_bit = A & 0x80;
                A = (A >> 1) | sign_bit;

                printf("   ASR  | ");
                printBin(A, 8); printf("   | ");
                printBin(Q, 8); printf("   |  %d  | ", Q_1);
                printf("Shift right\n");
            }

            /* Combine A (high byte) and Q (low byte) for 16-bit product */
            product = (short)(((unsigned short)A << 8) | (unsigned short)Q);
            ACC = (unsigned int)(((unsigned short)A << 8) | (unsigned short)Q);

            printf("\n  Product: "); printBin((product >> 8) & 0xFF, 8);
            printf(" "); printBin(product & 0xFF, 8);
            printf("  (%d)\n", product);
            printf("\n  %d x %d = %d\n", M, (signed char)operand2, product);

            /* Set flags for multiplication */
            OF = (product > 127 || product < -128) ? 1 : 0;
            SF = (product < 0) ? 1 : 0;
            Z  = (product == 0) ? 1 : 0;
            C  = (ACC > 0xFF) ? 1 : 0;

            printf("  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================================\n");
            break;
        }

        /* ==================== AND (0x04) ==================== */
        case 0x04:
        {
            printf("\n========================================\n");
            printf("              AND\n");
            printf("========================================\n");
            printf("  Operand 1:   "); printBin(operand1, 8); printf("\n");
            printf("  Operand 2: & "); printBin(operand2, 8); printf("\n");
            printf("               --------\n");

            ACC = (unsigned int)(operand1 & operand2);
            OF = 0;
            setFlags(ACC);

            printf("  ACC:         "); printBin(ACC & 0xFF, 8); printf("\n");
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== OR (0x05) ==================== */
        case 0x05:
        {
            printf("\n========================================\n");
            printf("               OR\n");
            printf("========================================\n");
            printf("  Operand 1:   "); printBin(operand1, 8); printf("\n");
            printf("  Operand 2: | "); printBin(operand2, 8); printf("\n");
            printf("               --------\n");

            ACC = (unsigned int)(operand1 | operand2);
            OF = 0;
            setFlags(ACC);

            printf("  ACC:         "); printBin(ACC & 0xFF, 8); printf("\n");
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== NOT (0x06) ==================== */
        case 0x06:
        {
            printf("\n========================================\n");
            printf("              NOT\n");
            printf("========================================\n");
            printf("  Operand 1: ~ "); printBin(operand1, 8); printf("\n");
            printf("               --------\n");

            ACC = (unsigned int)((unsigned char)(~operand1));
            OF = 0;
            setFlags(ACC);

            printf("  ACC:         "); printBin(ACC & 0xFF, 8); printf("\n");
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== XOR (0x07) ==================== */
        case 0x07:
        {
            printf("\n========================================\n");
            printf("              XOR\n");
            printf("========================================\n");
            printf("  Operand 1:   "); printBin(operand1, 8); printf("\n");
            printf("  Operand 2: ^ "); printBin(operand2, 8); printf("\n");
            printf("               --------\n");

            ACC = (unsigned int)(operand1 ^ operand2);
            OF = 0;
            setFlags(ACC);

            printf("  ACC:         "); printBin(ACC & 0xFF, 8); printf("\n");
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== SHIFT RIGHT - Logical (0x08) ==================== */
        case 0x08:
        {
            printf("\n========================================\n");
            printf("       SHIFT RIGHT (Logical)\n");
            printf("========================================\n");
            printf("  Operand:   "); printBin(operand1, 8);
            printf("  (0x%02X)\n", operand1);
            printf("  Shift by:  %d position(s)\n", operand2);
            printf("             --------\n");

            ACC = (unsigned int)(operand1 >> operand2);
            OF = 0;
            setFlags(ACC);

            printf("  ACC:       "); printBin(ACC & 0xFF, 8);
            printf("  (0x%02X)\n", (unsigned char)(ACC & 0xFF));
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        /* ==================== SHIFT LEFT - Logical (0x09) ==================== */
        case 0x09:
        {
            printf("\n========================================\n");
            printf("       SHIFT LEFT (Logical)\n");
            printf("========================================\n");
            printf("  Operand:   "); printBin(operand1, 8);
            printf("  (0x%02X)\n", operand1);
            printf("  Shift by:  %d position(s)\n", operand2);
            printf("             --------\n");

            ACC = (unsigned int)operand1 << operand2;
            OF = 0;
            setFlags(ACC);

            printf("  ACC:       "); printBin(ACC & 0xFF, 8);
            printf("  (0x%02X)\n", (unsigned char)(ACC & 0xFF));
            printf("\n  Flags: C=%d  Z=%d  OF=%d  SF=%d\n", C, Z, OF, SF);
            printf("========================================\n");
            break;
        }

        default:
            printf("\n  Error: Invalid control signal 0x%02X\n", control_signals);
            break;
    }

    return (int)(ACC & 0xFFFF);
}

/*******************************************************************************
 * main - Test the ALU with various operations
 ******************************************************************************/
void main(void) {
    printf("============================================\n");
    printf("  CpE 3202 - Laboratory Exercise #1\n");
    printf("  The Arithmetic and Logic Unit (ALU)\n");
    printf("============================================\n");

    /* Required test cases from the lab guide */
    ALU(0x03, 0x05, 0x02);  /* 00000011 - 00000101  (Subtract)    */
    ALU(0x88, 0x85, 0x01);  /* 10001000 + 10000101  (Add)         */
    ALU(0xC0, 0x02, 0x03);  /* 11000000 * 00000010  (Multiply)    */

    /* Additional test cases for all operations */
    ALU(0x0A, 0x05, 0x01);  /* Addition:     10 + 5 = 15          */
    ALU(0x14, 0x0A, 0x02);  /* Subtraction:  20 - 10 = 10         */
    ALU(0x03, 0xFD, 0x03);  /* Multiply:     3 * (-3) = -9        */
    ALU(0xF0, 0x0F, 0x04);  /* AND:          0xF0 & 0x0F = 0x00   */
    ALU(0xAA, 0x55, 0x05);  /* OR:           0xAA | 0x55 = 0xFF   */
    ALU(0xAA, 0x00, 0x06);  /* NOT:          ~0xAA = 0x55         */
    ALU(0xFF, 0x0F, 0x07);  /* XOR:          0xFF ^ 0x0F = 0xF0   */
    ALU(0xB4, 0x02, 0x08);  /* Shift Right:  0xB4 >> 2 = 0x2D    */
    ALU(0x15, 0x03, 0x09);  /* Shift Left:   0x15 << 3 = 0xA8    */
}
