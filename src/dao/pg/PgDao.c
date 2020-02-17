#include "dao/pg/PgDao.h"
#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>



static void PgDao_Init(PgDao*);
static void PgDao_setDefaultSchema(PgDao *This);

/******************************************************************************/

static void PgDao_Init(PgDao *This) {
	This->openDB = PgDao_openDB;
	This->closeDB = PgDao_closeDB;
	This->closeDBAndExit = PgDao_closeDBAndExit;

	// parent functions
	This->logError = (void*) Dao_logError;
	This->logDebug = (void*) Dao_logDebug;
}

PgDao PgDao_Create() {
	PgDao This;
	PgDao_Init(&This);
	// TODO This.Free = ?;
	return This;
}

PgDao* PgDao_New() {
	PgDao *This = malloc(sizeof(PgDao));
	if (!This) {
		This->logError("PgDao_New", "malloc failed");
		return NULL;
	}
	PgDao_Init(This);
	// TODO This.Free = ?;
	return This;
}

int PgDao_openDB(PgDao *This, const char *conninfo) {
	This->logDebug("PgDao_openDB", "start");
	if (!conninfo) {
		// TODO à supprimer
		conninfo = "dbname=postgres user=postgres password=admin";
	}

	/* Crée une connexion à la base de données */
	This->conn = PQconnectdb(conninfo);

	/* Vérifier que la connexion au backend a été faite avec succès */
	if (PQstatus(This->conn) != CONNECTION_OK) {
		This->logError("Connection to database failed", PQerrorMessage(This->conn));
		This->closeDBAndExit(This);
	}

	PgDao_setDefaultSchema(This);
	This->logDebug("PgDao_openDB", "end");
	return 0;
}

void PgDao_closeDB(PgDao *This) {
	PQfinish(This->conn);
}

void PgDao_closeDBAndExit(PgDao *This) {
	PQfinish(This->conn);
	exit(1);
}

static void PgDao_setDefaultSchema(PgDao *This) {
	This->logDebug("PgDao_setDefaultSchema", "start");
	PGresult *res;
	res = PQexec(This->conn, "SELECT pg_catalog.set_config('search_path', '\"Tdx\"', false)");
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		This->logError("PgDao_setDefaultSchema", PQerrorMessage(This->conn));
		PQclear(res);
		This->closeDBAndExit(This);
	}
	This->logDebug("PgDao_setDefaultSchema", "end");
	PQclear(res);
}



