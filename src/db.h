#ifndef DB_H
#define DB_H

#include <map>
#include <mysql/mysql.h>
#include <string>

using namespace std;

class DB {
	private:
		MYSQL conn;
		MYSQL_RES *res;
		MYSQL_ROW row;
		MYSQL_FIELD *fields;
		map<string,string> vars;
		string prepend;
		int first;
		int hasResult;

	public:
		DB();
		DB(const char *host,const char *user,const char *pass,const char *db=NULL,unsigned int port=3306);

		~DB();

		void exec(string sql,int log=1);
		void query(string sql,int store=0,int log=1);

		int fetchRow();
		int saveRow();
		string getCol(const int col);
		void freeResult();

		int num_rows();
		int num_fields();

		void clearVars();
		void setVar(string var,string val);
		string getVar(string var);
		int checkVar(string var);

		void setPrepend(string pre){ prepend=pre; first=1; }
		string getPrepend(){ return prepend; }

		void setRes(MYSQL_RES* result){ res=result; hasResult=1; }
		MYSQL_RES* getRes(){ return res; }
};

#endif
