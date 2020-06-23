#ifndef DAO_H
#define DAO_H

typedef struct Dao {
    /*  functions member                              */
    int id;
    int (*openDB)(struct Dao *This, const char *conn);
    int (*isDBOpen)(struct Dao *This);
    void (*closeDB)(struct Dao *This);
    void (*closeDBAndExit)(struct Dao *This);
    int (*execQuery)(struct Dao *This, const char *query);
    int (*execQueryMultiResults)(struct Dao *This, const char *query);
    int (*execQueryParams)(struct Dao *This, const char *queryFormat, const char *paramValues[], int nbParams);
    int (*execQueryParamsMultiResults)(struct Dao *This, const char *query, const char *values[], int nbValues);
    unsigned int (*newEntry)(struct Dao *This, const char *table, const char *fields[], const char *values[], int nb, const char* returnField);
    int (*updateEntries)(struct Dao *This, const char *table, const char *fields[], const char *values[], int nb, const char *filter, const char *filterValues[]);
    int (*removeEntries)(struct Dao *This, const char *table, const char *filter, const char *filterValues[]);
    int (*getEntries)(struct Dao *This, const char *table, const char *fields[], int nbFields, const char *filter, const char *values[], const char *sortFields[], int nbSortFields, int limit, int offset, int cursorMode);
    int (*getEntriesCount)(struct Dao *This, const char *table, const char *filter, const char *values[]);
    void (*getNextEntry)(struct Dao *This);
    int (*hasNextEntry)(struct Dao *This);
    char *(*getFieldValue)(struct Dao *This, const char *fieldName);
    int (*getFieldValueAsInt)(struct Dao *This, const char *fieldName);
    double (*getFieldValueAsDouble)(struct Dao *This, const char *fieldName);
    int (*getResultNbFields)(struct Dao *This);
    char *(*getFieldValueByNum)(struct Dao *This, int numField);
    int (*getFieldValueAsIntByNum)(struct Dao *This, int numField);
    double (*getFieldValueAsDoubleByNum)(struct Dao *This, int numField);
    int (*createTable)(struct Dao *This, const char *table, const char *fields[], const char *types[], int nb, int numSpecialField, int removeIfExists);
    int (*createIndex)(struct Dao *This, const char *table, const char *index, const char *fields[], int nb);
    int (*removeTable)(struct Dao *This, const char *table, int dropIfExists);
    int (*createTriggersEntryCount)(struct Dao *This, const char *tableName);
    int (*getNbResults)(struct Dao *This);
    void (*clearResult)(struct Dao *This);
    int (*beginTrans)(struct Dao *This);
    int (*endTrans)(struct Dao *This);
    int (*rollbackTrans)(struct Dao *This);
    void (*freeDao)(struct Dao *This);
    void (*logTrace)(const char *fctName, const char *msg);
    void (*logDebug)(const char *fctName, const char *msg);
    void (*logError)(const char *fctName, const char *msg);
    void (*logDebugFormat)(char *formatAndParams, ...);
    void (*logErrorFormat)(char *formatAndParams, ...);

    /*  datas member                                  */
} Dao;

void Dao_init(Dao *);
void Dao_freeDao(Dao *This);
void Dao_logTrace(const char *fctName, const char *msg);
void Dao_logDebug(const char *fctName, const char *msg);
void Dao_logError(const char *fctName, const char *msg);
void Dao_logDebugFormat(char *formatAndParams, ...);
void Dao_logErrorFormat(char *formatAndParams, ...);

#endif
