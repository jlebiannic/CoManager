#ifndef PGDAO_H
#define PGDAO_H

#include "libpq-fe.h"



typedef struct PgDao {
	/* parent functions                               */
	int (*openDB)(struct PgDao*, const char*);
	void (*closeDB)(struct PgDao*);
	void (*closeDBAndExit)(struct PgDao*);
	void (*logError)(const char*, const char*);
	void (*logDebug)(const char*, const char*);

	/*  member functions                              */

	/*  member datas                                  */
	PGconn *conn;
} PgDao;


/* Constructors  */
PgDao PgDao_Create(void);
PgDao* PgDao_New(void);

/* overrided functions */
int PgDao_openDB(PgDao*, const char*);
void PgDao_closeDB(PgDao*);
void PgDao_closeDBAndExit(PgDao*);



#endif
