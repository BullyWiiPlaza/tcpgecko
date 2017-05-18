#include "dynamic_libs/os_functions.h"
#include "title.h"

bool isRunningTitleID(unsigned long long int japaneseTitleID) {
	unsigned long long int currentTitleID = (unsigned long long int) OSGetTitleID();
	return currentTitleID == japaneseTitleID // JAP
		   || currentTitleID == japaneseTitleID + 0x100 // USA
		   || currentTitleID == japaneseTitleID + 0x200; // EUR
}

bool isRunningAllowedTitleID() {
	return OSGetTitleID != 0
		   && !isRunningTitleID(TITLE_ID_MII_VERSE)
		   && !isRunningTitleID(TITLE_ID_MII_MAKER)
		   && !isRunningTitleID(TITLE_ID_BAYONETTA_2)
		   && !isRunningTitleID(TITLE_ID_INTERNET_BROWSER);
}