#include "dao/pg/PgDao.h"
#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>


static void PgDao_init(PgDao*);
static void PgDao_setDefaultSchema(PgDao *This);
static void PgDao_addResult(PgDao *This, PGresult *res);

/******************************************************************************/

static void PgDao_init(PgDao *This) {
	This->openDB = PgDao_openDB;
	This->closeDB = PgDao_closeDB;
	This->closeDBAndExit = PgDao_closeDBAndExit;
	This->getElement = PgDao_getElement;
	This->getFieldValue = PgDao_getFieldValue;
	This->beginTrans = PgDao_beginTrans;
	This->endTrans = PgDao_endTrans;

	// parent functions
	Dao_init((Dao*) This);
}

PgDao* PgDao_new() {
	PgDao *This = malloc(sizeof(PgDao));
	if (!This) {
		This->logError("PgDao_New", "malloc failed");
		return NULL;
	}
	PgDao_init(This);
	This->conn = NULL;
	This->result = NULL;
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
	PQclear(This->result);
	PQfinish(This->conn);
}

void PgDao_closeDBAndExit(PgDao *This) {
	PQfinish(This->conn);
	exit(1);
}

void PgDao_getElement(PgDao *This, const char *table, const char *filter) {
	const char *select = "SELECT * FROM";
	const char *where = "WHERE";
	char *query = malloc(strlen(select) + strlen(table) + strlen(where) + strlen(filter) + 4);
	sprintf(query, "%s %s %s %s", select, table, where, filter);

	PgDao_addResult(This, PQexec(This->conn, query));
	free(query);
	if (PQresultStatus(This->result) != PGRES_TUPLES_OK) {
		This->logError("Query failed", PQerrorMessage(This->conn));
		PQclear(This->result);
	}
}

static void PgDao_addResult(PgDao *This, PGresult *res) {
	PQclear(This->result);
	This->result = res;
}

char* PgDao_getFieldValue(PgDao *This, const char *fieldName) {
	int nFields;
	int i;
	int numField = -1;
	char *name;
	char* fielValue = NULL;
	if (PQresultStatus(This->result)) {
		nFields = PQnfields(This->result);
		for (i = 0; i < nFields; i++) {
			name = PQfname(This->result, i);
			if (stricmp(name, fieldName) == 0) {
				numField = i;
				break;
			}
		}
		if (numField >= 0) {
			fielValue = PQgetvalue(This->result, 0, numField);
		} else {
			This->logError("PgDao_getFieldValue", "field not found");
		}
	} else {
		This->logError("PgDao_getFieldValue", "result is null");
	}
	return fielValue;
}

void PgDao_beginTrans(PgDao *This) {
	This->logDebug("PgDao_beginTrans", "start");
	PGresult *res;
	res = PQexec(This->conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		This->logError("BEGIN command failed", PQerrorMessage(This->conn));
		PQclear(res);
		This->closeDBAndExit(This);
	}
	PQclear(res);
	This->logDebug("PgDao_beginTrans", "end");
}

void PgDao_endTrans(PgDao *This) {
	This->logDebug("PgDao_endTransaction", "start");
	PGresult *res;
	/* termine la transaction */
	res = PQexec(This->conn, "END");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "END command failed: %s", PQerrorMessage(This->conn));
		PQclear(res);
		This->closeDBAndExit(This);
	}
	PQclear(res);
	This->logDebug("PgDao_endTransaction", "end");
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



