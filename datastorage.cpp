/*
 *  datastorage.cpp
 *  kageserv
 *
 *  Created by Administrator on Sat Aug 28 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include "datastorage.h"
#include <mysql.h>
using namespace std;

extern MYSQL database;
bool database_connect()
{
 if (mysql_init(&database) == NULL)
 { cerr << "Error: Could not allocate enough memory for MYSQL" << endl; return false;
 }
 else if (mysql_real_connect(&database,"localhost","eko","s3rv1c3s","KageServ",0,NULL,0) == NULL)
 { cerr << "Error: Could not connect to database:" << endl;
   cerr << mysql_error(&database) << endl;
   return false;
 }
 atexit(&database_cleanup);
 return true;
} // database_connect()

void database_cleanup()
{ mysql_close(&database);
} // database_cleanup()

string database_escape(string &str)
{
  string out;
  string::iterator c = str.begin();
  while (c != str.end())
  {
    switch (*c)
    {
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\'':
        out += "\\'";
        break;
      case '\"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      default:
        out += *c;
    }
    c++;
  }
  return out;
}

int database_query( char *fmt, ... )
{
  static char message[4096];
  char *format;
  memset(message, '\0', 4096);
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(message, 4096, fmt, ap);
  va_end(ap);
  return database_raw_query(message);
}

// Needed for the next functions
static vector<MYSQL_ROW> rows;
MYSQL_FIELD *fields;

int database_raw_query(string query)
{
  // Variables
  MYSQL_RES *result;
  MYSQL_ROW row;
  // Clear out the rows from the previous query
  rows.clear();
  // Send the query to the server
  cout << "[[ " << query << " ]]" << endl;
  if (mysql_query(&database, query.c_str())) // (true, [error])
  { cerr << "Error: Could not execute query:" << endl;
    cerr << mysql_error(&database) << endl;
    return -1;
  }
  // Store the result locally
  if ((result = mysql_store_result(&database)) == NULL) // (false, field_count != 0, [error])
  { 
    if (mysql_field_count(&database) != 0)
    { cerr << "Error: Error in query:" << endl;
      cerr << mysql_error(&database) << endl;
      return -1;
    }
    return mysql_affected_rows(&database); 
  }
  // Retrieve the field list
  fields = mysql_fetch_fields(result);
  // Store the rows in a vector for easy accessing
  while ((row = mysql_fetch_row(result)) != NULL) // (NULL, [no more rows])
    rows.push_back(row);
  // Free the result pointer
  mysql_free_result(result);
  // Return the number of rows returned
  return rows.size();
}
/* MYSQL_FIELD

char * name
char * table
...
enum enum_field_types type // IS_NUM(field->type)
...
*/