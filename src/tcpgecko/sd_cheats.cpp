#include <stdio.h> // snprintf
#include <string.h> // memcpy, memset
#include "../utils/logger.h"
#include "../fs/sd_fat_devoptab.h"
#include "../fs/fs_utils.h"
#include "../dynamic_libs/os_functions.h"
#include "tcp_gecko.h"
#include "../kernel/syscalls.h"
#include <fat.h>
#include <iosuhax.h>

#define CODE_HANDLER_ENABLED_ADDRESS 0x10014EFC
#define CODE_LIST_START_ADDRESS 0x01133000

#define EXTENSION_SIZE 6
#define SD_FILE_PATH_HEADER_LENGTH 10
#define TITLE_ID_LEADING_ZEROS 3
#define TITLE_ID_LENGTH 16
#define CODES_FILE_PATH_SIZE (SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LENGTH + EXTENSION_SIZE)

u64 cachedTitleID;

unsigned char *kernelCopyBufferOld2[DATA_BUFFER_SIZE];

void kernelCopyData2(unsigned char *destinationBuffer, unsigned char *sourceBuffer, unsigned int length) {
	if (length > DATA_BUFFER_SIZE) {
		OSFatal("Kernel copy buffer size exceeded");
	}

	memcpy(kernelCopyBufferOld2, sourceBuffer, length);
	SC0x25_KernelCopyData((unsigned int) OSEffectiveToPhysical(destinationBuffer),
						  (unsigned int) &kernelCopyBufferOld2,
						  length);
	DCFlushRange(destinationBuffer, (u32) length);
}

void setCodeHandlerEnabled(bool enabled) {
	unsigned int *codeHandlerEnabled = (unsigned int *) CODE_HANDLER_ENABLED_ADDRESS;
	*codeHandlerEnabled = (unsigned int) enabled;
	log_printf("Code handler status: %i\n", enabled);
}

/*void testMount() {
	int res = IOSUHAX_Open(NULL);
	log_printf("Result: %i", res);

	if (res < 0) {//
		mount_sd_fat("sd"); // Fallback to normal OS implementation
	} else {
		fatInitDefault(); // using libfat
	}

	log_print("Unmounting...");
	fatUnmount("sd");
	fatUnmount("usb");
	log_print("Closing...");
	IOSUHAX_Close();
	log_print("DONE");
}*/

void considerApplyingSDCheats() {
	u64 currentTitleID = OSGetTitleID();

	// testMount();

	if (cachedTitleID == currentTitleID) {
		// log_print("Title ID NOT changed\n");
	} else {
		log_print("Title ID changed\n");
		cachedTitleID = currentTitleID;
		log_print("Mounting...\n");
		int result = mount_sd_fat("sd");

		if (result < 0) {
			log_printf("Mounting error: %i\n", result);
			return;
		} else {
			log_print("Mounted!\n");
		}

		// Construct the file path
		unsigned char filePath[CODES_FILE_PATH_SIZE];
		memset(filePath, '0', sizeof(filePath));
		memcpy(filePath, "sd:/codes/", SD_FILE_PATH_HEADER_LENGTH); // File path header
		char asciiTitleID[TITLE_ID_LENGTH];
		snprintf(asciiTitleID, TITLE_ID_LENGTH, "%llX", currentTitleID);
		log_printf("Title ID: %s\n", asciiTitleID);
		memcpy(filePath + SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LEADING_ZEROS, asciiTitleID,
			   TITLE_ID_LENGTH); // Title ID
		memcpy(filePath + SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LENGTH, ".gctu", EXTENSION_SIZE); // Extension
		filePath[CODES_FILE_PATH_SIZE - 1] = '\0'; // Null-terminated
		log_printf("File Path: %s\n", filePath);

		unsigned char *codes = NULL;
		u32 codesSize = 0;
		result = LoadFileToMem((const char *) filePath, &codes, &codesSize);

		if (result < 0) {
			log_printf("LoadFileToMem() error: %i\n", result);
			setCodeHandlerEnabled(false);
			// Error, we won't write any codes
			goto CLEANUP;
		}

		log_print("Copying...\n");
		kernelCopyData2((unsigned char *) CODE_LIST_START_ADDRESS, codes, (unsigned int) codesSize);
		log_print("Copied!\n");
		setCodeHandlerEnabled(true);

		CLEANUP:

		log_print("Unmounting...\n");
		result = unmount_sd_fat("sd");

		if (result < 0) {
			log_printf("Unmounting error: %i\n", result);
		} else {
			log_print("Unmouted!\n");
		}
	}
}
