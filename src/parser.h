#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

using namespace std;

struct Token {
	string func;
	string param;
};

typedef std::vector <Token> Tokens;

class Parser {
	private:
		string str;

	public:
		Parser();
		Parser(string &input);

		int read(string tpl, string tph);
		Tokens parse();

		void setString(string &input){ str=input; }
		string getString(){ return str; }
};

#endif
