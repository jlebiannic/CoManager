#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>


//static void Dao_notImplementedError(const char *fctName);

// TODO static ?
void Dao_Init(Dao *This) {
//	This->openDB = Dao_openDB;
//	This->closeDB = Dao_closeDB;
//	This->closeDBAndExit = Dao_closeDBAndExit;
	This->logError = Dao_logError;
	This->logDebug = Dao_logDebug;
}
//
//Dao* Dao_New() {
//	Dao_notImplementedError("New_Dao");
//	return NULL;
//}
//
//int Dao_openDB(Dao *This, const char *conninfo) {
//	Dao_notImplementedError("Dao_openDB");
//	return (1);
//}
//
//void Dao_closeDB(Dao *This) {
//	Dao_notImplementedError("Dao_closeDB");
//}
//
//void Dao_closeDBAndExit(Dao *This) {
//	Dao_notImplementedError("Dao_closeDBAndExit");
//}

void Dao_logError(const char *fctName, const char *msg) {
	fprintf(stderr, "%s: %s\n", fctName, msg);
}

void Dao_logDebug(const char *fctName, const char *msg) {
#ifdef DEBUG
	printf("%s: %s\n", fctName, msg);
#endif
}

//static void Dao_notImplementedError(const char *fctName) {
//	fprintf(stderr, "%s not implemented", fctName);
//	exit(1);
//}
