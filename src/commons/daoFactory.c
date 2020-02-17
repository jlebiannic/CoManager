#include "commons/daoFactory.h"
#include "dao/pg/pgDao.h"

Dao* factoryInit() {
	return (Dao*) PgDao_New();
}

void* factoryEnd() {
	// TODO Free;
	return 0;
}
