/*
 * commons.h
 *
 *  Created on: 13 fevr. 2020
 *      Author:
 */

#ifndef COMMONS_COMMONS_H_
#define COMMONS_COMMONS_H_

#include <time.h>

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

char* allocStr(const char *formatAndParams, ...);
char *allocStrPlusSize(int plusSize, const char *formatAndParams, ...);
char *reallocStr(char *str, const char *formatAndParams, ...);
char* uitoa(unsigned int uint);
char* inttoa(int i);
char *dbltoa(double dbl);
void arrayFree(char **array, int nb);
char* arrayJoin(const char *fields[], int nb, char *sep);
void arrayAddElement(char **array, char *element, int idx, int allocElement);
void arrayAddTimeElement(char **array, time_t element, int idx);
void arrayAddDoubleElement(char **array, double element, int idx);
void arrayAddIntElement(char **array, int element, int idx);
char** arrayConcat(char *t1[], int nbT1, char *t2[], int nbT2);

#endif /* COMMONS_COMMONS_H_ */
