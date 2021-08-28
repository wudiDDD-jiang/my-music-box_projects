#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>

using namespace std;


////注册
//#define  DEF_PACK_REGISTER_RQ    (DEF_PACK_BASE + 0)
//#define  DEF_PACK_REGISTER_RS    (DEF_PACK_BASE + 1)
////登录
//#define  DEF_PACK_LOGIN_RQ    (DEF_PACK_BASE + 2)
//#define  DEF_PACK_LOGIN_RS    (DEF_PACK_BASE + 3)

static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_REGISTER_RQ , &TcpKernel::RegisterRq},
    {DEF_PACK_LOGIN_RQ , &TcpKernel::LoginRq},
    {DEF_PACK_SET_USER_INFO , &TcpKernel::SetUserInfoRq},
    {DEF_PACK_ADD_FRIEND_RQ , &TcpKernel::AddFriendRq},
    {DEF_PACK_ADD_FRIEND_RS , &TcpKernel::AddFriendRs},
    {DEF_PACK_OFFLINRE_RQ , &TcpKernel::OfflineRq},
    {DEF_PACK_CHAT_RQ , &TcpKernel::ChatRq},
    {DEF_PACK_CREATEROOM_RQ , &TcpKernel::CreateRoomRq},
    {DEF_PACK_JOINROOM_RQ , &TcpKernel::JoinRoomRq},
    {DEF_PACK_LEAVEROOM_RQ , &TcpKernel::LeaveRoomRq},
    {DEF_PACK_VIDEO_FRAME , &TcpKernel::VideoFrame},
    {DEF_PACK_AUDIO_FRAME , &TcpKernel::AudioFrame},
    {0,0}
};
#define RootPath   "/home/jiang/MyMusicPlayerSrever/"

int TcpKernel::Open()
{
    InitRand();
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","jiang","MyMusicPlayer")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    return TRUE;
}

void TcpKernel::Close()
{
    m_sql->DisConnect();
    m_tcp->UnInitNetWork();

    for(auto ite=m_mapIdToUserInfo.begin();ite!=m_mapIdToUserInfo.end();++ite){
        delete ite->second;
    }
    m_mapIdToUserInfo.clear();
}


void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            PFUN fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 &&
                m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

//注册
void TcpKernel::RegisterRq(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);

    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;

    //根据用户名 查用户
    char sqlStr[1024]={0};
    sprintf(sqlStr,"select tel from t_user where tel = '%s';",rq->m_szUser);
    list<string> resList;
    if(!m_sql->SelectMysql(sqlStr,1,resList)){
        printf("SelectMysql error: %s \n",sqlStr);
        return;
    }
    if(resList.size()==0){
        //没有用户 可以注册 写表

        sprintf(sqlStr,"insert into t_user(tel,password,name,icon) values('%s','%s','%s','%d');",
                rq->m_szUser,rq->m_szPassword,rq->m_szUser,1);
        rs.m_lResult = register_success;
        if(!m_sql->UpdataMysql(sqlStr)){
            printf("UpdataMysql error: %s \n",sqlStr);
        }
    }
        else{
            //有用户 则注册失败
            rs.m_lResult=userid_is_exist;
        }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}

//登录
void TcpKernel::LoginRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;

    //查用户名 密码
    char sqlStr[1024]="";
    sprintf(sqlStr,"select password , id from t_user where tel = '%s';",rq->m_szUser);
    list<string> resList;
    if(!m_sql->SelectMysql(sqlStr,2,resList)){
        printf("SelectMysql error: %s \n",sqlStr);
        return;
    }
    if(resList.size()>0){
        //有用户 校验密码
        if(strcmp(resList.front().c_str(),rq->m_szPassword)==0){
            //密码一致
            rs.m_lResult = login_success;
            resList.pop_front();
            int id=atoi(resList.front().c_str());
            rs.m_UserID=id;

            m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

            //id---fd 映射 map 覆盖之后 先登录的强制下线
            UserInfo* user =NULL;
            if(m_mapIdToUserInfo.find(id)!=m_mapIdToUserInfo.end()){
                //之前有
                user = m_mapIdToUserInfo[id];
                if(user->m_fd!=clientfd){
                    //强制下线
                    STRU_FORCE_OFFLINE offline;
                    offline.m_UserID=id;
                    printf("clientfd:%d was closed\n clientfd:%d login\n", user->m_fd,clientfd);
                    m_tcp->SendData(user->m_fd,(char*)&offline,sizeof(offline));
                }
            }else{
                //之前没有
                user=new UserInfo;
                m_mapIdToUserInfo[id]=user;
            }
            user->m_fd=clientfd;
            user->m_id=id;
            user->m_state=1;

            //数据库获取用户信息
            GetUserInfoFromMysql(user);
            //自己的信息 好友列表
            UserGetUserList(user->m_id);
            return;
        }else{
            //密码不一致
            rs.m_lResult=password_error;
        }
    }else{
            //没有用户
            rs.m_lResult=userid_no_exist;
        }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}

//查表给user赋值
void TcpKernel::GetUserInfoFromMysql(UserInfo *user)
{
    if(!user) return;
    if(user->m_id==0) return;

    //查sql语句
    char sql[1024]="";
    sprintf(sql,"select name,icon from t_user where id = '%d'",user->m_id);
    list<string> res;
    if(!m_sql->SelectMysql(sql,2,res)){
        printf("SelectMysql error: %s \n",sql);
        return;
    }
    if(res.size()==2){
        strcpy(user->m_userName,res.front().c_str());
        res.pop_front();
        user->m_iconID=atoi(res.front().c_str());
        res.pop_front();
    }
}

//身份为id的用户获取自己的信息 获取所有好友的信息 在线的好友获取这个人的信息
void TcpKernel::UserGetUserList(int id)
{
    //没找到 这个人不在线
    if(m_mapIdToUserInfo.find(id)==m_mapIdToUserInfo.end()) return;

    UserInfo* loginer=m_mapIdToUserInfo[id];

    //获取自己的信息
    STRU_FRIEND_INFO loginrq;
    loginrq.m_state=1;
    loginrq.m_iconID=loginer->m_iconID;
    strcpy(loginrq.m_szName,loginer->m_userName);
    loginrq.m_userID=loginer->m_id;

    m_tcp->SendData(loginer->m_fd,(char*)&loginrq,sizeof(loginrq));

    //获取 所有的好友信息 在线的好友获取这个人的信息
    //idA idB

    //查mysql 获取好友id
    char sql[1024]="";
    //查登录人的所有好友id
    sprintf(sql,"select idA from t_friend where idB = '%d'",loginer->m_id);
    list<string> res;
    if(!m_sql->SelectMysql(sql,1,res)){
        printf("SelectMysql error: %s \n",sql);
        return;
    }

    //遍历链表 拿到好友每个人的id
    while(res.size()>0){
        int friendid=atoi(res.front().c_str());
        res.pop_front();
        //好友的信息 包的封装 获取好友的信息 （在线 userinfo 不在线mysql获取）
        STRU_FRIEND_INFO friendrq;
        friendrq.m_userID=friendid;
            //在不在线
            if(m_mapIdToUserInfo.find(friendid)!=m_mapIdToUserInfo.end()){
                //在线 //在线的好友 会发送该登录人的信息 给在线的好友
                friendrq.m_state=1;
                UserInfo* friender=m_mapIdToUserInfo[friendid];
                friendrq.m_iconID=friender->m_iconID;
                strcpy(friendrq.m_szName,friender->m_userName);

                //发送该登录人的信息 给在线的好友 --上线提示
                m_tcp->SendData(friender->m_fd,(char*)&loginrq,sizeof(loginrq));
            }else{
                //不在线
                friendrq.m_state=0;
                UserInfo friender;
                friender.m_id=friendid;
                //完成赋值
                GetUserInfoFromMysql(&friender);
                friendrq.m_iconID=friender.m_iconID;
                strcpy(friendrq.m_szName,friender.m_userName);
            }

        //所有的好友信息发送给登录的人
        m_tcp->SendData(loginer->m_fd,(char*)&friendrq,sizeof(friendrq));
    }
}

void TcpKernel::SetUserInfoRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d SetUserInfoRq\n", clientfd);
    //拆包
    STRU_SET_USER_INFO* rq=(STRU_SET_USER_INFO*)szbuf;
    //看map 取出来 更新
    if(m_mapIdToUserInfo.find(rq->m_UserID)==m_mapIdToUserInfo.end()) return;
    UserInfo* user=m_mapIdToUserInfo[rq->m_UserID];
    user->m_iconID=rq->m_iconid;
    strcpy(user->m_userName,rq->m_name);

    //更新mysql
    char sql[1024]="";
    sprintf(sql,"update t_user set name = '%s' , icon = %d where id = %d",
            user->m_userName,user->m_id);
    m_sql->UpdataMysql(sql);
    //获取好友列表函数 同步状态
    UserGetUserList(rq->m_UserID);
}

//添加好友结果
//#define no_this_user    0
//#define user_refused    1
//#define user_is_offline 2
//#define add_success     3
void TcpKernel::AddFriendRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d AddFriendRq\n", clientfd);
    //拆包
    STRU_ADD_FRIEND_RQ* rq=(STRU_ADD_FRIEND_RQ*)szbuf;
    //根据name查找id
    char sql[1024]="";
    sprintf(sql,"select id from t_user where name = '%s';",rq->m_szAddFriendName);
    list<string> res;
    if(!m_sql->SelectMysql(sql,1,res)){
        printf("SelectMysql error: %s \n",sql);
         return;
    }
    STRU_ADD_FRIEND_RS rs;
    rs.m_friendID=0;
    rs.m_userID=rq->m_userID;
    strcpy(rs.szAddFriendName,rq->m_szAddFriendName);

    //判断是否存在和在线
    if(res.size()>0){
        int id = atoi( res.front().c_str());
        //在线 转发 不在线 返回结果
        if(m_mapIdToUserInfo.find(id)!=m_mapIdToUserInfo.end()){
            UserInfo* user=m_mapIdToUserInfo[id];
            m_tcp->SendData(user->m_fd,szbuf,nlen);
            return;
        }else{
            rs.m_result=user_is_offline;
        }
    }else{
        //不存在 返回结果
        rs.m_result=no_this_user;
    }
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
}

void TcpKernel::AddFriendRs(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d AddFriendRs\n", clientfd);
    //拆包
    STRU_ADD_FRIEND_RS* rs=(STRU_ADD_FRIEND_RS*)szbuf;
    //如果同意
    if(rs->m_result==add_success){
        //写数据库 idA--idB  idB--idA
        char sql[1024]="";
        sprintf(sql,"insert into t_friend values( %d , %d );",rs->m_userID,rs->m_friendID);
        m_sql->UpdataMysql(sql);

        sprintf(sql,"insert into t_friend values( %d , %d );",rs->m_friendID,rs->m_userID);
        m_sql->UpdataMysql(sql);
        //获取好友列表
        UserGetUserList(rs->m_friendID);
    }
    //转发
    SendMsgToID(rs->m_userID,szbuf,nlen);
}

//转发
void TcpKernel::SendMsgToID(int id, char *szbuf, int nlen)
{
    printf("Server SendMsgToID:%d\n", id);
    if(m_mapIdToUserInfo.find(id)!=m_mapIdToUserInfo.end()){
        UserInfo* info=m_mapIdToUserInfo[id];
        m_tcp->SendData(info->m_fd,szbuf,nlen);
    }
}

//处理离线
void TcpKernel::OfflineRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d OfflineRq\n", clientfd);
    //拆包
    STRU_OFFLINE_RQ* rq=(STRU_OFFLINE_RQ*)szbuf;

    char sql[_DEF_SQLIEN]="";
    sprintf(sql,"select idB from t_friend where idA = '%d';",rq->m_userID);
    list<string> lst;
    m_sql->SelectMysql(sql,1,lst);

    //解除映射关系
    if(m_mapIdToUserInfo.find(rq->m_userID)!=m_mapIdToUserInfo.end()){
        m_mapIdToUserInfo.erase(rq->m_userID);
    }

    //在房间里退出房间

    //给所有好友发送离线
    while(lst.size()>0){
        int id=atoi(lst.front().c_str());
        lst.pop_front();
        STRU_OFFLINE_RS rs;
        rs.m_userID=rq->m_userID;

        SendMsgToID(id,(char*)&rs,sizeof(rs));
    }

}

//聊天请求处理
void TcpKernel::ChatRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d ChatRq\n", clientfd);

    STRU_CHAT_RQ* rq=(STRU_CHAT_RQ*)szbuf;

    if(m_mapIdToUserInfo.find(rq->m_friendID)!=m_mapIdToUserInfo.end()){
        SendMsgToID(rq->m_friendID,szbuf,nlen);
    }else{
        STRU_CHAT_RS rs;
        rs.m_result=user_is_offline;
        rs.m_userID=rq->m_userID;
        rs.m_friendID=rq->m_friendID;

        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
    }
}

//创建房间请求
void TcpKernel::CreateRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d CreateRoomRq\n", clientfd);
    //拆包
    STRU_CREATEROOM_RQ* rq=(STRU_CREATEROOM_RQ*)szbuf;
    if(m_mapIdToUserInfo.find(rq->m_UserID)==m_mapIdToUserInfo.end()) return;
    //map 看看有没有对应 user
    UserInfo* info=m_mapIdToUserInfo[rq->m_UserID];
    //随机roomid 判定是否重复
    int roomid;
    do{
        roomid=rand()%100000000;
    }while(roomid==0||m_mapRoomIdToUserList.find(roomid)!=m_mapRoomIdToUserList.end());
    info->m_roomid=roomid;
    //list 添加用户 加入到map中
    list<UserInfo*> lst;
    lst.push_back(info);
    m_mapRoomIdToUserList[roomid]=lst;
    //回复信息
    STRU_CREATEROOM_RS rs;
    rs.m_RoomId=roomid;
    rs.m_lResult=1;

    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
}

//加入房间请求
void TcpKernel::JoinRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d JoinRoomRq\n", clientfd);
    //拆包
     STRU_JOINROOM_RQ* rq=(STRU_JOINROOM_RQ*)szbuf;
     STRU_JOINROOM_RS rs;
     //查看map中有没有 没有返回
     if(m_mapIdToUserInfo.find(rq->m_UserID)==m_mapIdToUserInfo.end()){
         rs.m_lResult=0;
         rs.m_RoomID=rq->m_RoomID;
         m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
         return;
     }
     UserInfo* joiner=m_mapIdToUserInfo[rq->m_UserID];
    //查看房间是否存在  有 没有 返回
     if(m_mapRoomIdToUserList.find(rq->m_RoomID)==m_mapRoomIdToUserList.end()){
         rs.m_lResult=0;
         rs.m_RoomID=rq->m_RoomID;
         m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
         return;
     }
     joiner->m_roomid=rq->m_RoomID;
     list<UserInfo*> lst=m_mapRoomIdToUserList[rq->m_RoomID];   //假定 map list 节点都是有效的
     rs.m_RoomID=rq->m_RoomID;
     rs.m_lResult=1;
     m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

    //有这个房间 写加入人的房间成员信息 获取房间成员列表
     STRU_ROOM_MEMBER_RQ joinrq;
     joinrq.m_UserID=rq->m_UserID;
     strcpy(joinrq.m_szUser,joiner->m_userName);

    //遍历列表 -- 写房间内所有人的成员信息
     for (auto ite = lst.begin(); ite != lst.end(); ++ite) {
         UserInfo* inner=*ite;
         STRU_ROOM_MEMBER_RQ innerrq;
         innerrq.m_UserID=inner->m_id;
         strcpy(innerrq.m_szUser,inner->m_userName);
         //加入人的信息发送给房间内所有人 房间内所有人的信息 发送给加入人
         m_tcp->SendData(inner->m_fd,(char*)&joinrq,sizeof(joinrq));
         m_tcp->SendData(joiner->m_fd,(char*)&innerrq,sizeof(innerrq));
     }

    //加入人的信息添加到list 更新map
     lst.push_back(joiner);
     m_mapRoomIdToUserList[rq->m_RoomID]=lst;

}

//处理离开房间请求
void TcpKernel::LeaveRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d LeaveRoomRq\n", clientfd);
    //拆包
    STRU_LEAVEROOM_RQ* rq=(STRU_LEAVEROOM_RQ*)szbuf;
    //看看map中是否有这个人 没有 不处理 有 记录信息
    if(m_mapIdToUserInfo.find(rq->m_nUserId)==m_mapIdToUserInfo.end()) return;
    UserInfo* leaver=m_mapIdToUserInfo[rq->m_nUserId];

    //看看map中有没有这个房间 没有 不处理 有 取到链表
    if(m_mapRoomIdToUserList.find(rq->m_RoomId)==m_mapRoomIdToUserList.end()) return;
    list<UserInfo*> lst=m_mapRoomIdToUserList[rq->m_RoomId];

    STRU_LEAVEROOM_RS rs;
    rs.m_UserID=leaver->m_id;
    strcpy(rs.szUserName,leaver->m_userName);
    //遍历链表 判断是否是此人 如果不是发送此人离开信息 如果是 删除该节点
    for(auto ite=lst.begin();ite!=lst.end();){
        UserInfo* inner = *ite;
        if(inner->m_id!=leaver->m_id){
            m_tcp->SendData(inner->m_fd,(char*)&rs,sizeof(rs));
            ++ite;
        }else{
            ite=lst.erase(ite);
        }
    }
    //更新map
    m_mapRoomIdToUserList[rq->m_RoomId]=lst;
    //如果离开房间后 成员数量为0 则从map中去除
    if(lst.size()==0){
        m_mapRoomIdToUserList.erase(rq->m_RoomId);
    }
}

//处理视频帧
void TcpKernel::VideoFrame(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d VideoFrame\n", clientfd);
    ///视频数据帧
    /// 成员描述
    /// int type;
    /// int userId;
    /// int roomId;
    /// QByteArray videoFrame;

    //拆包
    char *tmp=szbuf;
    int type=* (int*)tmp;
    tmp+=sizeof(int);
    int userId=* (int*)tmp;
    tmp+=sizeof(int);
    int roomId=* (int*)tmp;
    tmp+=sizeof(int);
    //找到map里面房间 转发给其他人
    if(m_mapRoomIdToUserList.find(roomId)==m_mapRoomIdToUserList.end()) return;
    list<UserInfo*> lst=m_mapRoomIdToUserList[roomId];
    for(auto ite=lst.begin();ite!=lst.end();++ite){
        UserInfo* user=*ite;
        if(user->m_id!=userId){
            SendMsgToID(user->m_id,szbuf,nlen);
        }
    }
}

//处理音频帧
void TcpKernel::AudioFrame(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d AudioFrame\n", clientfd);
    ///音频数据帧
    /// 成员描述
    /// int type;
    /// int userId;
    /// int roomId;
    /// QByteArray audioFrame;

    //拆包
    char *tmp=szbuf;
    int type=* (int*)szbuf;
    tmp+=sizeof(int);
    int userId=* (int*)szbuf;
    tmp+=sizeof(int);
    int roomId=* (int*)szbuf;
    tmp+=sizeof(int);
    //找到map里面房间 转发给其他人
    if(m_mapRoomIdToUserList.find(roomId)==m_mapRoomIdToUserList.end()) return;
    list<UserInfo*> lst=m_mapRoomIdToUserList[roomId];
    for(auto ite=lst.begin();ite!=lst.end();++ite){
        UserInfo* user=*ite;
        if(user->m_id!=userId){
             SendMsgToID(user->m_id,szbuf,nlen);
        }
    }

}

#include <sys/time.h>
//随机数初始化
void TcpKernel::InitRand()
{
    struct timeval time;
    gettimeofday(&time,NULL);
    srand(time.tv_sec+time.tv_usec);
}



