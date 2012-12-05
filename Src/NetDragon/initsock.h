//////////////////////////////////////////////////////////
// initsock.h�ļ�

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

class CInitSock		
{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 0)
	{
		// ��ʼ��WS2_32.dll
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer, majorVer);
		if(::WSAStartup(sockVersion, &wsaData) != 0)
		{
			exit(0);
		}
	}
	~CInitSock()
	{	
		::WSACleanup();	
	}
};
