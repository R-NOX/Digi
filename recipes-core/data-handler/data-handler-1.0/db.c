/* Sukhorukov <sukhourkov.mail@gmail.com> */
#include "db.h"
#include "rnox/log.h"

#include <stdbool.h>
//#include <uuid/uuid.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE
#include <stdio.h>
int asprintf(char **strp, const char *fmt, ...);

int db_exec(sqlite3 **ppDb, char *sql, int (* callback)(void *, int, char **, char **))
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    int return_code = SQLITE_OK;
    char *err_msg = 0;

    return_code = sqlite3_exec(*ppDb, sql, callback, 0, &err_msg);
    if (return_code != SQLITE_OK ) {

        log_print(LOG_MSG_ERR, "SQL: %s", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(*ppDb);

        free(sql);
        return return_code;
    }
    else log_print(LOG_MSG_DEBUG, "SQL: request successfully accepted: \"%s\"", sql);

    return return_code;
}

int db_open(const char *filename, sqlite3 **ppDb)
{
    int return_code = SQLITE_OK;
    char *sql = "CREATE TABLE IF NOT EXISTS "
                "sensors(" DB_KEY_FIELD " INTEGER PRIMARY KEY,"
                DB_JSON_FIELD " TEXT NOT NULL);";

    // open database
    return_code =sqlite3_open(filename, ppDb);
    if (return_code != SQLITE_OK) {
        log_print(LOG_MSG_ERR, "cannot open database: %s", sqlite3_errmsg(*ppDb));
        sqlite3_close(*ppDb);
        return return_code;
    }
    else log_print(LOG_MSG_INFO, "SQL: database is open: \"%s\"", filename);

    // create 'sensors' table if not exists
    return_code = db_exec(ppDb, sql, NULL);

    return return_code;
}

int db_add_item(sqlite3 **ppDb, char *json, char *key)
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    char *sql = NULL;
    int return_code = SQLITE_OK;
//    uuid_t uuid;
    char _key[37] = { 0 };
//
//    // primary key
//    uuid_generate(uuid);
//    uuid_unparse(uuid, _key);

    asprintf(&sql, "INSERT INTO sensors VALUES (NULL, '%s');", json);

    return_code = db_exec(ppDb, sql, NULL);

    free(sql);

    if (key != NULL) strcpy(key, _key);
    return return_code;
}

int db_delete_item(sqlite3 **ppDb, char * key)
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    char *sql = NULL;
    int return_code = SQLITE_OK;

    asprintf(&sql, "DELETE FROM sensors WHERE " DB_KEY_FIELD " = '%s';", key);

    return_code = db_exec(ppDb, sql, NULL);

    free(sql);

    return return_code;
}

int db_item_sended(sqlite3 **ppDb, char * key)
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    char *sql = NULL;
    int return_code = SQLITE_OK;

    asprintf(&sql, "UPDATE sensors SET " DB_IS_SENT_FIELD " = 1 WHERE " DB_KEY_FIELD " = '%s';", key);

    return_code = db_exec(ppDb, sql, NULL);

    free(sql);
    return return_code;
}

int db_cleanup(sqlite3 **ppDb)
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    char *sql = NULL;
    int return_code = SQLITE_OK;

    asprintf(&sql, "DELETE FROM sensors;");

    return_code = db_exec(ppDb, sql, NULL);

    free(sql);
    return return_code;
}

int db_get_items(sqlite3 **ppDb, int (* callback)(void *, int, char **, char **))
{
    if (*ppDb == NULL) return SQLITE_ERROR;

    char *sql = NULL;
    int return_code = SQLITE_OK;

    asprintf(&sql, "SELECT * FROM sensors;");

    return_code = db_exec(ppDb, sql, callback);

    free(sql);
    return return_code;
}
