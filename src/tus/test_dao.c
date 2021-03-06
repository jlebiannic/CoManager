/*
 * test_daservice.c
 *
 *  Created on: 18 mai 2020
 *
 */

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include <stdarg.h>
#include "commons/commons.h"
#include "dao/dao.h"
#include "commons/daoFactory.h"

static const char *TABLE = "tudao";
static const char *TX_INDEX = "TX_INDEX";
static const char *STATUS = "STATUS";
static const char *VAL = "VAL";
static const char *CREATED = "CREATED";
static const char *MODIFIED = "MODIFIED";
static const char *NEXT = "NEXT";
static const char *MSG = "MSG";
static const char *GENERATION = "GENERATION";
static const char *RESERVED1 = "RESERVED1";

static void test_verifyEntry(Dao *dao, int idx, const char *status, double val);

static void assertTrue(char *functionName, const char *paramName, int res) {
	if (res != TRUE) {
		fprintf(stderr, "ERROR: %s, résultat pour %s attendu %d mais obtenu %d\n", functionName, paramName, TRUE, res);
		exit(1);
	} else {
		printf("OK: %s, assertTrue %s is true\n", functionName, paramName);
	}
}

static void assertIntEquals(char *functionName, const char *paramName, int val1, int val2) {
	if (val1 != val2) {
		fprintf(stderr, "ERROR: %s, %s est différent de %d\n", functionName, paramName, val2);
		exit(1);
	} else {
		printf("OK: %s, assertIntEquals %s=%d\n", functionName, paramName, val2);
	}
}

static void assertIntNotEquals(char *functionName, const char *paramName, int val1, int val2) {
	if (val1 == val2) {
		fprintf(stderr, "ERROR: %s, %s est égal à %d\n", functionName, paramName, val2);
		exit(1);
	} else {
		printf("OK: %s, assertIntNotEquals %s<>%d\n", functionName, paramName, val2);
	}
}

static void assertStrEquals(char *functionName, const char *paramName, const char *val1, const char *val2) {
	if (strcmp(val1, val2) != 0) {
		fprintf(stderr, "ERROR: %s, %s est différent de %s\n", functionName, paramName, val2);
		exit(1);
	} else {
		printf("OK: %s, assertStrEquals %s=%s\n", functionName, paramName, val2);
	}
}

static void assertDblEquals(char *functionName, const char *paramName, double val1, double val2) {
	if (val1 != val2) {
		fprintf(stderr, "ERROR: %s, %s est différent de %f\n", functionName, paramName, val2);
		exit(1);
	} else {
		printf("OK: %s, assertStrEquals %s=%f\n", functionName, paramName, val2);
	}
}

// ==========================================================================

static void test_createTable(Dao *dao) {
    const char *fields[9] = { TX_INDEX, NEXT, CREATED, MODIFIED, VAL, GENERATION, RESERVED1, MSG , STATUS};
	const char *types[9] = { "INTEGER", "INTEGER", "INTEGER", "INTEGER", "NUMERIC", "INTEGER", "INTEGER", "TEXT", "TEXT" };
	int res = dao->createTable(dao, TABLE, fields, types, 9, 0, TRUE);
	assertTrue("test_createTable", "res", res);
}

static void test_createIndex(Dao *dao) {
	const char *indexeFields[3] = { "TX_INDEX", "VAL" };
	int res = dao->createIndex(dao, TABLE, "IDX_TEST_123", indexeFields, 2);
	assertTrue("test_createIndex", "res", res);
}

static int test_newEntry(Dao *dao) {
	const char *fields[8] = { NEXT, CREATED, MODIFIED, VAL, GENERATION, RESERVED1, MSG , STATUS};
	const char *values[8] = { "0", "0", "0", "0", "0", "0", "" , ""};
	int idx = dao->newEntry(dao, TABLE, fields, values, 8, "tx_index");
	assertIntNotEquals("test_newEntry", "idx", idx, 0);
	test_verifyEntry(dao, idx, "", 0);
	return idx;
}

static void test_findEntry(Dao *dao, int idx) {
	const char *fields[1] = { TX_INDEX };
	const char *values[1] = { inttoa(idx) };
	int res = dao->getEntries(dao, TABLE, fields, 1, "tx_index=$", values, NULL, 0, -1,-1, FALSE);
	assertTrue("test_findEntry", "res", res);
	assertIntEquals("test_findEntry", "getNbResults", dao->getNbResults(dao), 1);
	assertIntEquals("test_findEntry", "TX_INDEX", dao->getFieldValueAsInt(dao, TX_INDEX), idx);
}

static void test_updateEntries(Dao *dao, int idx, const char *status, const char *val) {
	const char *fields[2] = { STATUS, VAL };
	const char *values[2] = { status, val };
	const char *filterValues[1] = { inttoa(idx) };
	int res = dao->updateEntries(dao, TABLE, fields, values, 2, "TX_INDEX=$", filterValues);
	assertTrue("test_updateEntries", "res", res);
}

static void test_verifyEntry(Dao *dao, int idx, const char *status, double val) {
	const char *fields[2] = { STATUS, VAL };
	const char *values[1] = { inttoa(idx) };
	int res = dao->getEntries(dao, TABLE, fields, 2, "tx_index=$", values, NULL, 0, -1,-1, FALSE);
	assertTrue("test_verifyEntry", "res", res);
	assertIntEquals("test_verifyEntry", "getNbResults", dao->getNbResults(dao), 1);
	assertStrEquals("test_verifyEntry", "STATUS", dao->getFieldValue(dao, STATUS), status);
	assertDblEquals("test_verifyEntry", "VAL", dao->getFieldValueAsDouble(dao, VAL), val);

	const char *fields2[1] = { TX_INDEX };
	const char *values2[2] = { status, dbltoa(val) };
	res = dao->getEntries(dao, TABLE, fields2, 1, "status=$ and val=$", values2, NULL, 0, -1,-1, FALSE);
	assertTrue("test_verifyEntry", "res", res);
    assertIntEquals("test_verifyEntry", "getNbResults", dao->getNbResults(dao), 1);
	assertIntEquals("test_verifyEntry", "idx", dao->getFieldValueAsInt(dao, TX_INDEX), idx);
}

static void test_verifyEntry2(Dao *dao, int idx1, int idx2, const char *status1, double val1, const char *status2, double val2) {
	const char *fields[2] = { STATUS, VAL };
	const char *values[2] = { inttoa(idx1), inttoa(idx2) };
    const char *sortFields[1] = {"tx_index"};
    int res = dao->getEntries(dao, TABLE, fields, 2, "tx_index=$ or tx_index=$", values, sortFields, 1, 2, 0, TRUE);
	assertTrue("test_verifyEntry2", "res", res);
	assertIntEquals("test_verifyEntry2", "getNbResults", dao->getNbResults(dao), 2);
	int cpt = 0;
	while (dao->hasNextEntry(dao)) {
		char *status = dao->getFieldValueByNum(dao, 0);
		double val = dao->getFieldValueAsDoubleByNum(dao, 1);
		if (cpt == 0) {
			assertStrEquals("test_verifyEntry2", "STATUS", status, status1);
			assertDblEquals("test_verifyEntry2", "VAL", val, val1);
		} else {
			assertStrEquals("test_verifyEntry2", "STATUS", status, status2);
			assertDblEquals("test_verifyEntry2", "VAL", val, val2);
		}
		dao->getNextEntry(dao);
		cpt++;
	}
	assertIntEquals("test_verifyEntry2", "cpt", cpt, 2);
}

static void test_verifyEntryCount(Dao *dao, const char* tableName, int count) {
	const char *fields[1] = { "cnt" };
	const char *values[1] = { tableName };
	int res = dao->getEntries(dao, "entry_count", fields, 1, "table_name=$", values, NULL, 0, -1,-1, FALSE);
	assertTrue("test_verifyEntryCount", "res", res);
    assertTrue("test_verifyEntryCount", "hasNextEntry", dao->hasNextEntry(dao));
	assertIntEquals("test_verifyEntryCount", "CNT", dao->getFieldValueAsInt(dao, "CNT"), count);
}

static void test_addTrigerEntryCount(Dao *dao){
    int res = dao->createTriggersEntryCount(dao, TABLE);
	assertTrue("test_verifyEntryCount", "res", res);
}

static void test_removeEntries(Dao *dao, const char *val1, const char *val2) {
	const char *values[2] = { val1, val2 };
	int res = dao->removeEntries(dao, TABLE, "status = $ or status = $", values);
	assertTrue("test_removeEntries", "res", res);
}

static void test_verifyApiEntriesCount(Dao *dao, int count) {
	//const char *values[2] = { val1, val2 };
	int nb = dao->getEntriesCount(dao, TABLE, NULL, NULL);
	assertIntEquals("test_verifyApiEntriesCount", "nb", nb, count);
}

static void test_verifyApiEntriesCountWithFilter(Dao *dao, const char *status, double val, int count) {
	const char *values[2] = { status, dbltoa(val) };
	int nb = dao->getEntriesCount(dao, TABLE, "status=$ and val=$", values);
    assertIntEquals("test_verifyApiEntriesCountWithFilter", "nb", nb, count);
}

static void test_OffsetLimit(Dao *dao, int offset, int limit) {
	const char *fields[2] = { STATUS, VAL };
    int res = dao->getEntries(dao, TABLE, fields, 2, NULL, NULL, NULL, 0, limit, offset, TRUE);
	assertTrue("test_OffsetLimit", "res", res);
	assertIntEquals("test_OffsetLimit", "getNbResults", dao->getNbResults(dao), limit);
	int cpt = 0;
	while (dao->hasNextEntry(dao)) {
		double val = dao->getFieldValueAsDoubleByNum(dao, 1);
		assertDblEquals("test_OffsetLimit", "VAL", val, cpt+1+offset);
		dao->getNextEntry(dao);
		cpt++;
	}
	assertIntEquals("test_OffsetLimit", "cpt", cpt, limit);
}

static void test_rollbackTrans(Dao *dao, int idx) {
	const char *fields[2] = { STATUS, VAL };
	const char *values[1] = { inttoa(idx) };
	int res = dao->getEntries(dao, TABLE, fields, 2, "tx_index=$", values, NULL, 0, -1,-1, FALSE);
	assertTrue("test_verifyEntry", "res", res);
	assertIntEquals("test_verifyEntry", "getNbResults", dao->getNbResults(dao), 1);
	char* status = dao->getFieldValue(dao, STATUS);
	double val = dao->getFieldValueAsDouble(dao, VAL);

	const char *fields2[2] = { STATUS, VAL };
	const char *values2[2] = { "for rollback", "0" };
	const char *filterValues[1] = { inttoa(idx) };
    res = dao->beginTrans(dao);
    assertTrue("test_rollbackTrans", "res", res);
	res = dao->updateEntries(dao, TABLE, fields2, values2, 2, "TX_INDEX=$", filterValues);
	assertTrue("test_rollbackTrans", "res", res);
    test_verifyEntry(dao, idx, "for rollback", 0);
	res = dao->rollbackTrans(dao);
    assertTrue("test_rollbackTrans", "res", res);
	test_verifyEntry(dao, idx, status, val);
}

int main(int argc, char **argv) {

	Dao *dao = daoFactory_create(1);
	int res = dao->openDB(dao, NULL);
	assertTrue("openDB", "res", res);

	test_createTable(dao);
	test_createIndex(dao);
	
	test_addTrigerEntryCount(dao);
	int idx = test_newEntry(dao);
	test_findEntry(dao, idx);
	test_updateEntries(dao, idx, "Modified", "123.456");
	test_verifyEntry(dao, idx, "Modified", 123.456);
	int idx2 = test_newEntry(dao);
	test_updateEntries(dao, idx2, "Modified2", "789.01");
	test_verifyEntry2(dao, idx, idx2, "Modified", 123.456, "Modified2", 789.01);
    test_verifyEntryCount(dao, TABLE, 2);

    test_verifyApiEntriesCount(dao, 2);
	test_verifyApiEntriesCountWithFilter(dao, "Modified2", 789.01, 1);

    test_removeEntries(dao, "Modified", "Modified2");
	test_verifyEntryCount(dao, TABLE, 0);

	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified1", "1");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified2", "2");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified3", "3");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified4", "4");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified5", "5");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified6", "6");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified7", "7");
	idx = test_newEntry(dao);
	test_updateEntries(dao, idx, "Modified8", "8");

	test_OffsetLimit(dao, 0, 5);
	test_OffsetLimit(dao, 5, 3);

	test_rollbackTrans(dao, idx);

    dao->freeDao(dao);

    return 0;
}
