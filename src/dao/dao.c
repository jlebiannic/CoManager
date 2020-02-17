#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>


//static void Dao_notImplementedError(const char *fctName);

// TODO static ?
void Dao_Init(Dao *This) {
	This->logError = Dao_logError;
	This->logDebug = Dao_logDebug;
}

void Dao_logError(const char *fctName, const char *msg) {
	fprintf(stderr, "%s: %s\n", fctName, msg);
}

void Dao_logDebug(const char *fctName, const char *msg) {
#ifdef DEBUG
	printf("%s: %s\n", fctName, msg);
#endif
}
