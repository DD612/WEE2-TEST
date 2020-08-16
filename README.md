# WEE2-TEST

# c++实现简易聊天系统
可实现的功能：
1. 多个客户端登陆
2. 多个客户端可通过服务器互相发送信息
3. 查看同时登陆的用户数量及名字

# 使用说明
1. 下载“Chatservice”，使用CMake先创建工程，在新生成的文件夹中会看到Charservice.sln(创建和编译工程的步骤可以参考：https://blog.csdn.net/weixin_39951988/article/details/88719634?utm_medium=distribute.pc_relevant_t0.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase)
2. 使用Visual Studio 2019打开Chatservice.sln, 右键点击“解决方案Chatservice”后，选择“设置启动项目”，勾选多个启动项目，讲MyClient和MyServer设置为启动，然后点击确定，即可在VS中直接运行。
3. 编译好后，会在工程目录下看到Debug文件夹，文件夹中有可执行文件“MyClient.exe”和“MyServer.exe”，即可执行。
4. 如不想编译，在仓库Chatserviceexe中的Debug文件夹中既有编译好的“MyClient.exe”和“MyServer.exe”，可直接执行。
5. 先启动服务端MyServer.exe，显示Starting Server...后再启动客户端MyServer.exe。
6. 客户端需要输入用户名，可随机设置自己的用户名。
7. 输入用户名后，即可向别的用户发送信息，支持多个客户端互相发送信息，发送信息的格式为message@username
8. 还可以输入“？”来查看帮助界面，输入“$”来查询当前在线用户信息，输入“&”来退出客户端。




