#include <cstdlib>
#include "sessions.h"
#include "utils.h"

using namespace std;

Sessions::Sessions(DB* db){
	qdb = db;
	sqlite3_open("/tmp/.qcgi_sessions.db",&sql);
	string query = "CREATE TABLE IF NOT EXISTS sessions (id TEXT PRIMARY KEY, cookie_user NUMERIC, expires NUMERIC)";
	sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);
}

Sessions::~Sessions(){
	sqlite3_close(sql);
}

int Sessions::init(){
	new_login = 0;
	new_session = 0;
	int ret = 0;
	string session(qdb->getVar("QCGI_SESSID"));
	string query;

	if(qdb->checkVar("op") && qdb->getVar("op")=="logout"){
		query = "DELETE FROM sessions WHERE id = '"+session+"'";
		sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);

		qdb->setVar("QCGI_SESSID","U");
		new_session = 1;
		new_login = 1;
	} else {
		query = "SELECT cookie_user, expires, strftime('%s','now') as now FROM sessions WHERE id = '"+session+"'";
		sqlite3_prepare(sql,query.c_str(),query.length(),&stmt,NULL);

		int max_tries = 3;
		int ret_code = sqlite3_step(stmt);
		while(ret_code==SQLITE_BUSY && max_tries>0){
			trace("0","sessions","retrying_"+toString(max_tries)+": "+sqlite3_errmsg(sql));
			sleep(300);
			ret_code = sqlite3_step(stmt);
			max_tries--;
		}

		if(ret_code==SQLITE_ROW){
			if(sqlite3_column_int(stmt,1)>sqlite3_column_int(stmt,2)){
				string user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
				qdb->setVar("cookie_user",user_id);

				/*query = "UPDATE sessions SET expires = strftime('%s','now','+1 hour') WHERE id = '"+session+"'";
				sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);*/

				ret = 1;
			} else {
				query = "DELETE FROM sessions WHERE id = '"+session+"'";
				sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);

				qdb->setVar("mesg","You have been logged out due to inactivity. Please login again.");
				qdb->setVar("QCGI_SESSID","U");
				new_session = 1;
			}
		} else {
			if(qdb->checkVar("cookie_user") && qdb->checkVar("cookie_pass")){

				query = "SELECT id FROM security.users WHERE username = '"+qdb->getVar("cookie_user")+"' and password = md5('"+qdb->getVar("cookie_pass")+"')";	
				qdb->query(query,false,false);	

				if(qdb->fetchRow()){
					session = generate();
					qdb->setVar("cookie_user",qdb->getCol(0));
					qdb->setVar("QCGI_SESSID",session);
					
					query = "INSERT INTO sessions (id,cookie_user,expires) VALUES ('"+session+"','"+qdb->getCol(0)+"',strftime('%s','now','+1 day'))";
					sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);

					ret = 1;
					new_session = 1;
					new_login = 1;
				} else {
					qdb->setVar("mesg","Invalid username or password.");
				}	
				qdb->freeResult();
			} else {
				qdb->setVar("QCGI_SESSID","U");
				new_session = 1;
			}
		}
		ret_code = sqlite3_finalize(stmt);
		if(ret_code!=SQLITE_OK){
			trace("0","sessions",sqlite3_errmsg(sql));
		}
	}

	query = "DELETE FROM sessions WHERE expires <= strftime('%s','now')";
	sqlite3_exec(sql,query.c_str(),NULL,NULL,NULL);

	return ret;
}

int Sessions::set(){
	return new_session;
}

int Sessions::login(){
	return new_login;
}

string Sessions::generate(){
	string str;
	string possible = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	int numberOfPossibilities = possible.length();
	int length = 32;

	for (int i=0; i<length; i++){
		str += possible[rand() % numberOfPossibilities];
	}

	return str;
}
