///*
// * commons.c
// *
// *  Created on: 13 févr. 2020
// *      Author: jlebiannic
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//
//#include "commons/types.h"
//
//void closeAndExit(PGconn *conn) {
//	PQfinish(conn);
//	exit(1);
//}
//
//void closeDB(PGconn *conn) {
//	PQfinish(conn);
//}
//
//
//static void setDefaultSchema(PGconn *conn) {
//	PGresult *res;
//	res = PQexec(conn, "SELECT pg_catalog.set_config('search_path', '\"Tdx\"', false)");
//	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
//		fprintf(stderr, "SET failed: %s", PQerrorMessage(conn));
//		PQclear(res);
//		closeAndExit(conn);
//	}
//	PQclear(res);
//}
//
//PGconn* openDB(const char *conninfo) {
//	PGconn *conn;
//	printf("connect\n");
//	if (!conninfo) {
//		conninfo = "dbname=postgres user=postgres password=admin";
//	}
//
//	/* Crée une connexion à la base de données */
//	conn = PQconnectdb(conninfo);
//
//	/* Vérifier que la connexion au backend a été faite avec succès */
//	if (PQstatus(conn) != CONNECTION_OK) {
//		fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
//		closeAndExit(conn);
//	}
//
//	setDefaultSchema(conn);
//
//	return conn;
//}
//
//
//void query(const char *query, PGconn *conn) {
//	PGresult *res;
//	res = PQexec(conn, query);
//	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//		fprintf(stderr, "Query '%s' failed: %s", query, PQerrorMessage(conn));
//		PQclear(res);
//		closeAndExit(conn);
//	}
//	PQclear(res);
//}
//
//void beginTransaction(PGconn *conn) {
//	printf("begin\n");
//	PGresult *res;
//	/* Démarre un bloc de transaction */
//	res = PQexec(conn, "BEGIN");
//	//fprintf(stderr, "BEGIN, PQresultStatus(res): %d", (int) PQresultStatus(res));
//	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//		fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
//		PQclear(res);
//		closeAndExit(conn);
//	}
//	PQclear(res);
//}
//
//void endTransaction(PGconn *conn) {
//	printf("end\n");
//	PGresult *res;
//	/* termine la transaction */
//	res = PQexec(conn, "END");
//	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//		fprintf(stderr, "END command failed: %s", PQerrorMessage(conn));
//		PQclear(res);
//		closeAndExit(conn);
//	}
//	PQclear(res);
//}
//
