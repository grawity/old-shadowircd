/*
 *  datastorage.h
 *  kageserv
 *
 *  Created by Administrator on Sat Aug 28 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <string>
#include <mysql.h>
using namespace std;

static MYSQL database;
bool database_connect();
 // mysql_init(...)
 // mysql_real_connect(database,host,user,passwd,db,port,unix_socket,client_flag);

void database_cleanup();
 // mysql_close

string database_escape(string&);

int database_query(char *fmt, ...);
int database_raw_query(string query);
 // mysql_query(&database, query) (true, [error])
 // MYSQL_RES *mysql_store_result(&database) (false, field_count == 0, [error])
 // MYSQL_FIELD *mysql_fetch_fields(result)
 // MYSQL_ROW mysql_fetch_row(result) (NULL, [no more rows])
 // mysql_free_result
 // ret mysql_num_rows

/* MYSQL_FIELD

char * name
char * table
...
enum enum_field_types type // IS_NUM(field->type)
...
*/

#endif