#ifndef __USER_SERVICE_H_
#define __USER_SERVICE_H_

#include <memory>
#include "sqlconnection.h"

class UserService
{
public:
	UserService(std::shared_ptr<MysqlConnection> sql_conn);

	bool exist(std::string& userName);
	bool verifyUserInfo(std::string& userName, std::string& userPwd);
	bool insert(std::string& userName, std::string& userPwd);
	bool addBike(i32 bikeid, r64 longitude, r64 latitude);
	bool deleteBike(i32 bikeid);
	bool updataBikeinfo(char* sql_content);
	bool insertUseRideRecord(char* sql_content);
	bool userRecordModify(char* sql_content);
	bool findBikeInfo(char* sql_content, SqlRecordSet &record_set);
	bool findUserInfo(char* sql_content, SqlRecordSet& record_set);
private:
	std::shared_ptr<MysqlConnection> sql_conn_;

};

#endif // !__USER_SERVICE_H_




