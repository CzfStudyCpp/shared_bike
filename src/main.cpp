#include "iniconfig.h"
#include"configdef.h"
#include "Logger.h"
int main(int argc,char**argv){
	
  if(argc!=3){
    fprintf(stderr,"Please input shared_bike <config file path><log file config>!\n");
	return -1;

  }
  if(!Logger::instance()->init(std::string(argv[2]))){
	  fprintf(stderr,"init log module failed.\n");
	  return -2;
  }
  Iniconfig config;
  if(!config.loadFile(std::string(argv[1]))){
	  LOG_ERROR("load %s failed.\n",argv[1]);
	  Logger::instance()->GetHandle()->error("load %s failed.",argv[1]);
	  return -3;
  }
  
  st_env_config conf_args=config.getConfig();
  LOG_INFO("[database]ip: %s,port :%d,user:%s,pwd:%s,db_name:%s [server] port:%d \n",conf_args.db_ip.c_str(),conf_args.db_port,\
  conf_args.db_user.c_str(),conf_args.db_pwd.c_str(),conf_args.db_name.c_str(),conf_args.svr_port);
  return 0;
	
}
