#pragma once

#include "kernel.h"

#define ENDING_ADDRESS 0x10000000
unsigned int assemblySize = 0;

unsigned int getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}

void persistAssembly(unsigned char buffer[], unsigned int size) {
	assemblySize = size;

	// Write the assembly to an executable code region
	unsigned int startAddress = getStartAddress();
	kernelCopyData((unsigned char *) startAddress, buffer, assemblySize);
}

void executeAssembly() {
	int startAddress = getStartAddress();
	log_printf("[Execute assembly] Start address: %i\n", startAddress);
	int startAddressInstruction = *(int*)startAddress;

	if (startAddressInstruction != 0) {
		void (*function)() = (void (*)()) startAddress;
		function();
		log_print("[Execute assembly] Executed!\n");
	}
}

void clearAssembly() {
	log_printf("[Clear assembly] Assembly size: %i\n", assemblySize);
	unsigned char buffer[assemblySize];
	log_print("[Clear assembly] Buffer allocated\n");
	memset((void *) buffer, 0, assemblySize);
	log_print("[Clear assembly] Memory set\n");
	int startAddress = getStartAddress();
	log_printf("[Clear assembly] Start address: %i\n", startAddress);
	kernelCopyData((unsigned char *) startAddress, buffer, assemblySize);
	log_print("[Clear assembly] Kernel copy done\n");
	assemblySize = 0;
	log_print("[Clear assembly] Assembly size 0\n");
}
