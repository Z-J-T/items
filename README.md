# items

## FTP 文件传输系统 

###  一、主要框架介绍：
**基于半同步半异步框架，使用socket完成的文件传输**
- 主线程  负责和客户端的连接 
- 子线程  处理客户端和服务器的业务 （上传 下载）

具体实现：我每个子线程用的是同一个线程函数，由主线程创建，主线程连接一个客户端accept处理产生一个新的文件描述符，通过线程函数参数传入。此后，该客户端所有的任务都由该线程处理。

### 二、功能介绍:
**首先我主要说一下我主要完成的是怎样一个功能?**

我实现的是
1. 客户端对服务器文件的管理
2. 客户端对文件的上传，下载

在上传这一块我还实现了**断点续传还有秒传**

罗马不可能是一天建成的
我的项目也是，我开始只是写了客户端对服务器文件的管理

#### 1、通过客户端管理服务器文件功能模块
**管理这一块需要我细说一下吗？**

##### 1.1 ls显示服务器列表
举个栗子：
**如果我要获取服务器的目录列表，我会怎么处理？**
首先，我客户端肯定要发来是一个字符串（"ls"），我通过我的设计（if else）让他进入相应的处理模块。

对于管理类命令，我的处理方式都是一样的
**处理模块实现：**
- 服务器端
	1. 创建管道 （具体命令 ：pipe(fd[2])）
	2. fork（）产生一个子进程 
   - 对于子线程 
      - 先关闭读端
      - 把我的标准输出和标准错误输出通过dup2重定向到我管道的写端 （dup2(fd[1],1);）
      - 通过execv进行进程替换
      - exit(0)退出子进程
  
  **具体代码实现**：
   ![](https://i.imgur.com/ubZLYfG.png)
   - 对于父进程
      - 先关闭写端 
      - wait（防止产生僵死进程））
      - 从读端读数据
      - 通过send把读到的数据发送给客户端
 
- 客户端
   - 通过用户想要的操作，输入相应的选项
   - 经过我逻辑代码的过滤（过滤错误输入，去除非操作类命令）
   - 对于管理类命令，直接发送给服务器
   - 等待服务器操作
   - recv接收到数据，然后分析判断有无有效数据，有数据，直接在屏幕上打印出来
   （返回数据格式：#ok# + 有效数据）
   - 如果说没有数据，比如说我的rm删除服务器文件，我也会回复一个"#ok#“回去，那么他打印的时候让他先跳过接收到的数据的前4个字节。（这也保证我代码的复用，不用再单独写一块没有数据的处理）

####  2、上传和下载

然后，我完成了上传和下载的功能
客户端对服务器的下载，就相当于服务器接收客户端的上传，当然还有后续的断点续传，以及秒传

##### 2.1、上传的流程
**我说一下客户端上传一个文件，我服务器端具体的处理流程？**

- 首先客户端发送过来他要上传的文件名和MD5值

![](https://github.com/Z-J-T/items/blob/master/picture/FTP%E8%AE%BE%E8%AE%A1.png?raw=true)

##### 2.2 秒传
- 先在库中搜索有没有相同MD5文件
   - 有，判断相同MD5的文件的文件名和客户端发来的文件名是否相同
      - 相同，return返回（让客户端直接打印“文件已存在”）
      - 不同，判断客户端传来的文件名有没有和库中其他文件名重复
         - 重复，通过while循环，让客户端发送新的文件名，再在库中比较有没有同名文件，有的话，继续while循环，直到该名字独一无二
         - 独一无二的话：
            - fork()产生一个子进程
            - 子进程使用execvp()进程替换执行硬链接

            - return 结束 让客户端也结束（打印硬链接完成，秒传完成）
   - 无，继续
##### 2.3 断点续传
- 进入**断点续传判断模块**：

   - **断点问题的产生**(这部分在正常上传模块部分，非判断模块)：当我文件正常收发过程中，客户端突然关闭，在服务器端必然产生的一个不完整的文件，我把服务器已经接收到的文件大小（cur_size）和MD5值保存在一个broken数组。
   
![enter description here](./images/1.png)

- 所以，我会先在broken数组中查询有无客户端发送过来的MD5
   - 如果我在broken数组中找到相同的MD5值
      - 那我就把这个数组项中的size值传给客户端
      - 让客户端的文件指针偏移size位置
      - 让服务器文件指针直接偏移到最后
      - 然后让双方开始继续收发
      - 但是我考虑更多的是：要是再次发生断点问题怎么办？
      - 把这个数组项的size值重新更新
	![enter description here](./images/2.png)
	  - return 结束
  -  如果broken数组没有相同的MD5，继续

- 判断库中有没有相同的文件名
   - 重复，通过while循环，让客户端发送新的文件名，再在库中比较有没有同名文件，有的话，继续while循环，直到该名字独一无二
   - 独一无二的
- 进入正常的文件传输
  ![enter description here](./images/3.png)

## 局域网聊天

### 一、整体介绍
#### 1.代码量
大约1200行代码

#### 2.主要实现的功能
- 登录
   - 登录成功后功能
      - 获得在线列表
      - 一对一聊天 
      - 群发
      - 离线消息
- 注册
- 退出

#### 3. 项目所使用的框架
主要是**半同步半异步模型**
#### 4. 项目中事务处理模式
**MVC模式**
#### 5. 项目中使用的开源库
   - mysql
   - libevent
   - json

### 二、主要框架介绍

![局域网聊天项目设计图](https://github.com/Z-J-T/items/blob/master/picture/%E5%B1%80%E5%9F%9F%E7%BD%91%E8%81%8A%E5%A4%A9%E9%A1%B9%E7%9B%AE%E8%AE%BE%E8%AE%A1%E5%9B%BE.png)

### 三、整体流程介绍

#### 客户端

- 首先和服务器建立连接
- 通过用户选择生成对应类型的json包
- 发送给服务器

#### 服务器

##### 半同步半异步
- 主线程创建Ser对象，调构造函数
   - 创建libevent
   - 循环
      - 创建sockpair
      - 创建threadnum条线程，每条线程线程参数为s1端
   ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/1.png?raw=true)
      - 在线程函数中创建Childthread(s1)对象
      ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/4.png?raw=true)
      - 子线程对象构造时
         - 先创建libevent
         - 把s1监听起来
   ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/3.png?raw=true)
   - 创建套接字，绑定端口号，listen
   - 创建event_new,把套接字监听起来，当有客户端来连接的时候，fd上有事件发生，就会调回调函数
![enter description here](https://github.com/Z-J-T/items/blob/master/picture/2.png?raw=true)

当客户端connect:
   - 回调函数accept处理产生一个文件描述符cli_fd
   - 先在map<s1,int> 中找一个负载量最小的sockpair端口
   - 把文件描述符通过sockpair传给子线程对象
   ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/5.png?raw=true)
   - 子线程对象s1端读到数据，调回调函数接收到cli_fd
   - 把接收到的文件描述符监听起来，至此之后该客户端所有的交互都由该线程来处理
   - 最后把自己当前负载量返回给主线程Ser对象
 ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/6.png?raw=true)
##### MVC
客户端
- 根据用户选择输入数据，使用json打包把数据send给服务器

服务端
- 子线程对象cli_fd套接字事件发生
- 调回调函数
   - 调用contral.process(fd,buff)
   - 在contral.process()中解析buff（json数据包）的TYPE，通过map[TYPE]->process(fd,buff);
   - 进入对应的业务处理模块
   - 在模块中根据逻辑在数据库中search，insert，delete数据，再给客户端send数据

客户端根据服务器返回的数据，输出不同的格式

### 四、六大业务模块

#### 1、Login
- 在数据库user表中search，用户名和密码是否存在，不存在send一个failed，给客户端
- 存在online表中insert,name和fd
- 在offline表中search有没有该用户的数据
- 有send给客户端

#### 2、Register
- 在数据库user表中insert用户名和密码，
- 由于设置了主键，insert相同用户名会插入失败

#### 3、Exit
- 把数据库online表中该用户的记录删除

#### 4、Getlist
- 把整个online表中的数据提取
- 按固定格式拼接，send给客户端

#### 5、Talkone
- 在online表中查询接收方是否在线
- 在线，获取该用户的fd,直接send数据
- 不在线，把buff中的data存入数据库offline表

#### 6、Talkgroup
- 遍历数据库user表信息
- 判断该用户是否在线
- 在线，获得fd,直接send数据
- 不在，直接存入offline表中
