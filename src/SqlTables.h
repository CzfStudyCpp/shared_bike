﻿#ifndef __SQLTABLES_H_
#define __SQLTABLES_H_
#include "sqlconnection.h"
#include <memory>
#include "glo_def.h"

class SqlTables
{
public:
	SqlTables(std::shared_ptr<MysqlConnection> sqlconn) :sqlconn_(sqlconn)
	{

	}
	//~SqlTables();
	/*表名	userinfo
字段名	类型	是否为空	默认值	主、外键	备注
id	int(16)	NOT	1,自增长	PK	用户id
username	varchar(128)	NOT			用户名：英文字符、数字和特殊符号的组合
mobile	varchar(16)	NOT	‘13000000000’		
registertm	timestamp	NOT	当前时间		注册时间
money	int(4)	NOT	0		账户余额
mobile_index(mobile)	index		索引
*/
	bool CreateUserInfo()
	{
		LOG_DEBUG("正在创建用户信息表......");
		const char* pUserInfoTable = "\
								CREATE TABLE IF NOT EXISTS userinfo( \
								id			 int(16)			NOT NULL primary key auto_increment, \
								mobile		 varchar(16)		NOT NULL default '13000000000', \	
								username	 varchar(18)	    NOT NULL default 'bikeuser', \
								userpassword varchar(16)    NOT NULL, \
								registertime timestamp		NOT NULL default CURRENT_TIMESTAMP, \
								lastUsetime  timestamp		NULL DEFAULT NULL,\
								money		 int(4)			NOT NULL default 10, \
								INDEX		 mobile_index(mobile) \
								)";
		if (!sqlconn_->Execute(pUserInfoTable))
		{
			LOG_ERROR("create table userinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			return false;
		}
		LOG_DEBUG("用户信息表创建成功！");

		//创建一个管理员账户
		char sql_content[1024];
		sprintf(sql_content, "SELECT username FROM userinfo where username='czf' and userpassword='czf'");
		SqlRecordSet record_set;
		;
		if (!sqlconn_->Execute(sql_content, record_set))
		{
			LOG_DEBUG("sql语句执行失败！错误原因[%s]", sqlconn_->GetErrInfo());
			return false;
		}

		MYSQL_RES* res = record_set.GetResult();
		MYSQL_ROW row;
		if (mysql_fetch_row(res) == NULL)
		{
			memset(sql_content, '\0', sizeof(sql_content));
			sprintf(sql_content, "INSERT INTO userinfo(username, userpassword) VALUES('czf', 'czf')");
			if (!sqlconn_->Execute(sql_content))
			{
				LOG_DEBUG("管理员信息插入失败！错误原因[%s]", sqlconn_->GetErrInfo());
				return false;
			}
		}

		return true;		
	}



	bool UserRideRecordInfo()
	{
		LOG_DEBUG("正在创建用户使用单车记录信息表......");
		const char* pUserRideRecordInfoTable = "\
								CREATE TABLE IF NOT EXISTS user_ride_record_info( \
								id				int				NOT NULL primary key auto_increment, \
								mobile			varchar(128)    NOT NULL default '1300000000', \
								username		varchar(18)		NOT NULL,\
								bikeId			int(11)			NOT NULL,\
								bikeStatus		tinyint(2)		NOT NULL,\
								start_time		timestamp		NOT NULL default CURRENT_TIMESTAMP, \
								end_time	    timestamp		NOT NULL default 0, \
								start_point		point			NOT NULL,			\
								end_point		point			NOT NULL,			\														
								comsume		    int				NOT NULL default 0, \
								balance		    int				NOT NULL default 0, \
								INDEX		mobile_index(mobile) \
								)";
		if (!sqlconn_->Execute(pUserRideRecordInfoTable))
		{
			LOG_ERROR("create table userinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			printf("create bikeinfo table  table failed. error msg:%s", sqlconn_->GetErrInfo());
			return false;
		}
		LOG_DEBUG("用户使用单车记录信息表创建成功！");
		return true;
	}
	/*表名	bikeinfo
		字段名	类型	是否为空	默认值	主、外键	备注
		id	int	NOT	自增长	PK	单车id
		devno	int	NOT			单车编号
		status	tinyint(1)	NOT	0		单车状态 
		0 - 停止
		1 - 骑行中
		2 - 损坏
		3 - 维修中
		trouble	int	NOT	0		损坏类型编号
		tmsg	varchar(256)	NOT	‘’		损坏原因描述
		latitude	double(10,6)	NOT	0		维度
		longitude	double(10,6)	NOT	0		经度
		unique(devno)	唯一约束
*/
	bool CreateBikeTable()
	{
		LOG_DEBUG("正在创建单车信息表......");
		const char* pBikeInfoTable = " \
						CREATE TABLE IF NOT EXISTS bikeinfo( \
						id			int				NOT NULL primary key auto_increment, \
						devno		int				NOT NULL, \
						status		tinyint(1)		NOT NULL default 2, \
						trouble		int				NOT NULL default 0, \
						tmsg		varchar(256)	NOT NULL default '', \
						time		timestamp		NOT NULL default CURRENT_TIMESTAMP, \
						bike_point	point			NOT NULL	\
						)";
		

		//const char* pBikeInfoTable = "create table if not exists bikeinfo(id int, name char(6));";
		if (!sqlconn_->Execute(pBikeInfoTable))
		{
			LOG_ERROR("create bikeinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			printf("create bikeinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			return false;
		}
		LOG_DEBUG("单车信息表创建成功！");
		return true;

	}

private:
	std::shared_ptr<MysqlConnection> sqlconn_;
};


#endif // !__SQLTABLES_H_
