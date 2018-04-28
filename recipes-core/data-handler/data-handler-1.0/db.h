/* Sukhorukov <sukhourkov.mail@gmail.com> */
#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdint.h>

#define DB_KEY_FIELD        "key"
#define DB_JSON_FIELD       "json"
#define DB_IS_SENT_FIELD    "is_sent"

/* wrapper around "sqlite3_exec"
 *
 * inputs:
 *      ppDb         i   SQLite db handle
 *      sql          i   SQL to be evaluated
 *      callback     o   callback function
 *
 * returns:
 *      sqlite return code
 */
int db_exec(sqlite3 **ppDb, char *sql, int (* callback)(void *, int, char **, char **));

/* open db
 * create tables if not exists
 *
 * inputs:
 *      filename     i   Database filename (UTF-8)
 *      ppDb         o   SQLite db handle
 *
 * returns:
 *      sqlite return code
 */
int db_open(const char *filename, sqlite3 **ppDb);

/* insert new item to 'sensors' table
 *
 * inputs:
 *      ppDb         i   SQLite db handle
 *      json         i   object json (data from sensors)
 *      key          o   primary key
 *
 * returns:
 *      sqlite return code
 */
int db_add_item(sqlite3 **ppDb, char *json, char *key);

int db_delete_item(sqlite3 **ppDb, char * key);

/* set 'is_sent' value in true
 *
 * inputs:
 *      ppDb         i   SQLite db handle
 *      key          i   primary key
 *
 * returns:
 *      sqlite return code
 */
int db_item_sended(sqlite3 **ppDb, char * key);

/* delete all sent items (is_sent == true)
 *
 * inputs:
 *      ppDb         i   SQLite db handle
 *
 * returns:
 *      sqlite return code
 */
int db_cleanup(sqlite3 **ppDb);

/* get items (is_sent == false)
 *
 * inputs:
 *      ppDb         i   SQLite db handle
 *      callback     o   callback function
 *
 * returns:
 *      sqlite return code
 */
int db_get_items(sqlite3 **ppDb, int (* callback)(void *, int, char **, char **));


#endif // DB_H
