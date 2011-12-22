#ifndef SESSIONS_H
#define SESSIONS_H

#include <string>
#include <sqlite3.h>
#include "db.h"

using namespace std;

class Sessions {
	private:
		DB* qdb;
		sqlite3* sql;
		sqlite3_stmt* stmt;

		int new_session;
		int new_login;

		string generate();

	public:
		Sessions(DB* db);
		~Sessions();
		
		int init();
		int set();
		int login();
};

#endif
