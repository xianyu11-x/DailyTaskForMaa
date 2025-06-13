#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include "co_async/generic/semaphore.hpp"
#include <mutex>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string>
#include <co_async/co_async.hpp>
#pragma once
using namespace std;

class connectionPool
{
public:
	co_async::Task<MYSQL*> GetConnection();				 //获取数据库连接
	co_async::Task<bool> ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接

	//单例模式
	static connectionPool *GetInstance();

	void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn); 

private:
	connectionPool();
	~connectionPool();

	bool checkSqlState(MYSQL *con);

	int mMaxConn;  //最大连接数
	int mCurConn;  //当前已使用的连接数
	int mFreeConn; //当前空闲的连接数
	//locker lock;
	mutex sqlLock;
	list<MYSQL *> connList; //连接池
	//sem reserve;
	co_async::Semaphore* reserve;

public:
	string murl;			 //主机地址
	int mPort;		 //数据库端口号
	string mUser;		 //登陆数据库用户名
	string mPassWord;	 //登陆数据库密码
	string mDatabaseName; //使用数据库名
};



#endif