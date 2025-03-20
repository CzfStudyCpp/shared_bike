#ifndef __SQLCONNECTION_H_
#define __SQLCONNECTION_H_

#include <mysql/mysql.h>
#include <string.h>
#include <mysql/errmsg.h>
#include <assert.h>

#include "glo_def.h"

class SqlRecordSet
{
public:
	SqlRecordSet() : m_pRes(NULL)
	{
		
	}

	explicit SqlRecordSet(MYSQL_RES* pRes)
	{
	}
	~SqlRecordSet()
	{
		if (m_pRes)
		{
			mysql_free_result(m_pRes);
		}
	}

	MYSQL_RES* MysqlRes()
	{
		return m_pRes;
	}

	inline void SetResult(MYSQL_RES* pRes)
	{
		assert(m_pRes == NULL);
		if (m_pRes)
		{
			LOG_WARN("the MYSQL_RES has already stored result. may will cause memory leak!");
		}
		m_pRes = pRes;
	}

	inline MYSQL_RES* GetResult()
	{
		return m_pRes;
	}

	MYSQL_ROW FetchRow()
	{
		return mysql_fetch_row(m_pRes);
	}

	inline i32 GetRowCount()
	{
		return m_pRes->row_count;
	}
	

private:
	MYSQL_RES* m_pRes;//MYSQL结果集
};


//MYSQL连接
class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();

	MYSQL* Mysql()
	{
		return mysql_;
	}

	bool Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);

	bool Execute(const char* szSql);//执行SQL语句

	bool Execute(const char* szSql, SqlRecordSet& recordSet);//执行sql语句而且返回SQL结果

	int EscapeString(const char* pSrc, int nSrclen, char* pDest);

	void close();

	const char* GetErrInfo();

	void Reconnect();


private:

	MYSQL* mysql_;
};

#endif // !__SQLCONNECTION_H_
