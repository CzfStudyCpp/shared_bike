#include "user_service.h"

UserService::UserService(std::shared_ptr<MysqlConnection> sql_conn):sql_conn_(sql_conn)
{


}

bool UserService::exist(std::string & userName)
{
	char sql_content[1024] = { 0 };

	sprintf(sql_content, "select * from userinfo where username = %s", \
		userName.c_str());

	SqlRecordSet record_set;
	if (!sql_conn_->Execute(sql_content, record_set))
	{
		return false;
	}

	return (record_set.GetRowCount() != 0);
	//return true;
}

bool UserService::verifyUserInfo(std::string& userName, std::string& userPwd)
{
	char sql_content[1024] = { 0 };

	sprintf(sql_content, "select * from userinfo where username = '%s' and userpassword='%s'", \
		userName.c_str(), userPwd.c_str());

	SqlRecordSet record_set;
	if (!sql_conn_->Execute(sql_content, record_set))
	{
		return false;
	}

	return (record_set.GetRowCount() != 0);
}

bool UserService::insert(std::string & userName, std::string& userPwd)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "insert into userinfo(username, userpassword)  \
							values(\"%s\", \"%s\")", userName.c_str(), userPwd.c_str());

	LOG_DEBUG("executing sql insert into userinfo(username, userpassword)....");
	LOG_DEBUG("sql: [%s]", sql_content);
	//if (!sql_conn_->Execute(sql_content, record_set))
	if (!sql_conn_->Execute(sql_content))
	{
		LOG_ERROR("执行数据库插入语句失败！\n");
		return false;
	}
	LOG_DEBUG("执行sql: [%s]成功！\n", sql_content);

	//return (record_set.GetRowCount() != 0);
	return true;
}

bool UserService::addBike(i32 bikeid, r64 longitude, r64 latitude)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "INSERT INTO bikeinfo(devno, bike_point) VALUES(%d, GeomFromText('POINT(%lf %lf)'))", bikeid, longitude, latitude);
	return sql_conn_->Execute(sql_content);
}

bool UserService::deleteBike(i32 bikeid)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "DELETE FROM  bikeinfo WHERE devno=%d", bikeid);
	return sql_conn_->Execute(sql_content);
}

bool UserService::updataBikeinfo(char* sql_content)
{
	return sql_conn_->Execute(sql_content);
}

bool UserService::insertUseRideRecord(char* sql_content)
{
	return sql_conn_->Execute(sql_content);
}

bool UserService::userRecordModify(char* sql_content)
{
	return sql_conn_->Execute(sql_content);
}

bool UserService::findBikeInfo(char* sql_content, SqlRecordSet &record_set)
{
	return sql_conn_->Execute(sql_content,record_set);
}

bool UserService::findUserInfo(char* sql_content, SqlRecordSet& record_set)
{
	return sql_conn_->Execute(sql_content, record_set);
}
