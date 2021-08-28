#include "RedisConfig.h"
#include <sstream>

RedisConfig::RedisConfig()
{

}

std::string RedisConfig::getRedisIP()
{
    return "127.0.0.1";//设置为本机ip
}

int RedisConfig::getRedisPort()
{
    return 6379;
}
