#ifndef _MAIL_H_
#define _MAIL_H_

#include "Base64.h"

#ifdef __LINUX__

#else
#include <WinSock2.h>
#pragma comment(lib,"ws2_32")
#endif


#define MAIN_SERVER "smtp.yeah.net"

#define ERR_CONNECT_REQ       0x001
#define ERR_CONNECT_RSP       0x002
#define ERR_HELO              0x003
#define SUCCESS_INIT_MAIL_BOX 0x004
#define ERR_INIT_MAIL_BOX     0x005
#define ERR_AUTH_LOGIN        0x006
#define ERR_USERNAME          0x007
#define ERR_USERPASSWD        0x008
#define SUCCESS_LOGIN         0x009
#define ERR_LOGIN             0x00A
#define ERR_MAIL_SENDER       0x00B
#define ERR_MAIL_RECEVER      0x00C
#define ERR_DATA_REQ          0x00D
#define ERR_SEND_MAIL         0x00E

#define SUCCESS_SEND_MAIL     0x010

#define ERR_SEND_CONTENT      0x011

#define ERR_QUIT              0x012
#define SUCCESS_QUIT          0x013

#define IN_POINTER_NULL       0x100


typedef struct USER_INFO_T
{
	char userName  [64];
	char userPasswd[64];
}USER_INFO_T;


typedef struct MAIL_INFO_T
{
	char from   [64];
	char to     [64];
	char subject[128];
	char content[2048 + 256];
}MAIL_INFO_T;

class GosMail
{
public:
	static GosMail * GetInstance();
	int SendMail(USER_INFO_T * pstUserInfo, MAIL_INFO_T * pstMailInfo);
	bool GetStatus();
	~GosMail();
private:
	GosMail();
	bool Init();
	void GetMailSocket(int iPort = 25);
	void FreeResource();
	int InitMailBox();
	int Login(USER_INFO_T * pstUserInfo);
	int SendContent(MAIL_INFO_T * pstMailInfo);
	int QuitEmailBox();
public:
private:
	SOCKET           m_socket;
	SOCKADDR_IN      m_sockAddr;
	bool             m_mailStatus;
	static GosMail * m_mailInst;
	int              m_iSockAddrLen;
	char             recvBuff[1024];
	char             sendBuff[1024 * 3];
};
#endif
