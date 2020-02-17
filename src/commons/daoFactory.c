#include "commons/daoFactory.h"
#include "dao/pg/pgDao.h"

Dao* factoryInit() {
	return (Dao*) PgDao_new();
}

void* factoryEnd() {
	// TODO Free;
	return 0;
}
