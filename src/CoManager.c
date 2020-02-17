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

#include "commons/daoFactory.h"

int main(void) {
	puts("Main start");

	Dao *dao = factoryInit();
	dao->openDB(dao, NULL);

	dao->getElement(dao, "data", "\"TX_INDEX\" = 1");
	char *str = dao->getFieldValue(dao, "next");
	printf("next is %s\n", str);

	dao->closeDB(dao);

	factoryEnd();

	puts("Main end");
	return EXIT_SUCCESS;
}
