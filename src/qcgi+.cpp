#include <rude/cgi.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "fcgi_stdio.h"
#include "db.h"
#include "parser.h"
#include "interpreter.h"
#include "sessions.h"

using namespace std;

double diffclock(clock_t,clock_t);
string retrieveEnv(const char*);
void usage(void);
void error();

int main(int argc, char* argv[]){
	DB qdb("localhost","root","test");
	Parser qp;
	Interpreter qi(&qdb,&qp);
	Sessions ss(&qdb);


	while(FCGI_Accept()>=0){

		time_t start,stop;
		time(&start);

		rude::CGI cgi;

		int i;
		qdb.clearVars();
		for(i=0; i<cgi.numValues(); i++){
			//printf("%s : %s\n",cgi[i],cgi[cgi[i]]);
			qdb.setVar(cgi[i],cgi[cgi[i]]);
		}

		if(!qp.read(retrieveEnv("SCRIPT_FILENAME"),qdb.getVar("tph"))){
			error();
		} else {

			qdb.setVar("content-type","text/html");
			qdb.setVar("DOCUMENT_ROOT",retrieveEnv("DOCUMENT_ROOT"));
			qdb.setVar("REQUEST_METHOD",retrieveEnv("REQUEST_METHOD"));
			qdb.setVar("REMOTE_ADDR",retrieveEnv("REMOTE_ADDR"));
			qdb.setVar("QCGI_MAC_IP",retrieveEnv("REMOTE_ADDR"));
			qdb.setVar("tpl",retrieveEnv("SCRIPT_NAME"));
			qdb.setVar("tph",retrieveEnv("PATH_INFO"));

			if(!ss.init()){
				if(!qp.read(qdb.getVar("DOCUMENT_ROOT")+"/qcgi/login.v","")){
					error();
				}
			}

			printf("HTTP/1.0 200 OK\n");

			if(ss.set()){
				printf("Set-Cookie: QCGI_SESSID=%s; path=/;\n",qdb.getVar("QCGI_SESSID").c_str());
			}

			if(ss.login()){
				printf("Location: %s\n\n",qdb.getVar("tpl").c_str());
				qi.clear();
			} else {
				qi.setTokens(qp.parse());
				qi.exec();

				time(&stop);
				printf("Content-type: %s\n",qdb.getVar("content-type").c_str());
				printf("Qcgi-Exec: %f s\n",difftime(stop,start));
				printf("\n");
				qi.flush();
			}
		}

		cgi.finish();
	}

	/*if(argc==1){
		usage();
		return 1;
	}

	int i;
	for(i=1; i<argc; i++){
		inputFile.open(argv[i]);
		if(!inputFile.is_open()){
			cout << "Unable to open file.\n";
			return 1;
		}

		inputFile.seekg(0,ios::end);
		inputLength = inputFile.tellg();
		inputFile.seekg(0,ios::beg);
		inputArr = new char[inputLength+1];
		inputFile.read(inputArr,inputLength);
		inputArr[inputLength] = '\0';
		inputString = string(inputArr);
		inputFile.close();
	}*/
	
	return 0;
}

string retrieveEnv(const char* var){
	const char* env = getenv(var);
	if(!env){
		return "";
	} else {
		return string(env);
	}
}

double diffclock(clock_t clock1,clock_t clock2){
	double diffticks = clock1-clock2;
	double diffms = (diffticks)/(CLOCKS_PER_SEC/1000);
	return diffms;
}

void error(){
	printf("Content-type: text/html\n");
	printf("HTTP/1.0 404 Not Found\n\n");
	printf("<h1>Could not read requested file.</h1>\n");
}

void usage(){
	cout << "Usage: qcgi+ <filename>\n";
}
