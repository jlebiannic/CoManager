#include "dao/pg/pgDao.h"
#include "commons/commons.h"
#include "dao/dao.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ID = 1;
static int ROWS_PER_FETCH = 100; // TODO application parameter
static const char DECLARE_CURSOR_CMD[] = "DECLARE pgDaoCursor CURSOR FOR";
static const char FETCH_CMD_FORMAT[] = "FETCH %d IN pgDaoCursor";

static void PgDao_init(PgDao *);
static void PgDao_setDefaultSchema(PgDao *This);
static int p_execQueryParams(PgDao *This, const char *command, const char *const *paramValues, int nParams);
static void p_addResult(PgDao *This, PGresult *res);
static int p_fetch(PgDao *This);
static int p_doCommand(PgDao *This, char *command);
static void p_closeCursor(PgDao *This);
static char* p_toPgStx(const char *str, int *nbValuesFound, int startAt);
static char *p_makeSelectQuery(const char *table, const char *fields[], int nbFields, const char *filter, int *nbValues, const char *sortFields[], int nbSortFields, int limit, int offset);
static char *p_buildValuesTpl(int nb);
static void p_clearResultNotCloseCursor(PgDao *This);
static void p_addResultNotCloseCursor(PgDao *This, PGresult *res);
static int p_cursorMode(int cursorMode);


/**
 * @brief Function pointers initialisation
 * 
 * @param This 
 */
static void PgDao_init(PgDao *This) {
    This->openDB = PgDao_openDB;
    This->isDBOpen = PgDao_isDBOpen;
    This->closeDB = PgDao_closeDB;
    This->closeDBAndExit = PgDao_closeDBAndExit;
    This->execQuery = PgDao_execQuery;
    This->execQueryMultiResults = PgDao_execQueryMultiResults;
    This->execQueryParams = PgDao_execQueryParams;
    This->execQueryParamsMultiResults = PgDao_execQueryParamsMultiResults;
    This->newEntry = PgDao_newEntry;
    This->updateEntries = PgDao_updateEntries;
    This->removeEntries = PgDao_removeEntries;
    This->getEntries = PgDao_getEntries;
    This->getEntriesCount = PgDao_getEntriesCount;
    This->getNextEntry = PgDao_getNextEntry;
    This->hasNextEntry = PgDao_hasNextEntry;
    This->getFieldValue = PgDao_getFieldValue;
    This->getFieldValueAsInt = PgDao_getFieldValueAsInt;
    This->getFieldValueAsDouble = PgDao_getFieldValueAsDouble;
    This->getResultNbFields = PgDao_getResultNbFields;
    This->getFieldValueByNum = PgDao_getFieldValueByNum;
    This->getFieldValueAsIntByNum = PgDao_getFieldValueAsIntByNum;
    This->getFieldValueAsDoubleByNum = PgDao_getFieldValueAsDoubleByNum;
    This->createTable = PgDao_createTable;
    This->createIndex = PgDao_createIndex;
    This->removeTable = PgDao_removeTable;
    This->createTriggersEntryCount = PgDao_createTriggersEntryCount;
    This->getNbResults = PgDao_getNbResults;
    This->clearResult = PgDao_clearResult;
    This->beginTrans = PgDao_beginTrans;
    This->endTrans = PgDao_endTrans;
    This->rollbackTrans = PgDao_rollbackTrans;

    // parent functions
    Dao_init((Dao *)This);
    }

    /** 
     * @brief Constructor: Allocate memory for Dao
     * Initialise private attributes
     * 
     * @return *PgDao
    */
    PgDao *PgDao_new() {
        PgDao *This = malloc(sizeof(PgDao));
        if (!This) {
            This->logError("PgDao_New", "malloc failed");
            return NULL;
        }
        PgDao_init(This);

        This->id = ID;
        This->conn = NULL;
        This->result = NULL;
        This->resultCurrentRow = -1;
        This->cursorMode = FALSE;
        This->transactionInProgress = 0;
        return This;
    }

    /**
     * @brief Open database if not already open
     * 
     * @param This 
     * @param conninfo connexion informations if needed (default info are provide by PG_CONNINFO environnement variable)
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_openDB(PgDao * This, const char *conninfo) {
        This->logTrace("PgDao_openDB", "start");

        // Already open
        if (This->isDBOpen(This)) {
            return TRUE;
        }

//        if (!conninfo) {
//            conninfo = getenv("PG_CONNINFO");
//            if (!conninfo) {
//                This->logError("Connection to database failed", "Please check environment variable PG_CONNINFO");
//                return FALSE;
//            }
//        }
        /* Cree une connexion a la base de donnees */
        This->conn = PQconnectdb("host=localhost port=5432 dbname=postgres user=postgres password=admin");

        /* Verifier que la connexion au backend a ete faite avec succes */
        if (PQstatus(This->conn) != CONNECTION_OK) {
            This->logError("Connection to database failed", PQerrorMessage(This->conn));
            This->closeDB(This);
            return FALSE;
        }
        PgDao_setDefaultSchema(This);
        return TRUE;
    }

    /**
     * @brief Close database and clear current result
     * 
     * @param This 
     */
    void PgDao_closeDB(PgDao * This) {
        This->logTrace("PgDao_closeDB", "start");
        This->clearResult(This);
        if (This->conn != NULL) {
            PQfinish(This->conn);
            This->conn = NULL;
        }
    }

    /**
     * @brief Close database and clear current result and exit program
     * 
     * @param This 
     */
    void PgDao_closeDBAndExit(PgDao * This) {
        This->logTrace("PgDao_closeDBAndExit", "start");
        This->closeDB(This);
        exit(1);
    }

    /**
     * @brief Execute a query (warning: it is not SQL injection safe )
     * 
     * @param This 
     * @param query the query
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_execQuery(PgDao * This, const char *query) {
        This->logTrace("PgDao_execQuery", "start");
        int res = FALSE;
        if (This->openDB(This, NULL)) {
            This->logDebugFormat("[execQuery]query = %s\n", query);
            p_addResult(This, PQexec(This->conn, query));
            if (PQresultStatus(This->result) != PGRES_TUPLES_OK && PQresultStatus(This->result) != PGRES_COMMAND_OK) {
                This->logErrorFormat("PgDao_execQuery: %s failed, message=%s (status code=%d)", query, PQerrorMessage(This->conn), (int)PQresultStatus(This->result));
                This->clearResult(This);
            } else {
                res = TRUE;
            }
        }
        return res;
    }

    /**
     * @brief Execute a query using a cursor (warning: it is not SQL injection safe)
     * 
     * @param This 
     * @param query the query
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_execQueryMultiResults(PgDao * This, const char *query) {
        This->logTrace("PgDao_execQueryMultiResults", "start");
        int res = FALSE;

        if (This->openDB(This, NULL)) {
            if (This->beginTrans(This)) {
                char *cursorQuery = allocStr("%s %s", DECLARE_CURSOR_CMD, query);
                p_addResult(This, PQexec(This->conn, cursorQuery));
                free(cursorQuery);
                if (PQresultStatus(This->result) != PGRES_COMMAND_OK) {
                    This->logError("PgDao_execQueryMultiResults declare cursor failed", PQerrorMessage(This->conn));
                    This->clearResult(This);
                } else {
                    This->cursorMode = TRUE;
                    if (p_fetch(This) >= 0) {
                        res = TRUE;
                    }
                }
            }
        }
        return res;
    }

    /**
     * @brief Execute a query with parameters outside the query using PostgreSQL syntax for parameters (ie $1, $2, ...)
     * (SQL injection safe)
     * 
     * @param This 
     * @param query the query
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_execQueryParams(PgDao * This, const char *queryFormat, const char *paramValues[], int nbParams) {
        This->logTrace("PgDao_execQueryParams", "start");
        int res = FALSE;
        if (This->openDB(This, NULL)) {
            return p_execQueryParams(This, queryFormat, paramValues, nbParams);
        }
        return res;
    }

    /**
     * @brief Execute a query with internal cursor or not (depending of the cursorMode parameter)
     * (SQL injection safe )
     * 
     * @param This 
     * @param table table name
     * @param fields array of field to be selected in result
     * @param nbFields 
     * @param filter filer clause (example: c1=$ or c2=$)
     * @param values array of value for filter
     * @param sortFields array of sorted fields
     * @param nbSortFields 
     * @param limit results limit
     * @param offset results offset (begin at 0 for no offset) 
     * @param cursorMode (TRUE=1/FALSE=0 open a cursor or not)
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_getEntries(PgDao * This, const char *table, const char *fields[], int nbFields, const char *filter, const char *values[], const char *sortFields[], int nbSortFields, int limit, int offset, int cursorMode) {
        This->logTrace("PgDao_getEntries", "start");

        int nbValues = 0;
        char *query = p_makeSelectQuery(table, fields, nbFields, filter, &nbValues, sortFields, nbSortFields, limit, offset);
        This->logDebugFormat("[getEntries]select query = %s\n", query);

        int res;
        if (p_cursorMode(cursorMode)) {
            res = This->execQueryParamsMultiResults(This, query, values, nbValues);
        } else {
            res = This->execQueryParams(This, query, values, nbValues);
        }
        free(query);
        return res;
    }

    /**
     * @brief Return the number of entries in a table
     * 
     * @param This 
     * @param table table name
     * @param filter filer clause (example: c1=$ or c2=$)
     * @param values array of value for filter
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_getEntriesCount(PgDao * This, const char *table, const char *filter, const char *values[]) {
        This->logTrace("PgDao_getEntriesCount", "start");
        int res = -1;
        if (filter == NULL) {
            char *query = allocStr("select cnt from entry_count where table_name='%s'", table);
            if (This->execQuery(This, query)) {
                res = This->getFieldValueAsIntByNum(This, 0);
            }
            free(query);
        } else {
            const char *fields[1] = {"count(*)"};
            if (This->getEntries(This, table, fields, 1, filter, values, NULL, 0, -1, -1, FALSE)) {
                res = This->getFieldValueAsIntByNum(This, 0);
            }
        }
        return res;
    }

    /**
     * @brief Execute a query using a cursor with external parameters
     * (SQL injection safe)
     * 
     * @param This 
     * @param query the query
     * @param values values for the query
     * @param nbValues 
     * @return int 
     */
    int PgDao_execQueryParamsMultiResults(PgDao * This, const char *query, const char *values[], int nbValues) {
        This->logTrace("PgDao_execQueryParamsMultiResults", "start");
        int res = FALSE;
        if (This->openDB(This, NULL)) {
            if (This->beginTrans(This)) {
                char *cursorQuery = allocStr("%s %s", DECLARE_CURSOR_CMD, query);
                p_execQueryParams(This, cursorQuery, values, nbValues);
                free(cursorQuery);
                if (PQresultStatus(This->result) != PGRES_COMMAND_OK) {
                    This->logError("PgDao_execQueryParamsMultiResults declare cursor failed", PQerrorMessage(This->conn));
                    This->clearResult(This);
                } else {
                    This->cursorMode = TRUE;
                    if (p_fetch(This) >= 0) {
                        res = TRUE;
                    }
                }
            }
        }
        return res;
    }

    /**
     * @brief Return TRUE(=1) or FALSE(=0) depending if the current result has an other entry (row) after the execution of query
     * 
     * @param This 
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_hasNextEntry(PgDao * This) {
        This->logTrace("PgDao_hasNextEntry", "start");
        if ((This->resultCurrentRow != -1 && This->resultCurrentRow <= PQntuples(This->result) - 1) || p_fetch(This) > 0) {
            return TRUE;
        } else {
            p_closeCursor(This);
            return FALSE;
        }
    }

    /**
     * @brief Increment the current entry count if an other entry (row) remain since the execution of th query
     * 
     * @param This 
     */
    void PgDao_getNextEntry(PgDao * This) {
        This->logTrace("PgDao_getNextEntry", "start");
        if (This->hasNextEntry(This)) {
            This->resultCurrentRow++;
        }
    }

    /**
     * @brief Return the value of a field as Int for the current entry of the result
     * 
     * @param This 
     * @param fieldName name of the field to return the value for
     * @return int 
     */
    int PgDao_getFieldValueAsInt(PgDao * This, const char *fieldName) {
        This->logTrace("PgDao_getFieldValueAsInt", "start");
        return strtol(This->getFieldValue(This, fieldName), (char **)NULL, 10);
    }

    /**
     * @brief Return the value of a field as Double for the current entry of the result
     * 
     * @param This 
     * @param fieldName name of the field to return the value for
     * @return double 
     */
    double PgDao_getFieldValueAsDouble(PgDao * This, const char *fieldName) {
        This->logTrace("PgDao_getFieldValueAsInt", "start");
        return strtod(This->getFieldValue(This, fieldName), (char **)NULL);
    }

    /**
     * @brief Return the value of a field as Char* for the current entry of the result
     * 
     * @param This 
     * @param fieldName name of the field to return the value for
     * @return char 
     */
    char *PgDao_getFieldValue(PgDao * This, const char *fieldName) {
        This->logTrace("PgDao_getFieldValue", "start");
        int nFields;
        int i;
        int numField = -1;
        char *name;
        char *fielValue = NULL;
        if (This->result != NULL && PQresultStatus(This->result) == PGRES_TUPLES_OK) {
            nFields = PQnfields(This->result);
            for (i = 0; i < nFields; i++) {
                name = PQfname(This->result, i);
                if (strcasecmp(name, fieldName) == 0) {
                    numField = i;
                    break;
                }
            }
            // numField = PQfnumber(This->result, fieldName);
            if (numField >= 0) {
                fielValue = PQgetvalue(This->result, This->resultCurrentRow, numField);
            } else {
                This->logError("PgDao_getFieldValue", "invalid numField");
            }
        } else {
            This->logError("PgDao_getFieldValue", "result is null");
        }
        return fielValue;
    }

    /**
     * @brief Return the number of the fields returned by a query
     * 
     * @param This 
     * @return int the number of the fields 
     */
    int PgDao_getResultNbFields(PgDao * This) {
        This->logTrace("PgDao_getResultNbFields", "start");
        if (This->result == NULL) {
            This->logError("PgDao_getResultNbFields", "result is null");
            return -1;
        }
        return PQnfields(This->result);
    }

    /**
     * @brief Return the value of a field as Char* identified by his number in the current result
     * 
     * @param This 
     * @param fieldName name of the field to return the value for
     * @return Char* 
     */
    char *PgDao_getFieldValueByNum(PgDao * This, int numField) {
        This->logTrace("PgDao_getFieldValueByNum", "start");
        char *fielValue = NULL;
        if (numField >= 0) {
            fielValue = PQgetvalue(This->result, This->resultCurrentRow, numField);
        } else {
            This->logError("PgDao_getFieldValue", "invalid numField");
        }
        return fielValue;
    }

    /**
     * @brief Return the value of a field as Int identified by his number in the current result
     * 
     * @param This 
     * @param numField the number of the field to return the value for
     * @return int 
     */
    int PgDao_getFieldValueAsIntByNum(PgDao * This, int numField) {
        This->logTrace("PgDao_getFieldValueAsIntByNum", "start");
        return strtol(This->getFieldValueByNum(This, numField), (char **)NULL, 10);
    }

    /**
     * @brief Return the value of a field as Double identified by his number in the current result
     * 
     * @param This 
     * @param numField the number of the field to return the value for
     * @return double 
     */
    double PgDao_getFieldValueAsDoubleByNum(PgDao * This, int numField) {
        This->logTrace("PgDao_getFieldValueAsDoubleByNum", "start");
        //return strtod(This->getFieldValueByNum(This, numField), (char **)NULL);
        char *str = This->getFieldValueByNum(This, numField);
        double res = strtod(str, (char **)NULL);
        return res;
    }

    /**
     * @brief Insert a new entry in a table
     * 
     * @param This 
     * @param table table name
     * @param fields array of fields to be inserted
     * @param values array of values for the fields
     * @param nb number of fields/values
     * @param returnField name of the field wich value is returned (must be an unsigned int)
     * @return unsigned int 
     */
    unsigned int PgDao_newEntry(PgDao * This, const char *table, const char *fields[], const char *values[], int nb, const char *returnField) {
        This->logTrace("PgDao_addEntry", "start");

        const char insertInto[] = "insert into";
        char *query;
        unsigned int idx = 0;

        if (This->openDB(This, NULL)) {
            char *fieldsWithComma = arrayJoin(fields, nb, ",");
            char *valuesTpl = p_buildValuesTpl(nb);
            query = allocStr("%s %s (%s) values (%s) returning %s", insertInto, table, fieldsWithComma, valuesTpl, returnField);
            This->logDebugFormat("[newEntry]query: %s", query);
            p_execQueryParams(This, query, values, nb);
            free(fieldsWithComma);
            free(valuesTpl);
            free(query);

            if (PQresultStatus(This->result) != PGRES_TUPLES_OK) {
                This->logError("PgDao_addEntry failed", PQerrorMessage(This->conn));
            } else {
                idx = strtol(This->getFieldValue(This, returnField), (char **)NULL, 10);
            }

            This->clearResult(This);
        }
        return idx;
    }

    /**
     * @brief Execute an update query
     * 
     * @param This 
     * @param table table name
     * @param fields array of fields name to be updated
     * @param values array of values for fields
     * @param nb number of fields/values 
     * @param filter filter clause (example: c1=$ or c2=$)
     * @param filterValues array values for filter
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_updateEntries(PgDao * This, const char *table, const char *fields[], const char *values[], int nb, const char *filter, const char *filterValues[]) {
        int res;

        char updateElemTpl[] = "%s=$%d";
        char updateElemWithCommaTpl[] = "%s=$%d,";
        char *updateSet = allocStr("UPDATE %s SET", table);
        char *updateElems = NULL;
        int nbAllValues = -1;
        char *updateFilter = filter != NULL && strlen(filter) > 0 ? allocStr("WHERE %s", p_toPgStx(filter, &nbAllValues, nb + 1)) : "";
        int i = 0;
        for (i = 0; i < nb; i++) {
            char *tpl = NULL;
            if (i == nb - 1) {
                tpl = &updateElemTpl[0];
            } else {
                tpl = &updateElemWithCommaTpl[0];
            }

            if (updateElems == NULL) {
                updateElems = allocStr(tpl, fields[i], i + 1);
            } else {
                updateElems = reallocStr(updateElems, tpl, fields[i], i + 1);
            }
        }

        char *query = allocStr("%s %s %s", updateSet, updateElems, updateFilter);
        This->logDebugFormat("update query = %s\n", query);
        char **allValues = arrayConcat((char **)values, nb, (char **)filterValues, nbAllValues - nb);
        res = This->execQueryParams(This, query, (const char **)allValues, nbAllValues);
        free(allValues);
        free(updateSet);
        free(updateElems);
        free(updateFilter);
        free(query);

        return res;
    }

    /**
     * @brief Execute a delete query
     * 
     * @param This 
     * @param table table name
     * @param filter filter clause (example: c1=$ or c2=$)
     * @param filterValues array values for filter
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_removeEntries(PgDao * This, const char *table, const char *filter, const char *filterValues[]) {
        int res;
        char *deleteFrom = allocStr("DELETE FROM %s", table);
        int nbValues = -1;
        char *removeFilter = filter != NULL && strlen(filter) > 0 ? allocStr("WHERE %s", p_toPgStx(filter, &nbValues, 1)) : "";

        char *query = allocStr("%s %s", deleteFrom, removeFilter);
        This->logDebugFormat("remove entries query = %s\n", query);
        res = This->execQueryParams(This, query, (const char **)filterValues, nbValues);
        free(deleteFrom);
        free(removeFilter);
        free(query);

        return res;
    }

    /**
     * @brief Execute a "drop table" query
     * 
     * @param This 
     * @param table table name
     * @param dropIfExists TRUE(=1)/FALSE(=0) If TRUE: no error if table don't exists
     * @return int 
     */
    int PgDao_removeTable(PgDao * This, const char *table, int dropIfExists) {
        int res;
        char removeTableTpl[] = "DROP TABLE %s %s";
        char ifExitsTpl[] = "IF EXISTS";

        char *query = allocStr(removeTableTpl, dropIfExists ? ifExitsTpl : "", table);
        This->logDebugFormat("remove table query = %s\n", query);
        res = This->execQuery(This, query);

        free(query);

        return res;
    }

    /**
     * @brief Execute a "create table" query
     * 
     * @param This 
     * @param table table name
     * @param fields table's fields
     * @param types array of fields' types
     * @param nb number of fields
     * @param numSpecialField special field (used to mark auto increment field)
     * @param removeIfExists TRUE(=1)/FALSE(=0) If TRUE: no error if table exists and remove it before
     * @return int 
     */
    int PgDao_createTable(PgDao * This, const char *table, const char *fields[], const char *types[], int nb, int numSpecialField, int removeIfExists) {
        int res;
        char createTableTpl[] = "CREATE TABLE %s (%s)";
        char typedFieldTpl[] = "%s %s";
        char typedFieldTplWithComma[] = "%s %s,";
        char *typedFields = NULL;

        if (removeIfExists) {
            This->removeTable(This, table, TRUE);
        }

        int i = 0;
        for (i = 0; i < nb; i++) {
            char *tpl = NULL;

            if (i == nb - 1) {
                tpl = &typedFieldTpl[0];
            } else {
                tpl = &typedFieldTplWithComma[0];
            }

            char *type = NULL;
            if (numSpecialField == i) {
                type = allocStr("%s", "SERIAL PRIMARY KEY");
            } else {
                type = allocStr("%s", types[i]);
            }

            if (typedFields == NULL) {
                typedFields = allocStr(tpl, fields[i], type);
            } else {
                typedFields = reallocStr(typedFields, tpl, fields[i], type);
            }
            free(type);
        }

        char *query = allocStr(createTableTpl, table, typedFields);
        This->logDebugFormat("create table query = %s\n", query);
        res = This->execQuery(This, query);

        free(typedFields);
        free(query);

        return res;
    }

    /**
     * @brief Execute a query to create index on fields
     * 
     * @param This 
     * @param table table name
     * @param index index name
     * @param fields index fields
     * @param nb number of fields
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_createIndex(PgDao * This, const char *table, const char *index, const char *fields[], int nb) {
        int res;
        char createIndexTpl[] = "CREATE INDEX %s on %s (%s)";
        char fieldTpl[] = "%s";
        char fieldTplWithComma[] = "%s,";
        char *indexFields = NULL;

        int i = 0;
        for (i = 0; i < nb; i++) {
            char *tpl = NULL;

            if (i == nb - 1) {
                tpl = &fieldTpl[0];
            } else {
                tpl = &fieldTplWithComma[0];
            }

            if (indexFields == NULL) {
                indexFields = allocStr(tpl, fields[i]);
            } else {
                indexFields = reallocStr(indexFields, tpl, fields[i]);
            }
        }

        char *query = allocStr(createIndexTpl, index, table, indexFields);
        This->logDebugFormat("create index query = %s\n", query);
        res = This->execQuery(This, query);

        free(indexFields);
        free(query);

        return res;
    }

    /**
     * @brief create ENTRY_COUNT table if not already exists and triggers for the table 
     * ENTRY_COUNT table contains the number of entries for a table
     * Trigger are executed when adding or remvoving entries in the table to update ENTRY_COUNT table
     * 
     * @param This 
     * @param tableName table name
     * @return int (TRUE=1/FALSE=0)
     */
    int PgDao_createTriggersEntryCount(PgDao * This, const char *tableName) {
        int res;
        char createEntryCountTableTpl[] = "CREATE TABLE IF NOT EXISTS entry_count (table_name text, cnt bigint); %s; %s; %s; %s; %s; %s;";
        char createIndex[] = "CREATE UNIQUE INDEX IF NOT EXISTS table_name_idx on entry_count (table_name)";
        char *insertInto = allocStr("INSERT INTO entry_count (table_name, cnt) VALUES ('%s', 0) ON CONFLICT (table_name) DO UPDATE SET cnt=(select count(*) from %s)", tableName, tableName);
        char fctIncCount[] = "CREATE OR REPLACE FUNCTION inc_count_fct() RETURNS trigger AS $$ BEGIN UPDATE entry_count set cnt=cnt+1 where table_name=TG_ARGV[0]; RETURN NULL; END; $$ LANGUAGE plpgsql";
        char fctDecCount[] = "CREATE OR REPLACE FUNCTION dec_count_fct() RETURNS trigger AS $$ BEGIN UPDATE entry_count set cnt=cnt-1 where table_name=TG_ARGV[0]; RETURN NULL; END; $$ LANGUAGE plpgsql";
        char *triggerIncCount = allocStr("DROP TRIGGER IF EXISTS inc_count on %s; CREATE TRIGGER inc_count AFTER INSERT ON %s FOR EACH ROW EXECUTE PROCEDURE inc_count_fct(%s)", tableName, tableName, tableName);
        char *triggerDecCount = allocStr("DROP TRIGGER IF EXISTS dec_count on %s; CREATE TRIGGER dec_count AFTER DELETE ON %s FOR EACH ROW EXECUTE PROCEDURE dec_count_fct(%s)", tableName, tableName, tableName);

        char *query = allocStr(createEntryCountTableTpl, createIndex, insertInto, fctIncCount, fctDecCount, triggerIncCount, triggerDecCount);
        This->logDebugFormat("createTriggersEntryCount query = %s\n", query);
		res = This->execQuery(This, query);

        free(insertInto);
        free(triggerIncCount);
        free(triggerDecCount);
        free(query);

    return res;
}

/**
 * @brief Return the number of results for the last query
 * 
 * @param This 
 * @return int the number of results
 */
int PgDao_getNbResults(PgDao *This) {
	return PQntuples(This->result);
}

/**
 * @brief Begin a transaction (only one transaction, no sub transaction )
 * If a transaction is in progress a counter is incremented but no sub transaction is maintened (user must end as many transactions as he has opened)
 * @param This 
 * @return int (TRUE=1/FALSE=0)
 */
int PgDao_beginTrans(PgDao *This) {
    This->logTrace("PgDao_beginTrans", "start");
    int res;
    if (This->transactionInProgress == 0) {
        res = p_doCommand(This, "BEGIN");
    } else {
        This->logDebug("PgDao_beginTrans", "transaction already in progress\n");
        res = TRUE;
    }

    if(res){
        This->transactionInProgress = This->transactionInProgress + 1;
    }

    return res;
}

/**
 * @brief End a transaction (only one transaction, no sub transaction )
 * If many transactions has been opened a counter is decremented (user must end as many transactions as he has opened)
 * @param This 
 * @return int (TRUE=1/FALSE=0)
 */
int PgDao_endTrans(PgDao *This) {
    This->logTrace("PgDao_endTransaction", "start");
    /* termine la transaction */
    int res;
    if (This->transactionInProgress == 1) {
        res = p_doCommand(This, "END");
        if(res){
            This->transactionInProgress = 0;
        }
    } else {
        if (This->transactionInProgress == 0) {
            This->logDebug("PgDao_endTrans", "no transaction in progress\n");
        } else {
            This->transactionInProgress = This->transactionInProgress - 1;
        }
        res = TRUE;
    }
    return res;
}

/**
 * @brief Rollback a transaction
 * 
 * @param This 
 * @return int (TRUE=1/FALSE=0)
 */
int PgDao_rollbackTrans(PgDao *This) {
    This->logTrace("PgDao_endTransaction", "start");
    /* termine la transaction */
    int res;
    if (This->transactionInProgress > 0) {
        res = p_doCommand(This, "ROLLBACK");
        if(res){
            This->transactionInProgress = 0;
        }
    } else {
        This->logDebug("PgDao_rollbackTrans", "no transaction in progress\n");
        res = TRUE;
    }
    return res;
}

/**
 * @brief Return TRUE(=1)/FALSE(=0) whether or not database is opened
 *  
 * @param This 
 * @return int 
 */
int PgDao_isDBOpen(PgDao *This) {
    This->logTrace("PgDao_isDBOpen", "start");
    return This->conn != NULL ? TRUE : FALSE;
}

/**
 * @brief Clear the last result of a query
 * 
 * @param This 
 */
void PgDao_clearResult(PgDao *This) {
    This->logTrace("PgDao_clearResult", "start");
    p_clearResultNotCloseCursor(This);
    p_closeCursor(This);
}

//////////////////////////////////////////////////////////////////////////////////
// Private functions (static)
//////////////////////////////////////////////////////////////////////////////////

static void PgDao_setDefaultSchema(PgDao *This) {
	This->logDebug("PgDao_setDefaultSchema", "start");
	This->execQuery(This, "SELECT pg_catalog.set_config('search_path', '\"Tdx\"', false)");
	if (PQresultStatus(This->result) != PGRES_TUPLES_OK) {
		This->logError("PgDao_setDefaultSchema", PQerrorMessage(This->conn));
		PgDao_clearResult(This);
		This->closeDBAndExit(This);
	}
	PgDao_clearResult(This);
}

/**
 * @brief Clear the last result of a query but not close the cursor if it is opened
 * 
 * @param This 
 */
static void p_clearResultNotCloseCursor(PgDao *This) {
    This->logTrace("p_clearResult", "start");
    if (This->result != NULL) {
        PQclear(This->result);
        This->result = NULL;
    }
    This->resultCurrentRow = -1;
}

/**
 * @brief Execute a query with parameters and had the result to the Dao (This)
 * 
 * 
 * @param This 
 * @param command query
 * @param paramValues arryay of parameters values
 * @param nParams number of parameters
 * @return int (TRUE=1/FALSE=0)
 */
static int p_execQueryParams(PgDao *This, const char *command, const char *const *paramValues, int nParams) {
    int res = TRUE;
    This->logTrace("p_execQueryParams", "start");
    p_addResult(This, PQexecParams(This->conn, command, nParams, NULL, paramValues, NULL, NULL, 0));

    if (PQresultStatus(This->result) != PGRES_TUPLES_OK && PQresultStatus(This->result) != PGRES_COMMAND_OK) {
        This->logError("p_execQueryParams failed", PQerrorMessage(This->conn));
        This->clearResult(This);
        res = FALSE;
    }

    return res;
}

/**
 * @brief Add a result (PGresult) to the Dao (This)
 * Clear last result before and close cursor if opened
 * 
 * @param This 
 * @param res result (PGresult) to be added
 */
static void p_addResult(PgDao *This, PGresult *res) {
    This->logTrace("p_addResult", "start");
    This->clearResult(This);
    This->result = res;
    This->resultCurrentRow = 0;
}
/**
 * @brief Add a result (PGresult) to the Dao (This)
 * Clear last result before and not close cursor if opened
 * 
 * @param This 
 * @param res  result (PGresult) to be added
 */
static void p_addResultNotCloseCursor(PgDao *This, PGresult *res) {
    This->logTrace("p_addResult", "start");
    p_clearResultNotCloseCursor(This);
    This->result = res;
    This->resultCurrentRow = 0;
}

/**
 * @brief  Fetch a cursor and return the number of entries of result
 * 
 * @param This 
 * @return int the number of entries of result
 */
static int p_fetch(PgDao *This) {
    This->logTrace("p_fetch", "start");
    int res = -1;
    if (This->cursorMode) {
        char *fetchQuery = allocStr(FETCH_CMD_FORMAT, ROWS_PER_FETCH);
        p_addResultNotCloseCursor(This, PQexec(This->conn, fetchQuery));
        free(fetchQuery);
        if (PQresultStatus(This->result) != PGRES_TUPLES_OK) {
            This->logError("p_fetch fetch failed", PQerrorMessage(This->conn));
            This->clearResult(This);
        } else {
            res = PQntuples(This->result);
        }
    }
    return res;
}

/**
 * @brief Execute a query without adding result ot the Dao (This)
 * (Not SQL injection safe)
 * 
 * @param This 
 * @param command query
 * @return int 
 */
static int p_doCommand(PgDao *This, char *command) {
    This->logTrace("p_doCommand", "start");
    PGresult *res;
    if (This->openDB(This, NULL)) {
        res = PQexec(This->conn, command);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            This->logErrorFormat("doCommand: %s failed, message=%s (status code=%d)", command, PQerrorMessage(This->conn), (int)PQresultStatus(res));
            PQclear(res);
            return FALSE;
        }
        PQclear(res);
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Close the cursor and clear result
 * 
 * @param This 
 */
static void p_closeCursor(PgDao *This) {
    if (This->cursorMode) {
        This->logTrace("p_closeCursor", "start");
        p_doCommand(This, "CLOSE pgDaoCursor");
        This->endTrans(This);
        This->cursorMode = FALSE;
        p_clearResultNotCloseCursor(This);
    }
}

/**
 * @brief Transform str to PostgreSQL synthax.
 * Ex: select * from t where c1=$ and c2>$
 *     become
 *     select * from t where c1=$1 and c2>$2
 * 
 * @param str query
 * @param nbValuesFound number of values (ie '$' characters) found
 * @param startAt begin counter (ie if 2 the first parameters will be $2)
 * @return char* query with PostgreSQL synthax
 */
static char* p_toPgStx(const char *str, int *nbValuesFound, int startAt) {
    char *strRes = NULL;
    int len = strlen(str) * 2;
    int currentResLen = len;
    strRes = (char *)malloc(len + 1);
    strcpy(strRes, "");
	int cpt = startAt;
    char *varNum = NULL;
    while (*str != '\0') {
        strncat(strRes, str, 1);
        if (*str == '$') {
            // verify not $1 (for example) but $ only
            if (!isdigit((int)*(str + 1))) {
                varNum = inttoa(cpt);
                if (strlen(strRes) + strlen(varNum) > currentResLen) {
                    currentResLen = currentResLen + len;
                    strRes = (char *)realloc(strRes, currentResLen + 1);
                }
                strcat(strRes, varNum);
                free(varNum);
            }
            cpt++;
        }
        str++;
    }
    *nbValuesFound = cpt - 1;
    return strRes;
}

/**
 * @brief Build a "Select" query with parameters
 * 
 * @param table table name
 * @param fields fields to be retrieve
 * @param nbFields number of fields
 * @param filter filter clause (example: c1=$1 or c1=$2)
 * @param nbValues number of values in filter
 * @param sortFields sorted filelds
 * @param nbSortFields number of sorted fields
 * @param limit results limit 
 * @param offset results offset (begin at 0 for no offset) 
 * @return char* query
 */
static char* p_makeSelectQuery(const char *table, const char *fields[], int nbFields, const char *filter, int *nbValues, const char *sortFields[], int nbSortFields, int limit, int offset) {
	char *selectFields = arrayJoin(fields, nbFields, ",");
    int plusSize = nbSortFields * 100 + 100;
    char *query =
        filter != NULL && strlen(filter) > 0 ? allocStrPlusSize(plusSize, "Select %s from %s where %s", selectFields, table, p_toPgStx(filter, nbValues, 1))
                                             : allocStrPlusSize(plusSize, "Select %s from %s", selectFields, table);
    if(nbSortFields != 0){
        sprintf(query, "%s ORDER BY %s", query, arrayJoin(sortFields, nbSortFields, ","));
    }

    if(limit > 0){
        sprintf(query, "%s LIMIT %d", query, limit);
    }

    if(offset > 0){
        sprintf(query, "%s OFFSET %d", query, offset);
    }

    free(selectFields);
	return query;
}

/**
 * @brief Build VALUES template from nb
 * Example: nb=3 => result will be $1,$2,$3
 * 
 * @param nb number of parametrs in result
 * @return char* string with parameters like $1,$2,$3,...
 */
static char* p_buildValuesTpl(int nb){
    char *tpl = (char *)malloc(nb*10); //$99'999'999,
    strcpy(tpl, "");
    int i = 0;
    for (i = 0; i < nb; i++){
        if(i != 0){
            sprintf(tpl, "%s,$%d", tpl, i + 1);
        } else {
            strcpy(tpl, "$1");
        }
    }
    return tpl;
}

/**
 * @brief Decides to be in cursor mode or not depending on the DATA_ACCESS_NOCURSOR environment variable
 * 
 * @param cursorMode 
 * @return int 
 */
static int p_cursorMode(int cursorMode){
    if (getenv("DATA_ACCESS_NOCURSOR") == NULL) {
        return cursorMode;
    } else {
        return FALSE;
    }
}
