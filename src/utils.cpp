#include <fstream>
#include <iostream>
#include <time.h>
#include "utils.h"

string replace(const string &str,const char* c,const char n){
	string res = str;
	size_t found;

	found = res.find_first_of(c);
	while(found!=string::npos){
		res[found]=n;
		found = res.find_first_of(c,found+1);
	}

	return res;
}

string getWord(const string &str, int count){
	string res = "";
	string delimiters = " \n\t";
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	int i;
	for(i=0; string::npos!=pos || string::npos!=lastPos; i++){
		if(i==count){
			res.assign(str.substr(lastPos,pos-lastPos));
			break;
		}
		lastPos = str.find_first_not_of(delimiters,pos);
		pos = str.find_first_of(delimiters,lastPos);
	}

	return res;
}

string toString(const int val){
	stringstream str;
	str << val;
	return str.str();
}

string toLower(const string &str){
	string res  = str;
	transform(res.begin(),res.end(),res.begin(),(int(*)(int)) tolower);
	return res;
}

string toUpper(const string &str){
	string res  = str;
	transform(res.begin(),res.end(),res.begin(),(int(*)(int)) toupper);
	return res;
}

string escapeJS(const string &str){
	string res = str;
        string delimiters = "\\\"\'\n\t\r\f";
	string replacement;
        string::size_type pos = res.find_first_of(delimiters,0);

        int i;
        for(i=0; string::npos!=pos; i++){
		replacement="";
		if(res[pos]=='\n'){
			replacement = "\\n";
		} else if(res[pos]=='\t'){
			replacement = "\\t";
		} else if(res[pos]=='\r'){
			replacement = "\\r";
		} else if(res[pos]=='\f'){
			replacement = "\\f";
		} else if(res[pos]=='\''){
			replacement = "\\'";
		} else if(res[pos]=='\\'){
			replacement = "\\\\";
		} else if(res[pos]=='\"'){
			replacement = "\\\"";
		}
		res = res.replace(pos,1,replacement);
                pos = res.find_first_of(delimiters,pos+2);
        }

        return res;
}

void trace(const string &cookie_user,const string &title, const string &message){
	int debug = 1;
	if(debug){
		ofstream log;
		time_t now;
		struct tm* timeinfo;
		string filename;
		char buffer[30];

		time(&now);
		timeinfo = localtime(&now);

		if(cookie_user!=""){
			filename = "/tmp/"+cookie_user+"."+title+".log";
		} else {
			filename = "/tmp/0."+title+".log";
		}

		strftime(buffer,30,"[%d %b %Y %H:%M:%S] ",timeinfo);

		log.open(filename.c_str(),ios_base::app);
		log << buffer << message << endl;
	}
}
