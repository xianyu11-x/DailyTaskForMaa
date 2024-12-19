#include <mutex>
#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include "sqlConnectPool.h"
#include <co_async/co_async.hpp>

using namespace std;

connectionPool::connectionPool()
{
	mCurConn = 0;
	mFreeConn = 0;
}

connectionPool *connectionPool::GetInstance()
{
	static connectionPool connPool;
	return &connPool;
}

//构造初始化
void connectionPool::init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn)
{
	murl = url;
	mPort = Port;
	mUser = User;
	mPassWord = PassWord;
	mDatabaseName = DataBaseName;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DataBaseName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			exit(1);
		}
		connList.push_back(con);
		++mFreeConn;
	}

	reserve  = new co_async::Semaphore(mFreeConn, mFreeConn);
	mMaxConn = mFreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
co_async::Task<MYSQL*> connectionPool::GetConnection()
{
	MYSQL *con = NULL;

	if (0 == connList.size())
		co_return NULL;

	co_await reserve->acquire();
	{
		std::lock_guard<decltype(sqlLock)> lock(sqlLock);
		con = connList.front();
		connList.pop_front();
		--mFreeConn;
		++mCurConn;
	}
	co_return con;
}

//释放当前使用的连接
co_async::Task<bool> connectionPool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		co_return false;

	{
		std::lock_guard<decltype(sqlLock)> lock(sqlLock);
		connList.push_back(con);
		++mFreeConn;
		--mCurConn;
	}
	co_await reserve->release();
	co_return true;
}

//销毁数据库连接池
void connectionPool::DestroyPool()
{

	std::lock_guard<decltype(sqlLock)> lock(sqlLock);
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		mCurConn = 0;
		mFreeConn = 0;
		connList.clear();
	}
}

//当前空闲的连接数
int connectionPool::GetFreeConn()
{
	return this->mFreeConn;
}

connectionPool::~connectionPool()
{
	DestroyPool();
}
