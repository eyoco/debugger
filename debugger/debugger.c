#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include "instruction.h"
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

#define MAX_LINE 256

static void addBreakpoint(uint64_t address);
static void deleteBreakpoint(uint64_t address);
static void deleteAllBreakpoints(void);
static int  hasBreakpoint(uint64_t address);
uint64_t* breakpointMap; // added by me
int main(int argc, char **argv) {

  int fd;
  struct stat st;

  machine_state_t state;
  y86_instruction_t nextInstruction;
  memset(&state, 0, sizeof(state));

  char line[MAX_LINE + 1], previousLine[MAX_LINE + 1] = "";
  char *command, *parameters;
  int c;
  // breakpointMap = malloc(10000);

  // Verify that the command line has an appropriate number of
  // arguments
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s InputFilename [startingPC]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it for
  // reading and verify that the open did occur.
  fd = open(argv[1], O_RDONLY);

  if (fd < 0) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(stderr, "Failed to stat %s: %s\n", argv[1], strerror(errno));
    close(fd);
    return ERROR_RETURN;
  }

  state.programSize = st.st_size;

  // If there is a 2nd argument present it is an offset so convert it
  // to a numeric value.
  if (3 <= argc) {
    errno = 0;
    state.programCounter = strtoul(argv[2], NULL, 0);
    if (errno != 0) {
      perror("Invalid program counter on command line");
      close(fd);
      return ERROR_RETURN;
    }
    if (state.programCounter > state.programSize) {
      fprintf(stderr, "Program counter on command line (%lu) "
        "larger than file size (%lu).\n",
        state.programCounter, state.programSize);
      close(fd);
      return ERROR_RETURN;
    }
  }

  // Maps the entire file to memory. This is equivalent to reading the
  // entire file using functions like fread, but the data is only
  // retrieved on demand, i.e., when the specific region of the file
  // is needed.
  state.programMap = mmap(NULL, state.programSize, PROT_READ | PROT_WRITE,
        MAP_PRIVATE, fd, 0);
  if (state.programMap == MAP_FAILED) {
    fprintf(stderr, "Failed to map %s: %s\n", argv[1], strerror(errno));
    close(fd);
    return ERROR_RETURN;
  }
  breakpointMap = malloc(state.programSize * sizeof(uint64_t));

  // Move to first non-zero byte
  while (!state.programMap[state.programCounter]) state.programCounter++;

  printf("# Opened %s, starting PC 0x%lX\n", argv[1], state.programCounter);

  fetchInstruction(&state, &nextInstruction);
  printInstruction(stdout, &nextInstruction);

  while(1) {
    // Show prompt, but only if input comes from a terminal
    if (isatty(STDIN_FILENO))
      printf("> ");

    // Read one line, if EOF break loop
    if (!fgets(line, sizeof(line), stdin))
      break;

    // If line could not be read entirely
    if (!strchr(line, '\n')) {
      // Read to the end of the line
      while ((c = fgetc(stdin)) != EOF && c != '\n');
      if (c == '\n') {
  			printErrorCommandTooLong(stdout);
  			continue;
      }
      else {
  // In this case there is an EOF at the end of a line.
  // Process line as usual.
      }
    }

    // Obtain the command name, separate it from the arguments.
    command = strtok(line, " \t\n\f\r\v");
    // If line is blank, repeat previous command.
    if (!command) {
      strcpy(line, previousLine);
      command = strtok(line, " \t\n\f\r\v");
      // If there's no previous line, do nothing.
      if (!command) continue;
    }

    // Get the arguments to the command, if provided.
    parameters = strtok(NULL, "\n\r");

    sprintf(previousLine, "%s %s\n", command, parameters ? parameters : "");

    /* THIS PART TO BE COMPLETED BY THE STUDENT */
    // switch statement for instruction (mentioned assignment description)
    if (strcasecmp(command, "quit") == 0){

      // system("exit");
    	break;

    } else if(strcasecmp(command, "exit") == 0){

      break;

    } else if(strcasecmp(command, "step") == 0){

        executeInstruction(&state, &nextInstruction);

        if(nextInstruction.icode == I_HALT){
          state.programCounter = state.programCounter;
        } else if(nextInstruction.icode == I_INVALID){
          printErrorInvalidInstruction(stdout, &nextInstruction);
          state.programCounter = state.programCounter;
        } else {
            fetchInstruction(&state, &nextInstruction);
            printInstruction(stdout, &nextInstruction);
        }
    } else if(strcasecmp(command, "run") == 0){
        while(nextInstruction.icode != I_HALT && hasBreakpoint(state.programCounter) != 1 && nextInstruction.icode != I_INVALID){
          fetchInstruction(&state, &nextInstruction);
          executeInstruction(&state, &nextInstruction);
          printInstruction(stdout, &nextInstruction);
        }
        if(nextInstruction.icode == I_INVALID){
            printErrorInvalidInstruction(stdout, &nextInstruction);
        }

    } else if(strcasecmp(command, "next") == 0){

        if(nextInstruction.icode != I_CALL){
            executeInstruction(&state, &nextInstruction);
            if(nextInstruction.icode == I_HALT){
              state.programCounter = state.programCounter;
            } else if(nextInstruction.icode == I_INVALID){
              printErrorInvalidInstruction(stdout, &nextInstruction);
              state.programCounter = state.programCounter;
            } else {
                fetchInstruction(&state, &nextInstruction); 
                printInstruction(stdout, &nextInstruction);
            }
         } else if(nextInstruction.icode == I_CALL){
            uint64_t rspStored = state.registerFile[4];
            while(nextInstruction.icode != I_HALT && hasBreakpoint(state.programCounter) != 1 && nextInstruction.icode != I_INVALID){
              if(state.registerFile[4] == rspStored){
                return 1;//???
              }
              executeInstruction(&state, &nextInstruction);
              printInstruction(stdout, &nextInstruction);
            }
            printErrorInvalidInstruction(stdout, &nextInstruction);
        }
    } else if(strcasecmp(command, "jump") == 0){
        state.programCounter = strtoul(parameters, NULL, 0);
        if(state.programMap[state.programCounter] != NULL){
          fetchInstruction(&state, &nextInstruction);
          printInstruction(stdout, &nextInstruction);
        } else if(state.programMap[state.programCounter] == NULL){
          printErrorInvalidMemoryLocation(stdout, &nextInstruction, state.programCounter);
        }

    } else if(strcasecmp(command, "break") == 0){
        if(!hasBreakpoint(strtoul(parameters, NULL, 0))){
          addBreakpoint(strtoul(parameters, NULL, 0));
        }
    } else if(strcasecmp(command, "delete") == 0){

        if(hasBreakpoint(strtoul(parameters, NULL, 0))){
          deleteBreakpoint(strtoul(parameters, NULL, 0));
        }

    } else if(strcasecmp(command, "registers") == 0){

        for(int count = 0; count < 15; count = count + 1){
              switch(count){
                case 0: 
                	printRegisterValue(stdout, &state, R_RAX);
                	break;
                case 1: 
                	printRegisterValue(stdout, &state, R_RCX);
                	break;
                case 2: 
                	printRegisterValue(stdout, &state, R_RDX);
                	break;
                case 3: 
                	printRegisterValue(stdout, &state, R_RBX);
                	break;
                case 4: 
                	printRegisterValue(stdout, &state, R_RSP);
                	break;
                case 5: 
                	printRegisterValue(stdout, &state, R_RBP);
                	break;
                case 6: 
                	printRegisterValue(stdout, &state, R_RSI);
                	break;
                case 7: 
                	printRegisterValue(stdout, &state, R_RDI);
                	break;
                case 8: 
                	printRegisterValue(stdout, &state, R_R8);
                	break;
                case 9: 
                	printRegisterValue(stdout, &state, R_R9);
                	break;
                case 10: 
                	printRegisterValue(stdout, &state, R_R10);
                	break;
                case 11: 
                	printRegisterValue(stdout, &state, R_R11);
                	break;
                case 12: 
                	printRegisterValue(stdout, &state, R_R12);
                	break;
                case 13: 
                	printRegisterValue(stdout, &state, R_R13);
                	break;
                case 14: 
                	printRegisterValue(stdout, &state, R_R14);
                	break;
              }
           }  

    } else if(strcasecmp(command, "examine") == 0){

        printMemoryValueQuad(stdout, &state, strtoul(parameters, NULL, 0));

    } else{
        printErrorInvalidCommand(stdout, command, parameters); 
    }
  }
  deleteAllBreakpoints();
  munmap(state.programMap, state.programSize);
  close(fd);
  return SUCCESS;
}

/* Adds an address to the list of breakpoints. If the address is
 * already in the list, it is not added again. */
static void addBreakpoint(uint64_t address) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  if(!hasBreakpoint(address)){
    breakpointMap[address] = 1;
  }
 }

/* Deletes an address from the list of brelakpoints. If the address is
 * not in the list, nothing happens. */
static void deleteBreakpoint(uint64_t address) {

  breakpointMap[address] = 0;
  /* THIS PART TO BE COMPLETED BY THE STUDENT */
}

/* Deletes and frees all breakpoints. */
static void deleteAllBreakpoints(void) {

	free(breakpointMap);
	int index = 0;
	while(breakpointMap[index] != NULL){
		breakpointMap[index] = NULL;
		index = index + 1;
	}
  /* THIS PART TO BE COMPLETED BY THE STUDENT */
}

/* Returns true (non-zero) if the address corresponds to a breakpoint
 * in the list of breakpoints, or false (zero) otherwise. */
static int hasBreakpoint(uint64_t address) {

  /* THIS PART TO BE COMPLETED BY THE STUDENT */
  return (breakpointMap[address] == 1);
}
