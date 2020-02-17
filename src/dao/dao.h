#ifndef DAO_H
#define DAO_H


typedef struct Dao {
	/*  functions member                              */
	int (*openDB)(struct Dao*, const char*);
	void (*closeDB)(struct Dao*);
	void (*closeDBAndExit)(struct Dao*);
	void (*logError)(const char*, const char*);
	void (*logDebug)(const char*, const char*);
/*  datas member                                  */
} Dao;

///* Constructors  */
//Dao Dao_Create(void);
//Dao* Dao_New(void);
//
//int Dao_openDB(Dao*, const char*);
//void Dao_closeDB(Dao*);
//void Dao_closeDBAndExit(Dao*);

void Dao_logError(const char *fctName, const char *msg);
void Dao_logDebug(const char *fctName, const char *msg);


#endif
