/*
 ============================================================================
 Name        : CoManager.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "commons/daoFactory.h"

int main(void) {
	puts("Main start");

	Dao *dao = factoryInit();
	dao->openDB(dao, NULL);

	dao->getElement(dao, "data", "\"TX_INDEX\" = 1");
	char *str = dao->getFieldValue(dao, "next");
	printf("next is %s\n", str);

	dao->addEntry(dao, "data", time(NULL));
	dao->addEntry(dao, "data", 0);

	dao->closeDB(dao);

	factoryEnd();

	puts("Main end");
	return EXIT_SUCCESS;
}
