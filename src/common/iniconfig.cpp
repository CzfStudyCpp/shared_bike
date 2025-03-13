#include<iniparser/iniparser.h>
#include "iniconfig.h"


Iniconfig::Iniconfig():_isLoaded(false){
	
	
}

Iniconfig::~Iniconfig(){
	
}

bool Iniconfig::loadFile(const std::string &path){
	 dictionary *ini=NULL;
	 ini =iniparser_load(path.c_str());
	 if(ini==NULL){
		 fprintf(stderr,"cannot parser the file%s!\n",path.c_str());
		 return false;
	 }
	const char* ip    = iniparser_getstring(ini, "database:ip", "127.0.0.1");
    const int   port  = iniparser_getint(ini, "database:port", 3306);
    const char* user  = iniparser_getstring(ini, "database:user", "root");
    const char* pwd   = iniparser_getstring(ini, "database:pwd", "123456");
    const char* db    = iniparser_getstring(ini, "database:db", "shared_bike");
    const int   sport = iniparser_getint(ini, "server:port", 9090);
    
	
	_config=st_env_config(std::string(ip),port,std::string(user),std::string(pwd),std::string(db),sport);
	
	iniparser_freedict(ini);
	_isLoaded=true;
	
	return true;
 }