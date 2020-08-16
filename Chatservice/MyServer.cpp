#include<thread>
#include <WinSock2.h>
#include<iostream>
#include<vector>
#include <csignal>
#include<exception>
#include<mutex>
#pragma comment(lib,"ws2_32.lib")

using namespace std;


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
	while ((*temp++ = *str2++) != '\0');
	return str1;
}
char *myStrcp(char *str1, char* str2)
{
	char *temp = str1;
	while ((*temp++ = *str2++) != '\0');
	return str1;
}
bool myStrSame(char *str1, char *str2)
{
	if (myStrLen(str1) != myStrLen(str2))return false;
	for (int i = 0; i<myStrLen(str1); i++)
	if (*str1++ != *str2++)return false;
	return true;
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
bool myStrcontoins(char *str, char ch)
//�ж��Ƿ�����ַ�
{
	while (*str++ != '\0')
	{
		if (*str == ch)return true;
	}
	return false;
}
//start set global variable
mutex mu;
DWORD exitCode = 1;
HANDLE hThrd1;

SOCKET ServerSocket = INVALID_SOCKET;//������׽���
SOCKET ClientSocket = INVALID_SOCKET;//�ͻ����׽���
SOCKADDR_IN ServerAddr = { 0 };//����˵�ַ
SOCKADDR_IN ClientAddr = { 0 };//�ͻ��˵�ַ
SOCKET ServerSocket1 = INVALID_SOCKET;//������׽���
SOCKET ServerSocket2= INVALID_SOCKET;//������׽���
SOCKET ServerSocket3= INVALID_SOCKET;//������׽���
USHORT uPort1 = 10001;//�����������˿�
USHORT uPort2 = 10002;//�����������˿�
USHORT uPort3 = 10003;//�����������˿�

int iClientAddrLen = sizeof(ClientAddr);

struct Client
{
	SOCKET client_socket;
	char clint_name[255];
};
Client temp_client;
vector<Client> clients;



char buffer[4096] = { 0 };
int iRecvLen = 0;
int iSendLen = 0;
char msg_type = '#';
int create_thread_flag = 1;//��ʱ�����߳�



bool showClients()
//�鿴���ߵ��û�
{
	if (clients.size() == 0)
	{
		cout << "\n����û�����ߵ��û�\n";
		return false;
	}
	cout << "\n���ڵ��û���Ϊ��" << clients.size() << endl;;
	for (auto it = clients.begin(); it < clients.end(); it++)
		cout << it->clint_name << endl;
	return true;
}

void deleteClient(char *name)
//�뿪�Ŀͻ��˽��д���
{
		for (auto it = clients.begin(); it < clients.end(); it++)
		{
			if (myStrSame(name, it->clint_name))
			{
				clients.erase(it);
				cout << "ɾ���û���  " << name << " �ɹ�\n";
				return;
			}
		}
	
}

int startServer(SOCKET* server_socket, const USHORT server_port)
//�򿪷�����
{
	//����׽�����Ϣ�Ľṹ
	WSADATA wsaData = { 0 };
	//��ʼ���׽���
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}
	//�жϰ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("wVersion was not 2.2\n");
		return -1;
	}
	*server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*server_socket == INVALID_SOCKET)
	{
		printf("socket failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	//���÷�������ַ
	ServerAddr.sin_family = AF_INET;//���ӷ�ʽ
	ServerAddr.sin_port = htons(server_port);//�����������˿�
	ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�κοͻ��˶����������������

	//�󶨷�����
	if (SOCKET_ERROR == ::bind(*server_socket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		printf("bind failed with error code: %d\n", WSAGetLastError());
		closesocket(*server_socket);
		return -1;
	}
	if (SOCKET_ERROR == listen(*server_socket, 20))
	{
		printf("listen failed with error code: %d\n", WSAGetLastError());
		closesocket(*server_socket);
		WSACleanup();
		return -1;
	}
	cout << "starting server...\n";
	return 0;

}

bool isExistClient(char *name)
//�жϿͻ����Ƿ������clients��
{
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		if (myStrSame(name, it->clint_name))return true;
	}
	return false;
}

void sendMessage(Client * client, char * message, char *name)
//������Ϣ,Ŀ�ĵص�name,�Լ���Ϣmessage,ת������Ϣ��ʽ:message@sender name
{
	char buf[2048] = { 0 };
	buf[0] = '\0';
	int ret = 0;
	//Ѱ��Ŀ�Ŀͻ���,��ʼ������Ϣ
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		if (myStrSame(name, it->clint_name))
		{
			myStrcat(buf, message);
			myStrcat(buf, "@");
			myStrcat(buf, client->clint_name);
			cout << "���ͳ�ȥ����Ϣ��:" << message << "@" << client->clint_name << endl;
			ret = send(it->client_socket, buf, myStrLen(buf), 0);
			break;
		}
	}
	if (SOCKET_ERROR == ret)
	{
		printf("send failed with error code: %d\n", WSAGetLastError());
		//WSACleanup();
	}
	else
	{
		buf[0] = '\0';
	}
}

void recvMessage(Client& client)
//��ȡ�ͻ��˵���Ϣ
{
		
		char buf[1024] = { 0 };
		int ret = 0;
		memset(buf, 0, sizeof(buf));
		ret = recv(client.client_socket, buf, sizeof(buf), 0);

		if (SOCKET_ERROR == ret)
		{			
			//deleteClient(client.clint_name);
			printf("recv failed with error code: %d\n", WSAGetLastError());
			return;
		}
		else
		{
			char messege[2048] = { 0 };
			char dest_name[36] = { 0 };
			messege[0] = '\0';
			dest_name[0] = '\0';

			char judge_type = buf[myStrLen(buf) - 1];
			buffer[myStrLen(buf) - 1] = '\0';

			if (myStrcontoins(buf,'@'))
				myStrSplit(dest_name, buf, messege, '@');//����:����,�ֽ���Ϣ,�õ���Ϣ,�ָ���

			switch (judge_type)
			{
			case '#'://���ͻ��˼��뵽clients,ѡ�����,�ڱ�ĺ����Ѿ�ʵ��
			{
				 break;
			}
			case '&'://����clients
			{
				deleteClient(client.clint_name);
				break;
			}
			case '$':
			{
						
						messege[0] = '\0';
						myStrcat(messege,"���ߵ��û���:\n");
						for (auto it = clients.begin(); it < clients.end(); it++)
						{
							myStrcat(messege, it->clint_name);
							myStrcat(messege, "\n");
						}
						sendMessage(&client, messege, client.clint_name);
				break;
			}
			default:
			{

					   if (myStrLen(dest_name)>=1)
					   {
						   cout << "ת��������:" << client.clint_name << "to" << dest_name << ":" << messege << endl;
						   sendMessage(&client, messege, dest_name);
						   buf[0] = '\0';
					   }
					   else
					   {
						   cout << "����Ϣ���յ��ˣ�����ת�����û�������\n";
					   }
					   break;
			}
				
			}//end switch
		}//end else
}

void forwardingMessage()
//���պ�ת����Ϣ�ĺ���
{
	
	while (1)
	{
		for (auto it = clients.begin(); it < clients.end(); it++)
		{
			if (myStrSame(it->clint_name, "tempname"))continue;
			if (isExistClient(it->clint_name))
			{
				thread t1(recvMessage, (*it));
				t1.detach();
			}
		}
		this_thread::sleep_for(chrono::seconds(3));
		this_thread::yield();
	}//end while
	
}

void addClient()
//��Ҫ�Ǵ���õ���client socket������,��tempname�滻��
{
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		std::cout << clients.size() << endl;
		if (myStrSame(it->clint_name, "tempname"))
		{
			memset(buffer, 0, sizeof(buffer));
			//���տͻ�����Ϣ,��Ϣ���ݽ����������ֲŽ���
			iRecvLen = recv(it->client_socket, buffer, sizeof(buffer), 0);
			char judge_type = buffer[myStrLen(buffer) - 1];
			buffer[myStrLen(buffer) - 1] = '\0';
			if (isExistClient(buffer))//�û����ڵ����,ɾ��ǰ����û�
			{
				std::cout << "�ͻ��˴���:" << buffer << endl;
				deleteClient("tempname");
				return;
			}
			if (judge_type == '#')
			{
				std::cout << "��ȡ���ĵ�¼��Ϊ:" << buffer << endl;
				myStrcp(it->clint_name, buffer);
				std::cout << "����������:" << it->clint_name << endl;
				buffer[0] = '\0';
				std::cout << "����û��ɹ�\n";
				return;
			}
			else
			{
				std::cout << "����û����ִ���" << endl;
			}
		}
		
	}
	
	std::cout << "����û�ʧ��\n";
	return ;
}

void mulPortServer(SOCKET* server_socket, const USHORT server_port)
//�������̵߳ķ�����
{
	//1.�򿪷�����,����ɽ�������
	startServer(server_socket,server_port);

	//2.��ȡ��ת����Ϣ���߳�
	thread forward_thread(forwardingMessage);
	forward_thread.detach();//�ŷ��߳�

	//3.��ʼ�������߳�ѭ��
	while (1)
	{
		ClientSocket = accept(*server_socket, (SOCKADDR*)&ClientAddr, &iClientAddrLen);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("accept failed with error code: %d\n", WSAGetLastError());
			closesocket(*server_socket);
			WSACleanup();
			continue;
		}
		else
		{
			printf("�пͻ��˽��� IP:%s Port:%d\n\n",
				inet_ntoa(ClientAddr.sin_addr), htons(ClientAddr.sin_port));
			//���ͻ��˵�socket������,�����޸�name
			myStrcp(temp_client.clint_name, "tempname");
			temp_client.client_socket = ClientSocket;
			clients.push_back(temp_client);
			showClients();	//show online users 
			addClient();	//add user
			showClients();	//show online users 
		}
		Sleep(2);
	}
}

int main()
{
	
	thread serverthread1(mulPortServer,&ServerSocket1,uPort1);
	thread serverthread2(mulPortServer, &ServerSocket2, uPort2);
	thread serverthread3(mulPortServer, &ServerSocket3, uPort3);

	serverthread2.join();
	serverthread1.join();
	serverthread3.join();
	::system("pause");

	return 0;
}
