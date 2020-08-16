#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<thread>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

//�ַ�������ĺ���
int myStrLen(char* str)
{
	int i = 0;
	while (*str != '\0')
	{
		i++;
		str++;
	}
	return i;
}
char* myStrcat(char *str1, char* str2)
{
	char *temp = str1;
	while (*temp != '\0')temp++;
	while ((*temp++=*str2++) != '\0');
	return str1;
}
char* myStrcp(char *str1, char *str2)
{
	char *temp = str1;
	int i = 0;
	while ((*temp++ = *str2++) != '\0');
	return str1;

}
char * myStrSplit(char *str1, char *str2, char *str3, char sp)
//����:����,��Ҫ�ֽ���Ϣ,�õ���Ϣ,�ָ���
{
	char *temp = str1;
	int i = 0;
	while ((*str3++ = *str2++) != sp)i++;
	while ((*temp++ = *str2++) != '\0');
	*(--str3) = '\0';
	return str3;
}
bool myStrSame(char *str1, char *str2)
{
	if (myStrLen(str1) != myStrLen(str2))return false;
	for (int i = 0; i<myStrLen(str1); i++)
	if (*str1++ != *str2++)return false;
	return true;
}
//start set global varibal
WSADATA wsaData = { 0 };//����׽�����Ϣ
SOCKET ClientSocket = INVALID_SOCKET;//�ͻ����׽���
SOCKADDR_IN ServerAddr = { 0 };//����˵�ַ
USHORT uPort = 10002;//����˶˿�
char IP[32] = "127.0.0.1";

char msg_type = '#';
char buffer[4096] = { 0 };
char name[30] = {0};
int iRecvLen = 0;
int iSnedLen = 0;




bool startConnectServer()
//���ӷ�����
{
	//��ʼ���׽���
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return false;
	}
	//�ж��׽��ְ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("wVersion was not 2.2\n");
		return false;
	}
	//�����׽���
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("socket failed with error code: %d\n", WSAGetLastError());
		return false;
	}
	//���÷�������ַ
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(uPort);//�������˿�
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);//��������ַ
	//���ӷ�����
	if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		printf("connect failed with error code: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return false;
	}
	printf("���ӷ������ɹ� IP:%s Port:%d\n\n\n\n",
		inet_ntoa(ServerAddr.sin_addr), htons(ServerAddr.sin_port));
	return true;
}

void sendMessage(char *buf)
{
		iSnedLen = send(ClientSocket, buf, myStrLen(buf), 0);
		if (SOCKET_ERROR == iSnedLen)
		{
			printf("send failed with error code: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
		}
		else
		{
			// << buf << endl;
		}
}

void recvMessage()
{
	char buf[1024] = { 0 };
	int ret = 0;
	while (1)
	{
		this_thread::sleep_for(chrono::seconds(3));
		this_thread::yield();
		memset(buf, 0, sizeof(buf));
		ret = recv(ClientSocket, buf, sizeof(buf), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("recv failed with error code: %d\n", WSAGetLastError());
			continue;
		}
		else
		{
			char chat_name[30] = { 0 };
			char message[2048] = { 0 };
			 myStrSplit(chat_name, buf, message,'@');
			cout << chat_name << ":\t" << message << "\n\n";
		}//end else
		
	}//end while
	
}

void helpinfo()
{
	cout << "����ҳ�棨����ҳ�棩\n\n";
	cout << "���롰&�� ��ʾ�˳�\n";
	cout << "���롰?�� ����ѯ��ʽ\n";
	cout << "���롰$�� ��չʾ�������ߵ��û�\n";
	cout << "���롰message@user�� ����ʾ���û�user������Ϣ \n";
	cout << "��Ҫ�����������ַ� \n\n\n\n";
}
void main_thread()
//���߳�,�������ӷ�����+������Ϣ
{
	while (1)
	{
		Sleep(10);
		uPort = 10001;
		if (startConnectServer())break;
		uPort = 10002;
		if (startConnectServer())break;
		uPort = 10003;
		if (startConnectServer())break;
	}
	//��������
	cout << "����������û���:\n";
	cin >> name;
	myStrcat(name, "#");
	sendMessage(name);
	while (1)
	{
		buffer[0] = '\0';
		cout << "������Ҫ���͵���Ϣ����ʽΪmessage@user\n";
		cin >> buffer;
		if (myStrSame(buffer, "?"))
		{
			helpinfo();
			continue;
		}
		sendMessage(buffer);
		msg_type = buffer[myStrLen(buffer) - 1];
		Sleep(10);
		if (msg_type == '&')exit(0);//�˳��ͻ���,����ǰ���Ƿ�����Ϣ��server,��֪�˳�
	}
}

void normal_thread()
//��ͨ���߳����ڶ�ȡ��Ϣ
{
	recvMessage();
}
int main()
{
	thread normal_thread(normal_thread);
	normal_thread.detach();
	main_thread();
	system("pause");
	return 0;
}
