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

	dao->closeDB(dao);

	factoryEnd();

	puts("Main end");
	return EXIT_SUCCESS;
}
