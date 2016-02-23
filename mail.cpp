#include "mail.h"
#include "string"


GosMail * GosMail::m_mailInst = NULL;
GosMail * GosMail::GetInstance()
{
	if (!m_mailInst)
	{
		m_mailInst = new GosMail();
	}
	return m_mailInst;
}

GosMail::GosMail()
{
	m_mailStatus = true;
	if (!Init())
	{
		m_mailStatus = false;
		return;
	}
}

bool GosMail::GetStatus()
{
	return m_mailStatus;
}

bool GosMail::Init()
{
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2,0);
	INT nErr = WSAStartup(wVersionRequested, &wsaData);
	if(nErr != 0)
	{
		return false;
	}

	//version
	if(LOBYTE(wsaData.wVersion) != 2 ||
	   HIBYTE(wsaData.wVersion) != 0)
	{
		WSACleanup( );
		return false;
	}
	return true;
}

void GosMail::GetMailSocket(int iPort)
{
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if (m_socket == INVALID_SOCKET)
	{
		FreeResource();
		return ;
	}

	hostent * pstHost = gethostbyname(MAIN_SERVER);
	if (!pstHost)
	{
		FreeResource();
		return;
	}
	memcpy(&m_sockAddr.sin_addr.S_un.S_addr, pstHost->h_addr_list[0], pstHost->h_length);
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_port = htons(iPort);

	m_iSockAddrLen = sizeof(SOCKADDR);
}

void GosMail::FreeResource()
{
	m_mailStatus = false;
	closesocket(m_socket);
}


GosMail::~GosMail()
{
	FreeResource();
	WSACleanup();
}

int GosMail::InitMailBox()
{
	if (connect(m_socket, (sockaddr *)&m_sockAddr, m_iSockAddrLen))
	{
		return ERR_CONNECT_REQ;
	}

	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);
	if (recvBuff[0] != '2' || recvBuff[1] != '2' || recvBuff[2] != '0')
	{
		return ERR_CONNECT_RSP;
	}

	memset(sendBuff, 0, sizeof(sendBuff));

	sprintf(sendBuff, "HELO %s\r\n", MAIN_SERVER);

	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0] != '2' || recvBuff[1] != '5' || recvBuff[2] != '0')
	{
		return ERR_CONNECT_RSP;
	}
	return SUCCESS_INIT_MAIL_BOX;

}

int GosMail::Login(USER_INFO_T * pstUserInfo)
{
	if (!pstUserInfo)
	{
		return IN_POINTER_NULL;
	}

	sendto(m_socket,"AUTH LOGIN\r\n",strlen("AUTH LOGIN\r\n"),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff), 0 ,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0] != '3' || recvBuff[1] != '3' || recvBuff[2] != '4')
	{
		return ERR_AUTH_LOGIN;
	}

	Base64 pcBase64 = Base64();

	unsigned char encodeBuf[128]={0};
	int encodeBufSize = 128;
	pcBase64.Encrypt((unsigned char *)pstUserInfo->userName, strlen(pstUserInfo->userName), encodeBuf, &encodeBufSize);

	memset(sendBuff, 0, sizeof(sendBuff));

	sprintf(sendBuff ,"%s\r\n", encodeBuf);
	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));

	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='3' || recvBuff[1]!='3' ||recvBuff[2]!='4')
	{
		return ERR_USERNAME;
	}

	memset(sendBuff, 0, sizeof(sendBuff));

	pcBase64.Encrypt((unsigned char *)pstUserInfo->userPasswd, strlen(pstUserInfo->userPasswd), encodeBuf, &encodeBufSize);
	sprintf(sendBuff ,"%s\r\n", encodeBuf);
	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));

	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='2' || recvBuff[1]!='3' ||recvBuff[2]!='5')
	{
		return ERR_USERPASSWD;
	}

	return SUCCESS_LOGIN;
}

int GosMail::SendContent(MAIL_INFO_T * pstMailInfo)
{
	memset(sendBuff, 0, sizeof(sendBuff));

	sprintf(sendBuff, "MAIL FROM:<%s>\r\n", pstMailInfo->from);

	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='2' || recvBuff[1]!='5' ||recvBuff[2]!='0')
	{
		return ERR_MAIL_SENDER;
	}

	memset(sendBuff, 0, sizeof(sendBuff));
	sprintf(sendBuff, "RCPT TO:<%s>\r\n", pstMailInfo->to);

	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='2' || recvBuff[1]!='5' ||recvBuff[2]!='0')
	{
		return ERR_MAIL_RECEVER;
	}

	sendto(m_socket,"DATA\r\n",strlen("DATA\r\n"),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='3' || recvBuff[1]!='5' ||recvBuff[2]!='4')
	{
		return ERR_DATA_REQ;
	}
	memset(sendBuff, 0, sizeof(sendBuff));
	sprintf(sendBuff, "from: \"%s\"\r\n" "to: \"%s\"\r\n" "subject: \"%s\"\r\n\r\n %s\r\n.\r\n", pstMailInfo->from, pstMailInfo->to, pstMailInfo->subject, pstMailInfo->content);

	sendto(m_socket,sendBuff,strlen(sendBuff),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);

	if (recvBuff[0]!='2' || recvBuff[1]!='5' ||recvBuff[2]!='0')
	{
		return ERR_SEND_MAIL;
	}
	return SUCCESS_SEND_MAIL;
}

int GosMail::QuitEmailBox()
{
	sendto(m_socket,"QUIT\r\n",strlen("QUIT\r\n"),0,(SOCKADDR*)&m_sockAddr,sizeof(SOCKADDR));
	recvfrom(m_socket, recvBuff, sizeof(recvBuff),0,(SOCKADDR*)&m_sockAddr,&m_iSockAddrLen);
	if (recvBuff[0]!='2' || recvBuff[1]!='2' ||recvBuff[2]!='1')
	{
		FreeResource();
		return ERR_QUIT;
	}
	return SUCCESS_QUIT;
}

int GosMail::SendMail(USER_INFO_T * pstUserInfo, MAIL_INFO_T * pstMailInfo)
{
	GetMailSocket();
	if (InitMailBox() != SUCCESS_INIT_MAIL_BOX)
	{
		return ERR_INIT_MAIL_BOX;
	}

	if (Login(pstUserInfo) != SUCCESS_LOGIN)
	{
		return ERR_LOGIN;
	}

	if (SendContent(pstMailInfo) != SUCCESS_SEND_MAIL)
	{
		return ERR_SEND_CONTENT;
	}

	return QuitEmailBox();
}
