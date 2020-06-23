#include "dao/dao.h"

#include<stdlib.h>
#include<stdio.h>
#include <stdarg.h>

/**
 * @brief Function pointers initialisation
 * 
 * @param This 
 */
void Dao_init(Dao *This) {
	This->logTrace = Dao_logTrace;
	This->logDebug = Dao_logDebug;
	This->logError = Dao_logError;
	This->logErrorFormat = Dao_logErrorFormat;
	This->logDebugFormat = Dao_logDebugFormat;
    This->freeDao = Dao_freeDao;
}

/**
 * @brief Close database and free memory of Dao 
 * 
 * @param This 
 */
void Dao_freeDao(Dao* This){
	This->logTrace("Dao_freeDao", "start");
	if(This){
        This->closeDB(This);
        free(This);
    }
}

/**
 * @brief Trace
 * 
 * @param fctName function name of the trace
 * @param msg message
 */
void Dao_logTrace(const char *fctName, const char *msg) {
#ifdef DEBUG
	printf("%s: %s\n", fctName, msg);
	fflush(stdout);
#endif
}

/**
 * @brief Debug
 * 
 * @param fctName function name of the debug
 * @param msg message
 */
void Dao_logDebug(const char *fctName, const char *msg) {
	if (getenv("DATA_ACCESS_DEBUG") != NULL) {
		fprintf(stdout, "%s: %s\n", fctName, msg);
		fflush(stdout);
	}
}

/**
 * @brief Log error
 * 
 * @param fctName function name of the error
 * @param msg message
 */
void Dao_logError(const char *fctName, const char *msg) {
	fprintf(stderr, "%s: %s\n", fctName, msg);
	fflush(stderr);
}

/**
 * @brief Log debug with format
 * 
 * @param formatAndParams format parameters
 */
void Dao_logDebugFormat(char *formatAndParams, ...) {
	if (getenv("DATA_ACCESS_DEBUG") != NULL) {
		va_list ap;
		va_start(ap, formatAndParams);
		vprintf(formatAndParams, ap);
		printf("\n");
		fflush(stdout);
		va_end(ap);
	}
}

/**
 * @brief Log error with format
 * 
 * @param formatAndParams format parameters
 */
void Dao_logErrorFormat(char *formatAndParams, ...) {
	va_list ap;
	va_start(ap, formatAndParams);
	vprintf(formatAndParams, ap);
	printf("\n");
	fflush(stdout);
	va_end(ap);
}

