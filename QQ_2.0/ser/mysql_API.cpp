#include"ser.h"

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//MYSQL *mysql = NULL;//mysql_init((MYSQL*)0);
//MYSQL_RES *res;
//MYSQL_ROW row;

/*
void mysql_connect()
{
	if (!mysql_real_connect(mysql, "localhost", "root", NULL, "chat", 0, NULL, 0))
	{
		cerr << "sql connect fail;errno:" << errno << endl;
		return;
	}
	cout << "connect success!" << endl;
}

bool mysql_insert(const char* table, const char *name, const char *pw)
{
	char buff[64] = { 0 };
	int flag;
	mysql_connect();

	sprintf(buff, "INSERT INTO %s VALUE('%s','%s')", table, name, pw);
	cout << buff << endl;
	flag = mysql_real_query(mysql, buff, (unsigned int)strlen(buff));
	if (flag)
	{
		cout << "Insert data failed!" << endl;
		return false;
	}

	cout << "Insert data success!" << endl;
	return true;
}

bool mysql_search(const char* table, const char* name, const char *pw)
{
	char buff[128] = { 0 };
	int flag;
	mysql_connect();
	sprintf(buff, "select * from %s where name='%s' and pw='%s'", table, name, pw);
	cout << "buff= " << buff << endl;
	flag = mysql_real_query(mysql, buff, (unsigned int)strlen(buff));
	if (flag)
	{
		cout << "query failed" << endl;
	}
	else
	{
		cout << "success" << endl;
		MYSQL_RES *res = mysql_store_result(mysql);
		MYSQL_ROW row = mysql_fetch_row(res);

		if (row == 0)
		{
			return false;
		}

	}
	return true;
}

bool mysql_delete(const char* table, const char* s1, const char* s2)
{
	char buff[64] = { 0 };
	int flag;
	mysql_connect();
	sprintf(buff, "delete from %s where %s = '%s'", table, s1, s2);
	cout << buff << endl;
	flag = mysql_real_query(mysql, buff, (unsigned int)strlen(buff));
	if (flag)
	{
		cout << "Delete data failed!" << endl;
		return false;
	}
	cout << "Delete data success!" << endl;
	return true;
}

*/



//数据库构造
Mysql::Mysql(const char* username, const char* pw)
{
    cout<<"Mysql()"<<endl;
  
    mysql = mysql_init(&mm);
    cout<<"&mm : "<<&mm<<endl;
    
    cout<<"mysql : "<<mysql<<endl;
    mysql_connect();
}

void Mysql::mysql_connect()
{
    cout<<"connect mysql : "<<mysql<<endl;

    if (!mysql_real_connect(mysql,"localhost", "root", "111111","chat", 0, NULL, 0))
	{
		cerr << "sql connect fail;errno:" << errno << endl;
		return;
	}
	cout << "connect success!" << endl;
}

//析构
Mysql::~Mysql()
{
    cout<<"mysql ~~~~~ ovw"<<endl;
}

//插入
bool Mysql::insert(const char* table,const char *name,const char *pw)
{
   // pthread_mutex_lock(&mutex);
    
    cout<<"insert"<<endl;
    cout<<"mysql : "<<mysql<<endl;
    
    char buff[64] = {0};
    int flag;
    
    //mysql_connect();   
    
    sprintf(buff,"INSERT INTO %s VALUE('%s','%s')",table,name,pw);
    cout<<buff<<endl;
    flag = mysql_real_query(mysql,buff,(unsigned int)strlen(buff));
    if(flag)
    {
        cout<<"Insert data failed!"<<endl;
     //   pthread_mutex_unlock(&mutex);
        return false;
    }    
    cout<<"Insert data success!"<<endl;
   // pthread_mutex_unlock(&mutex);
    return true;
}

//查询
bool Mysql::search(const char* table,const char* name,const char *pw)
{
   // pthread_mutex_lock(&mutex);
    cout<<"search"<<endl;
    cout<<"mysql : "<<mysql<<endl;

    char buff[128] = {0};
    int flag = 0;
    
    //mysql_connect();
    //const char *sql = "select * from user where name='zjt' and pw='111';";
    sprintf(buff,"select * from %s where name='%s' and pw='%s';",table,name,pw);
    
    cout<<"buff= "<<buff<<endl;
    
    flag = mysql_real_query(mysql,buff,(unsigned int)strlen(buff));
    //flag = mysql_query(mysql,sql);
    //cout<<mysql_error(mysql)<<endl;
    
    cout<<"flag : "<<flag<<endl;
    

    if(flag)
    {
        cout<<"query failed"<<endl; 
        return false;
    }
    else
    {
        cout<<"success"<<endl;   
        res = mysql_store_result(mysql);
        row = mysql_fetch_row(res);

        if(row == 0)
        {
     //       pthread_mutex_unlock(&mutex);
            return false;    
        }
    }
   // pthread_mutex_unlock(&mutex);
    return true;   
}

//删除
bool Mysql::mydelete(const char* table,const char* s1,const char* s2)
{
    //pthread_mutex_lock(&mutex);
    cout<<"mydelet"<<endl;       
    cout<<"mysql : "<<mysql<<endl;
    
    char buff[64] = {0};
    int flag;
    
    //mysql_connect();
    
    sprintf(buff,"delete from %s where %s = '%s'",table,s1,s2);
    cout<<buff<<endl;
    flag = mysql_real_query(mysql,buff,(unsigned int)strlen(buff));
    if(flag)
    {
        cout<<"Delete data failed!"<<endl;
      //  pthread_mutex_unlock(&mutex);
        return false;
    }
    cout<<"Delete data success!"<<endl;
    //pthread_mutex_unlock(&mutex);
    return true;
}

