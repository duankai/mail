#include <string>
#include "mail.h"


void main()
{
	GosMail * gosMail = GosMail::GetInstance();
	if (!gosMail)
	{
		return;
	}
	if (!gosMail->GetStatus())
	{
		delete gosMail;
	}

	USER_INFO_T userInfo = {"username@yeah.net","password"};
	MAIL_INFO_T mailInfo = {"username@yeah.net",
							"destination@qq.com",
							"Demo",
							"This is a simple demo"};

	for(int i = 0; i< 10; i++)
	gosMail->SendMail(&userInfo, &mailInfo);
}
