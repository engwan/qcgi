#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <sqlite3.h>
#include "db.h"
#include "parser.h"

using namespace std;

struct if_state {
        int exec;
        int done;
};

struct loop_state {
        int exec;
        int jmp;
        int acc;
        int lim;
        string pre;
	MYSQL_RES *res;
};

class Interpreter {
	private:
		Parser* qp;
		DB* qdb;
		sqlite3* sql;

		Tokens input;
		vector<if_state> if_stack;
		vector<loop_state> loop_stack;
		int exec_flag;
		int break_flag;

		string result;

		void _echo(Token &t);

		void _sql(Token &t);
		void _select(Token &t);

		void _if(Token &t);
		void _else(Token &t);
		void _elseif(Token &t);
		void _endif(Token &t);

		void _while(Token &t,int i);
		int _wend(Token &t,int i);

		void _break(Token &t);

		void _for(Token &t,int i);
		int _next(Token &t,int i);

		void _asc(Token &t);
		void _prepend(Token &t);

		void _include(Token &t);

		void _cgi(Token &t);

		void _var(Token &t);

		string replaceVars(string &str);

	public:
		Interpreter(DB* db,Parser* qp);
		~Interpreter();

		void exec();
		void flush();
		void clear();

		void setTokens(Tokens in){ input=in; }
		Tokens getTokens(){ return input; }
		string getResult(){ return result; }
};

#endif
