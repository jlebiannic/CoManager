#ifndef PGDAO_H
#define PGDAO_H

#include "libpq-fe.h"

typedef struct PgDao {
	int id;
	/* parent functions                               */
	int (*openDB)(struct PgDao*, const char*);
	int (*isDBOpen)(struct PgDao*);
	void (*closeDB)(struct PgDao*);
	void (*closeDBAndExit)(struct PgDao*);
	int (*execQuery)(struct PgDao*, const char*);
	void (*getEntry)(struct PgDao*, const char*, const char*, const char*);
	void (*getNextEntry)(struct PgDao*);
	int (*hasNextEntry)(struct PgDao*);
	char* (*getFieldValue)(struct PgDao*, const char*);
	int (*getFieldValueAsInt)(struct PgDao*, const char*);
	unsigned int (*newEntry)(struct PgDao*, const char *table);
	void (*clearResult)(struct PgDao*);
	void (*beginTrans)(struct PgDao*);
	void (*endTrans)(struct PgDao*);
	void (*logError)(const char*, const char*);
	void (*logDebug)(const char*, const char*);
	void (*logDebugFormat)(const char*, const char*);

	/*  member functions                              */

	/*  member datas                                  */
	PGconn *conn;
	PGresult *result; // last result's query
	int resultCurrentRow;
} PgDao;


/* Constructors  */
PgDao* PgDao_new(void);

/* overrided functions */
int PgDao_openDB(PgDao*, const char*);
int PgDao_isDBOpen(PgDao*);
void PgDao_closeDB(PgDao*);
void PgDao_closeDBAndExit(PgDao*);
int PgDao_execQuery(PgDao *This, const char *query);
void PgDao_getEntry(PgDao*, const char*, const char*, const char*);
void PgDao_getNextEntry(PgDao*);
int PgDao_hasNextEntry(PgDao*);
char* PgDao_getFieldValue(PgDao *This, const char *fieldName);
int PgDao_getFieldValueAsInt(PgDao *This, const char *fieldName);
unsigned int PgDao_newEntry(PgDao *This, const char *table);
void PgDao_clearResult(PgDao*);
void PgDao_beginTrans(PgDao*);
void PgDao_endTrans(PgDao*);


#endif
