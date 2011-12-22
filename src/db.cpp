#include "db.h"
#include "utils.h"

using namespace std;

DB::DB(){
	hasResult = 0;
	prepend = "";
	first = 0;
	mysql_init(&conn);
	mysql_options(&conn,MYSQL_OPT_RECONNECT,"true");
	mysql_real_connect(&conn,"localhost","root","test",NULL,0,NULL,0);	
}

DB::DB(const char *host,const char *user,const char *pass,const char *db,unsigned int port){
	hasResult = 0;
	prepend = "";
	first = 0;
	mysql_init(&conn);
	mysql_options(&conn,MYSQL_OPT_RECONNECT,"true");
	mysql_real_connect(&conn,host,user,pass,db,port,NULL,0);	
}

DB::~DB(){
	mysql_close(&conn);
}

void DB::exec(string sql,int log){
	mysql_ping(&conn);
	string type = toLower(getWord(sql,0));
	string id = getVar("cookie_user");

	if(log) trace(id,type,"exec: "+sql);
	if(mysql_real_query(&conn,sql.c_str(),sql.length())){
		if(log) trace(id,type,"exec_err: "+string(mysql_error(&conn)));
	}
}

void DB::query(string sql,int store,int log){
	if(!first){
		prepend = "";
	} else {
		first = 0;
	}

	mysql_ping(&conn);
	string type = toLower(getWord(sql,0));
	string id = getVar("cookie_user");

	if(log) trace(id,type,"query: "+sql);
	if(mysql_real_query(&conn,sql.c_str(),sql.length())){
		hasResult = 0;
		if(log) trace(id,type,"query_err: "+string(mysql_error(&conn)));
	} else {
		MYSQL_RES* temp;
		if(store){
			if(temp = mysql_store_result(&conn)){
				hasResult = 1;
				res = temp;
			}
		} else {
			if(temp = mysql_use_result(&conn)){
				hasResult = 1;
				res = temp;
			}
		}
	}
}

int DB::fetchRow(){
	int ret = 0;
	if(hasResult){
		if(row = mysql_fetch_row(res)){
			fields = mysql_fetch_fields(res);
			ret = 1;
		}
	}
	return ret;
}

int DB::saveRow(){
	if(fetchRow()){
		int len = num_fields();
		int i;

		for(i=0; i<len; i++){
			setVar(prepend+fields[i].name,getCol(i));
		}
	}
}

string DB::getCol(const int col){
	if(row[col]){
		return string(row[col]);
	} else {
		return "";
	}
}

void DB::freeResult(){
	if(hasResult){
		mysql_free_result(res);
		hasResult = 0;
	}
}

int DB::num_rows(){
	int ret = 0;
	if(hasResult){
		ret = mysql_num_rows(res);
	}
	return ret;
}

int DB::num_fields(){
	int ret = 0;
	if(hasResult){
		ret = mysql_num_fields(res);
	}
	return ret;
}

void DB::clearVars(){
	vars.clear();
}

void DB::setVar(string var,string val){
	vars[var] = val;
}

string DB::getVar(string var){
	if(vars.find(var) == vars.end()){
		return "";
	} else {
		return vars[var];
	}
}

int DB::checkVar(string var){
	if(vars.find(var) == vars.end()){
		return 0;
	} else {
		return 1;
	}
}
