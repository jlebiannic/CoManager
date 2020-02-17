#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>


void Dao_init(Dao *This) {
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
