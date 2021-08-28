#ifndef _H_REDIS_TOOLS_
#define _H_REDIS_TOOLS_
#include <iostream>
#include <vector>
#include <hiredis/hiredis.h>
#include "RedisConfig.h"

using namespace std;

class RedisTool
{
public:
    RedisTool();
    ~RedisTool();
    int setString(string key, string value);
    string getString(string key);

    void RemoveKey(string key);
    int AddOneItemToList(string key,string value);
    int setList(string key, vector<string> value);
    vector<string> getList(string key);
    void SetExpire(string key, int time);

    void SetHashValue(string key,string field , string value);
    string GetHashValue(string key,string field );
    int isHashKeyExists(string key );
private:
    void init();
    redisContext *m_redis;
    RedisConfig m_config;
};

#endif
