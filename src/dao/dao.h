#ifndef DAO_H
#define DAO_H
#include <crtdefs.h>


typedef struct Dao {
	/*  functions member                              */
	int (*openDB)(struct Dao*, const char*);
	void (*closeDB)(struct Dao*);
	void (*closeDBAndExit)(struct Dao*);
	void (*getElement)(struct Dao*, const char*, const char*);
	char* (*getFieldValue)(struct Dao*, const char*);
	void (*addEntry)(struct Dao*, const char*, time_t);
	void (*beginTrans)(struct Dao*);
	void (*endTrans)(struct Dao*);
	void (*logError)(const char*, const char*);
	void (*logDebug)(const char*, const char*);
	void (*logDebugFormat)(char *formatAndParams, ...);

/*  datas member                                  */
} Dao;


void Dao_init(Dao*);
void Dao_logError(const char *fctName, const char *msg);
void Dao_logDebug(const char *fctName, const char *msg);
void Dao_logDebugFormat(char *formatAndParams, ...);

#endif
