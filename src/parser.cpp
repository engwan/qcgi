#include <boost/regex.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "parser.h"
using namespace std;

Parser::Parser(){}

Parser::Parser(string &input){
	str = input;
}

int Parser::read(string tpl, string tph){
	string inputString;
        char* inputArr;
        long inputLength=0;
        ifstream inputFile;
	int no_tph=0;

	inputFile.open(tpl.c_str());
	inputFile.seekg(0,ios::end);
	inputLength = inputFile.tellg();
	inputFile.seekg(0,ios::beg);
	inputArr = new char[inputLength+1];
	inputFile.read(inputArr,inputLength);
	inputArr[inputLength] = '\0';
	inputString = string(inputArr);
	inputFile.close();

	boost::smatch matches;
	boost::regex re;

	string::const_iterator begin,end;
	begin = inputString.begin();
	end = inputString.end();
	
	if(tph==""){
		no_tph=1;
		tph.assign("main");
	}

	re.assign("<tph:"+tph+">(.*)</tph:"+tph+">");

	if(boost::regex_search(begin,end,matches,re)){
		inputString = matches[1].str();
	} else if(no_tph==0){
		inputString = "";
	}

	setString(inputString);
	delete[] inputArr;
}

Tokens Parser::parse(){
	boost::smatch matches;
	boost::regex re;
	//re.assign("(?:\\n\\s*)?\\{\\.(!)?([^\\}\\s]*)(([^\\}\"\']([\"\'].*?[\"\'])?)*)\\}?",boost::regex_constants::icase);
	re.assign("(?:\\n\\s*)?\\{\\.(!)?([^\\}\\s]*)(.*?)(?<!\\\\)\\}",boost::regex_constants::icase);

	string leftovers = str;
	string::const_iterator begin,end;
	begin = str.begin();
	end = str.end();

	Tokens result;
	result.reserve(50);

	while(boost::regex_search(begin,end,matches,re)){
		string comm(matches[1].str());
		string func(matches[2].str());
		string parm(matches[3].str());
		string text(matches.prefix().str());

		if(text!=""){
			Token temp;
			temp.func = "ECHO";
			temp.param = text;
			result.push_back(temp);
		}
		if(comm==""){
			Token temp;
			temp.func = func;
			temp.param = parm;
			result.push_back(temp);
		}
		begin = matches.suffix().first;
		leftovers = matches.suffix().str();
	}

	if(leftovers!=""){
		Token temp;
		temp.func = "ECHO";
		temp.param = leftovers;
		result.push_back(temp);
	}

	return result;
}
