#ifndef PGDAO_H
#define PGDAO_H

#include "libpq-fe.h"



typedef struct PgDao {
	/* parent functions                               */
	int (*openDB)(struct PgDao*, const char*);
	void (*closeDB)(struct PgDao*);
	void (*closeDBAndExit)(struct PgDao*);
	void (*getElement)(struct PgDao*, const char*, const char*);
	char* (*getFieldValue)(struct PgDao*, const char*);
	void (*beginTrans)(struct PgDao*);
	void (*endTrans)(struct PgDao*);
	void (*logError)(const char*, const char*);
	void (*logDebug)(const char*, const char*);

	/*  member functions                              */

	/*  member datas                                  */
	PGconn *conn;
	PGresult *result; // last result's query
} PgDao;


/* Constructors  */
PgDao* PgDao_new(void);

/* overrided functions */
int PgDao_openDB(PgDao*, const char*);
void PgDao_closeDB(PgDao*);
void PgDao_closeDBAndExit(PgDao*);
void PgDao_getElement(PgDao*, const char*, const char*);
char* PgDao_getFieldValue(PgDao *This, const char *fieldName);
void PgDao_beginTrans(PgDao*);
void PgDao_endTrans(PgDao*);


#endif
