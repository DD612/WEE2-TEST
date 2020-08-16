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
//参数:姓名,需要分解信息,得到信息,分隔符
{
	char *temp = str1;
	int i = 0;
	while ((*str3++ = *str2++) != sp)i++;
	while ((*temp++ = *str2++) != '\0');
	*(--str3) = '\0';
	return str3;
}
bool myStrcontoins(char *str, char ch)
//判断是否存在字符
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

SOCKET ServerSocket = INVALID_SOCKET;//服务端套接字
SOCKET ClientSocket = INVALID_SOCKET;//客户端套接字
SOCKADDR_IN ServerAddr = { 0 };//服务端地址
SOCKADDR_IN ClientAddr = { 0 };//客户端地址
SOCKET ServerSocket1 = INVALID_SOCKET;//服务端套接字
SOCKET ServerSocket2= INVALID_SOCKET;//服务端套接字
SOCKET ServerSocket3= INVALID_SOCKET;//服务端套接字
USHORT uPort1 = 10001;//服务器监听端口
USHORT uPort2 = 10002;//服务器监听端口
USHORT uPort3 = 10003;//服务器监听端口

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
int create_thread_flag = 1;//此时创建线程



bool showClients()
//查看在线的用户
{
	if (clients.size() == 0)
	{
		cout << "\n现在没有在线的用户\n";
		return false;
	}
	cout << "\n现在的用户数为：" << clients.size() << endl;;
	for (auto it = clients.begin(); it < clients.end(); it++)
		cout << it->clint_name << endl;
	return true;
}

void deleteClient(char *name)
//离开的客户端进行处理
{
		for (auto it = clients.begin(); it < clients.end(); it++)
		{
			if (myStrSame(name, it->clint_name))
			{
				clients.erase(it);
				cout << "删除用户：  " << name << " 成功\n";
				return;
			}
		}
	
}

int startServer(SOCKET* server_socket, const USHORT server_port)
//打开服务器
{
	//存放套接字信息的结构
	WSADATA wsaData = { 0 };
	//初始化套接字
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}
	//判断版本
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

	//设置服务器地址
	ServerAddr.sin_family = AF_INET;//连接方式
	ServerAddr.sin_port = htons(server_port);//服务器监听端口
	ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//任何客户端都能连接这个服务器

	//绑定服务器
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
//判断客户端是否存在在clients中
{
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		if (myStrSame(name, it->clint_name))return true;
	}
	return false;
}

void sendMessage(Client * client, char * message, char *name)
//发送消息,目的地的name,以及信息message,转发的消息格式:message@sender name
{
	char buf[2048] = { 0 };
	buf[0] = '\0';
	int ret = 0;
	//寻找目的客户端,开始发送信息
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		if (myStrSame(name, it->clint_name))
		{
			myStrcat(buf, message);
			myStrcat(buf, "@");
			myStrcat(buf, client->clint_name);
			cout << "发送出去的消息是:" << message << "@" << client->clint_name << endl;
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
//读取客户端的消息
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
				myStrSplit(dest_name, buf, messege, '@');//参数:姓名,分解信息,得到信息,分隔符

			switch (judge_type)
			{
			case '#'://将客户端加入到clients,选择忽略,在别的函数已经实现
			{
				 break;
			}
			case '&'://减少clients
			{
				deleteClient(client.clint_name);
				break;
			}
			case '$':
			{
						
						messege[0] = '\0';
						myStrcat(messege,"在线的用户是:\n");
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
						   cout << "转发过程是:" << client.clint_name << "to" << dest_name << ":" << messege << endl;
						   sendMessage(&client, messege, dest_name);
						   buf[0] = '\0';
					   }
					   else
					   {
						   cout << "此信息接收到了，但是转发的用户不存在\n";
					   }
					   break;
			}
				
			}//end switch
		}//end else
}

void forwardingMessage()
//接收和转发消息的函数
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
//主要是处理得到的client socket的名字,将tempname替换掉
{
	for (auto it = clients.begin(); it < clients.end(); it++)
	{
		std::cout << clients.size() << endl;
		if (myStrSame(it->clint_name, "tempname"))
		{
			memset(buffer, 0, sizeof(buffer));
			//接收客户端消息,消息传递进来的是名字才接受
			iRecvLen = recv(it->client_socket, buffer, sizeof(buffer), 0);
			char judge_type = buffer[myStrLen(buffer) - 1];
			buffer[myStrLen(buffer) - 1] = '\0';
			if (isExistClient(buffer))//用户存在的情况,删除前面的用户
			{
				std::cout << "客户端存在:" << buffer << endl;
				deleteClient("tempname");
				return;
			}
			if (judge_type == '#')
			{
				std::cout << "读取到的登录名为:" << buffer << endl;
				myStrcp(it->clint_name, buffer);
				std::cout << "处理后的姓名:" << it->clint_name << endl;
				buffer[0] = '\0';
				std::cout << "添加用户成功\n";
				return;
			}
			else
			{
				std::cout << "添加用户出现错误" << endl;
			}
		}
		
	}
	
	std::cout << "添加用户失败\n";
	return ;
}

void mulPortServer(SOCKET* server_socket, const USHORT server_port)
//开启多线程的服务器
{
	//1.打开服务器,让其可接受连接
	startServer(server_socket,server_port);

	//2.读取和转发消息的线程
	thread forward_thread(forwardingMessage);
	forward_thread.detach();//迸发线程

	//3.开始进行主线程循环
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
			printf("有客户端接入 IP:%s Port:%d\n\n",
				inet_ntoa(ClientAddr.sin_addr), htons(ClientAddr.sin_port));
			//将客户端的socket存起来,后面修改name
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
