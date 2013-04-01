.origin 0
.entrypoint INIT

#include "../sage_pru.hp"

.struct AppParams
    .u32    RunFlag
    .u32    DDRBaseAddress
    .u32    DDRBytesAvailable
.ends

#define     CUR_WORD        R5

#define     ADDR_PRURAM     R16
#define     ADDR_DDR        R17

#define     DDR_SIZE        R18

INIT:

// Enable OCP master port
// This enables the PRU to write directly to DDR
LBCO    r0, CONST_PRUCFG, 4, 4
CLR     r0, r0, 4
SBCO    r0, CONST_PRUCFG, 4, 4

// Load the address of PRU0 RAM into ADDR_PRURAM
MOV     ADDR_PRURAM, MEM_PRU_DATA0_BASE

// Get the DDR base address from pru_mem[0] and put it into R5
LBBO    ADDR_DDR, ADDR_PRURAM, OFFSET(AppParams.DDRBaseAddress), 4

// Get the DDR size from pru_mem[1] and put it into R18
LBBO    DDR_SIZE, ADDR_PRURAM, OFFSET(AppParams.DDRBytesAvailable), 4

// Devide DDR_SIZE by 4 because we're treating it as 32-bit words
LSR     DDR_SIZE, DDR_SIZE, 2

MAIN_LOOP:

// Reset the current word counter
MOV     CUR_WORD, 0

WRITE_LOOP:

// Put the current word into R0
MOV     R0, CUR_WORD

// Write R0 into DDR[CUR_WORD]
SBBO    R0, ADDR_DDR, CUR_WORD, 4

// Increment the current word counter by 4
ADD     CUR_WORD, CUR_WORD, 4

// Jump to WRITE_LOOP if CUR_WORD < DDR_SIZE
QBGT WRITE_LOOP, CUR_WORD, DDR_SIZE

// Check to see if the host is still running
LBBO r0, ADDR_PRURAM, OFFSET(AppParams.RunFlag), 4
// If not, jump to exit
QBEQ EXIT, r0, 0

// Do the loop again
JMP MAIN_LOOP

EXIT:

// Tell the host program we're DONE
MOV R31.b0, PRU0_ARM_INTERRUPT+16
HALT

