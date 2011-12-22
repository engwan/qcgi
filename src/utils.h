#ifndef UTILS_H
#define UTILS_H

#include<algorithm>
#include<sstream>
#include<string>

using namespace std;

string getWord(const string &str,int count);
string replace(const string &str,const char* c,const char n);
string toString(const int val);
string toLower(const string &str);
string toUpper(const string &str);

string escapeJS(const string &str);

void trace(const string &cookie_user,const string &title, const string &message);

#endif
