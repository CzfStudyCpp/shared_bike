#ifndef SHBK_COMMON_INICONFIG_h_
#define SHBK_COMMON_INICONFIG_h_
#include<string>
#include"configdef.h"

class Iniconfig{
public:

  Iniconfig();
  ~Iniconfig(); 
  bool loadFile(const std::string &path);
  const st_env_config &getConfig();  

private:

  st_env_config _config;
  bool _isLoaded;
};

#endif