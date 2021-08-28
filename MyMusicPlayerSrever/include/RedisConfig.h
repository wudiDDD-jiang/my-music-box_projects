#ifndef REDISCONFIG_H
#define REDISCONFIG_H
#include <string>

class RedisConfig
{
public:
    RedisConfig();
    std::string getRedisIP();//获取ip
    int getRedisPort();//获取端口号

};

#endif
