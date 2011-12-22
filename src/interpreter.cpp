#include <boost/regex.hpp>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "fcgi_stdio.h"
#include "interpreter.h"
#include "utils.h"

using namespace std;

Interpreter::Interpreter(DB* db,Parser* p){
	qp = p;
	qdb = db;
	exec_flag = 1;
	break_flag = 0;
	if_stack.reserve(20);
	sqlite3_open(":memory:",&sql);
}

Interpreter::~Interpreter(){
	sqlite3_close(sql);
}

void Interpreter::exec(){
	int i;
	string func;
	
	for(i=0; i<input.size(); i++){
		func = toUpper(input[i].func);
		if(func=="ECHO"){
			_echo(input[i]);
		} else if(func=="SQL" || func=="INSERT" || func=="UPDATE" || func=="DELETE"){
			_sql(input[i]);
		} else if(func=="SELECT"){
			_select(input[i]);
		} else if(func=="IF"){
			_if(input[i]);
		} else if(func=="ELSE"){
			_else(input[i]);
		} else if(func=="ELSEIF"){
			_elseif(input[i]);
		} else if(func=="ENDIF"){
			_endif(input[i]);
		} else if(func=="WHILE"){
			_while(input[i],i);
		} else if(func=="WEND"){
			i = _wend(input[i],i);
		} else if(func=="BREAK"){
			_break(input[i]);
		} else if(func=="FOR"){
			_for(input[i],i);
		} else if(func=="NEXT"){
			i = _next(input[i],i);
		} else if(func=="ASC"){
			_asc(input[i]);
		} else if(func=="PREPEND"){
			_prepend(input[i]);
		} else if(func=="INCLUDE"){
			_include(input[i]);
		} else if(func=="CGI"){
			_cgi(input[i]);
		} else {
			_var(input[i]);
		}
	}
}

void Interpreter::_echo(Token &t){
	if(exec_flag){
		result += t.param;
	}
}

void Interpreter::_sql(Token &t){
	if(exec_flag){
		string sql = toUpper(t.func)=="SQL"? t.param : t.func+t.param;
		qdb->exec(replaceVars(sql));
	}
}

void Interpreter::_select(Token &t){
	if(exec_flag){
		string sql = t.func+t.param;

		MYSQL_RES* temp = qdb->getRes();
		qdb->query(replaceVars(sql));
		qdb->saveRow();
		qdb->freeResult();
		qdb->setRes(temp);
	}
}

void Interpreter::_if(Token &t){
	if_state curr = {exec_flag,0};
	if_stack.push_back(curr);

	if(exec_flag){
		string query = replaceVars(t.param);
		if(toUpper(getWord(t.param,0))=="SELECT"){
			MYSQL_RES* temp = qdb->getRes();
			qdb->query(query);
			if(qdb->fetchRow() && qdb->getCol(0)!="1"){
				exec_flag = 0;
			} else {
				if_stack.back().done = 1;
			}
			qdb->freeResult();
			qdb->setRes(temp);
		} else {
			sqlite3_stmt* stmt;
			string query = "SELECT "+replaceVars(t.param);
			sqlite3_prepare(sql,query.c_str(),query.length(),&stmt,NULL);
			if(SQLITE_ROW==sqlite3_step(stmt)){
				int res = sqlite3_column_int(stmt,0);
				if(res!=1){
					exec_flag = 0;
				} else {
					if_stack.back().done = 1;
				}
			}
			sqlite3_finalize(stmt);
		}
	}
}

void Interpreter::_else(Token &t){
	if(!if_stack.empty()){
		if_state last = if_stack.back();
		if(last.exec){
			exec_flag = last.done? 0:1;
			if_stack.back().done = 1;
		}
	}
}

void Interpreter::_elseif(Token &t){
	if(!if_stack.empty()){
		if_state last = if_stack.back();
		if(last.exec){
			if(last.done){
				exec_flag = 0;
			} else {
				string query = replaceVars(t.param);
				if(toUpper(getWord(t.param,0))=="SELECT"){
					MYSQL_RES* temp = qdb->getRes();
					qdb->query(query);
					qdb->fetchRow();
					if(qdb->getCol(0)!="1"){
						exec_flag = 0;
					} else {
						exec_flag = 1;
						if_stack.back().done = 1;
					}
					qdb->freeResult();
					qdb->setRes(temp);
				} else {
					sqlite3_stmt* stmt;
					string query = "SELECT "+replaceVars(t.param);
					sqlite3_prepare(sql,query.c_str(),query.length(),&stmt,NULL);
					if(SQLITE_ROW==sqlite3_step(stmt)){
						int res = sqlite3_column_int(stmt,0);
						if(res!=1){
							exec_flag = 0;
						} else {
							exec_flag = 1;
							if_stack.back().done = 1;
						}
					}
					sqlite3_finalize(stmt);
				}
			}
		}
	}
}

void Interpreter::_endif(Token &t){
	if(!if_stack.empty()){
		if(!break_flag){
			exec_flag = if_stack.back().exec;
		}
		if_stack.pop_back();
	}
}

void Interpreter::_while(Token &t,int count){

	loop_state curr = {exec_flag,count,1,0,"",NULL};
	curr.pre = qdb->getPrepend();
	curr.res = qdb->getRes();
	loop_stack.push_back(curr);

        if(exec_flag){
		string query = replaceVars(t.param);
		string first = toUpper(getWord(t.param,0));
		if(first=="SELECT"){
			qdb->query(query,1);
			curr.lim = qdb->num_rows();
			qdb->saveRow();

			if(curr.acc>curr.lim){
				exec_flag = 0;
			}	
		} else if(first=="1"){
			curr.lim = -1;
		}

		qdb->setVar("LOOP",toString(curr.acc));
		loop_stack.back().lim = curr.lim;
	}
}

int Interpreter::_wend(Token &t,int count){
	if(!loop_stack.empty()){
		loop_stack.back().acc++;
		loop_state last = loop_stack.back();
		if(exec_flag && (last.acc<=last.lim || last.lim==-1)){
			qdb->setPrepend(last.pre);
			qdb->setVar("LOOP",toString(last.acc));
			if(last.lim!=-1){
				qdb->saveRow();
			}
			return last.jmp;
		} else {
			qdb->freeResult();
			qdb->setRes(last.res);
			break_flag = 0;
			exec_flag = last.exec;
			loop_stack.pop_back();
			if(!loop_stack.empty()){
				qdb->setVar("LOOP",toString(loop_stack.back().acc));
			}
		}
	}
	return count;
}

void Interpreter::_break(Token &t){
	if(!loop_stack.empty() && exec_flag){
		break_flag = 1;
		exec_flag = 0;
		loop_stack.back().acc = loop_stack.back().lim;
	}
}

void Interpreter::_for(Token &t,int count){
	loop_state curr = {exec_flag,count,0,0,"",NULL};
	t.param = replaceVars(t.param);
	curr.acc = atoi(getWord(t.param,0).c_str()); 
	curr.lim = atoi(getWord(t.param,2).c_str());

        loop_stack.push_back(curr);

        if(exec_flag){
		qdb->setVar("LOOP",toString(curr.acc));
		if(curr.acc>curr.lim){
			exec_flag = 0;
		}
        }
}

int Interpreter::_next(Token &t,int count){
	if(!loop_stack.empty()){
		loop_stack.back().acc++;
		loop_state last = loop_stack.back();
		if(exec_flag && last.acc<=last.lim){
			qdb->setVar("LOOP",toString(last.acc));
			return last.jmp;
		} else {
			break_flag = 0;
			exec_flag = last.exec;
			loop_stack.pop_back();
			if(!loop_stack.empty()){
				qdb->setVar("LOOP",toString(loop_stack.back().acc));
			}
		}
	}
	return count;
}

void Interpreter::_asc(Token &t){
	if(exec_flag){
		int code = atoi(replaceVars(t.param).c_str());
		if(code>=0 && code<255){
			result += char(code);
		} else if(code==255){
			result += "\n";
		}
	}
}

void Interpreter::_prepend(Token &t){
	if(exec_flag){
		t.param = replaceVars(t.param);
		qdb->setPrepend(getWord(t.param,0));
	}
}

void Interpreter::_include(Token &t){
	if(exec_flag){
		t.param = replaceVars(t.param);
		string tpl = qdb->getVar("DOCUMENT_ROOT") + getWord(t.param,0);
		string tph = getWord(t.param,1);
		
		if(qp->read(tpl,tph)){
			Interpreter qi(qdb,qp);
			qi.setTokens(qp->parse());
                        qi.exec();
			result += qi.getResult();
		}
	}
}

void Interpreter::_cgi(Token &t){
	if(exec_flag){
		t.param = replaceVars(t.param);
		qdb->setVar(getWord(t.param,0),getWord(t.param,1));
	}
}

void Interpreter::_var(Token &t){
	if(exec_flag && t.param==""){
		string varname = "@"+t.func;
		result += replaceVars(varname);
	}
}

void Interpreter::flush(){
	if(qdb->getVar("content-type")=="application/json"){
		printf("%s",replace(result,"\n\r\f",' ').c_str());
	} else {
		printf("%s",result.c_str());
	}
	result.assign("");
}

void Interpreter::clear(){
	result.assign("");
}

string Interpreter::replaceVars(string &str){
	boost::smatch matches;
	boost::regex re;
	re.assign("(@)?@(\\w+)(\\$(\\w+))?(`(ESC|NUM|DATE))?",boost::regex_constants::icase);

	string output;
	string leftovers = str;
	string::const_iterator begin,end;
	begin = str.begin();
	end = str.end();

	while(boost::regex_search(begin,end,matches,re)){
		string sys(matches[1].str());
		string var(matches[2].str());
		string idx(matches[5].str());
		string fmt(matches[6].str());
		string text(matches.prefix().str());
		string res;

		if(text!=""){
			output.append(text);
		}

		if(sys==""){
			if(var!=""){
				if(idx!=""){
					res = qdb->getVar(var+qdb->getVar(idx));
				} else {
					res = qdb->getVar(var);
				}
			}
		} else {
			res = sys+var;
		}

		if(fmt!=""){
			res = escapeJS(res);
		}

		output.append(res);

		begin = matches.suffix().first;
		leftovers = matches.suffix().str();
	}

	if(leftovers!=""){
		output.append(leftovers);
	}

	return output;
}
