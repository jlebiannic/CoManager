#ifndef PGDAO_H
#define PGDAO_H

#include "libpq-fe.h"

typedef struct PgDao {
    int id;
    /* parent functions                               */
    int (*openDB)(struct PgDao *This, const char *conn);
    int (*isDBOpen)(struct PgDao *This);
    void (*closeDB)(struct PgDao *This);
    void (*closeDBAndExit)(struct PgDao *This);
    int (*execQuery)(struct PgDao *This, const char *query);
    int (*execQueryMultiResults)(struct PgDao *This, const char *query);
    int (*execQueryParams)(struct PgDao *This, const char *queryFormat, const char *paramValues[], int nbParams);
    int (*execQueryParamsMultiResults)(struct PgDao *This, const char *query, const char *values[], int nbValues);
    unsigned int (*newEntry)(struct PgDao *This, const char *table, const char *fields[], const char *values[], int nb, const char* returnField);
    int (*updateEntries)(struct PgDao *This, const char *table, const char *fields[], const char *values[], int nb, const char *filter, const char *filterValues[]);
    int (*removeEntries)(struct PgDao *This, const char *table, const char *filter, const char *filterValues[]);
    int (*getEntries)(struct PgDao *This, const char *table, const char *fields[], int nbFields, const char *filter, const char *values[], const char *sortFields[], int nbSortFields, int limit, int offset, int cursorMode);
    int (*getEntriesCount)(struct PgDao *This, const char *table, const char *filter, const char *values[]);
    void (*getNextEntry)(struct PgDao *This);
    int (*hasNextEntry)(struct PgDao *This);
    char *(*getFieldValue)(struct PgDao *This, const char *fieldName);
    int (*getFieldValueAsInt)(struct PgDao *This, const char *fieldName);
    double (*getFieldValueAsDouble)(struct PgDao *This, const char *fieldName);
    int (*getResultNbFields)(struct PgDao *This);
    char *(*getFieldValueByNum)(struct PgDao *This, int numField);
    int (*getFieldValueAsIntByNum)(struct PgDao *This, int numField);
    double (*getFieldValueAsDoubleByNum)(struct PgDao *This, int numField);
    int (*createTable)(struct PgDao *This, const char *table, const char *fields[], const char *types[], int nb, int numSpecialField, int removeIfExists);
    int (*createIndex)(struct PgDao *This, const char *table, const char *index, const char *fields[], int nb);
    int (*removeTable)(struct PgDao *This, const char *table, int dropIfExists);
    int (*createTriggersEntryCount)(struct PgDao *This, const char *tableName);
    int (*getNbResults)(struct PgDao *This);
    void (*clearResult)(struct PgDao *This);
    int (*beginTrans)(struct PgDao *This);
    int (*endTrans)(struct PgDao *This);
    int (*rollbackTrans)(struct PgDao *This);
    void (*freeDao)(struct PgDao *This);
    void (*logTrace)(const char *fctName, const char *msg);
    void (*logDebug)(const char *fctName, const char *msg);
    void (*logError)(const char *fctName, const char *msg);
    void (*logDebugFormat)(char *formatAndParams, ...);
    void (*logErrorFormat)(char *formatAndParams, ...);

    /*  member functions                              */

    /*  member datas                                  */
    /** Connexion Handle*/
    PGconn *conn;
    /** Last result's query */
    PGresult *result; 
    /** Current row of last query result */
    int resultCurrentRow;
    /** TRUE(=1)/FALSE(=0) if a cursor is currently opened */
    int cursorMode;
    /** Number of opened transactions (only one will be really opened) */
    int transactionInProgress;
} PgDao;

/* Constructors  */
PgDao *PgDao_new(void);

/* overrided functions */
int PgDao_openDB(PgDao *This, const char *conn);
int PgDao_isDBOpen(PgDao *This);
void PgDao_closeDB(PgDao *This);
void PgDao_closeDBAndExit(PgDao *This);
int PgDao_execQuery(PgDao *This, const char *query);
int PgDao_execQueryMultiResults(PgDao *This, const char *query);
int PgDao_execQueryParams(PgDao *This, const char *queryFormat, const char *paramValues[], int nbParams);
int PgDao_execQueryParamsMultiResults(PgDao *This, const char *query, const char *values[], int nbValues);
unsigned int PgDao_newEntry(PgDao *This, const char *table, const char *fields[], const char *values[], int nb, const char* returnField);
int PgDao_updateEntries(PgDao *This, const char *table, const char *fields[], const char *values[], int nb, const char *filter, const char *filterValues[]);
int PgDao_removeEntries(PgDao *This, const char *table, const char *filter, const char *filterValues[]);
int PgDao_getEntries(PgDao *This, const char *table, const char *fields[], int nbFields, const char *filter, const char *values[], const char *sortFields[], int nbSortFields, int limit, int offset, int cursorMode);
int PgDao_getEntriesCount(PgDao *This, const char *table, const char *filter, const char *values[]);
void PgDao_getNextEntry(PgDao *This);
int PgDao_hasNextEntry(PgDao *This);
char *PgDao_getFieldValue(PgDao *This, const char *fieldName);
int PgDao_getFieldValueAsInt(PgDao *This, const char *fieldName);
double PgDao_getFieldValueAsDouble(PgDao *This, const char *fieldName);
int PgDao_getResultNbFields(PgDao *This);
char *PgDao_getFieldValueByNum(PgDao *This, int numField);
int PgDao_getFieldValueAsIntByNum(PgDao *This, int numField);
double PgDao_getFieldValueAsDoubleByNum(PgDao *This, int numField);
int PgDao_createTable(PgDao *This, const char *table, const char *fields[], const char *types[], int nb, int numSpecialField, int removeIfExists);
int PgDao_createIndex(PgDao *This, const char *table, const char *index, const char *fields[], int nb);
int PgDao_removeTable(PgDao *This, const char *table, int dropIfExists);
int PgDao_createTriggersEntryCount(PgDao *This, const char *tableName);
int PgDao_getNbResults(PgDao *This);
void PgDao_clearResult(PgDao *This);
int PgDao_beginTrans(PgDao *This);
int PgDao_endTrans(PgDao *This);
int PgDao_rollbackTrans(PgDao *This);

#endif
