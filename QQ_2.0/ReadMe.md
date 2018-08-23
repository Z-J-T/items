# QQ轻聊（局域网聊天）

## 一、整体介绍
### 1.代码量
大约1200行代码

### 2.主要实现的功能
- 登录
   - 登录成功后功能
      - 获得在线列表
      - 一对一聊天 
      - 群发
      - 离线消息
- 注册
- 退出

### 3. 项目所使用的框架
主要是**半同步半异步模型**
### 4. 项目中事务处理模式
**MVC模式**
### 5. 项目中使用的开源库
   - mysql
   - libevent
   - json

## 二、主要框架介绍

![局域网聊天项目设计图](https://github.com/Z-J-T/items/blob/master/picture/%E5%B1%80%E5%9F%9F%E7%BD%91%E8%81%8A%E5%A4%A9%E9%A1%B9%E7%9B%AE%E8%AE%BE%E8%AE%A1%E5%9B%BE.png)

## 三、整体流程介绍

### 客户端

- 首先和服务器建立连接
- 通过用户选择生成对应类型的json包
- 发送给服务器

### 服务器

#### 半同步半异步
- 主线程创建Ser对象，调构造函数
   - 创建libevent
   - 循环
      - 创建sockpair
      - 创建threadnum条线程，每条线程线程参数为s1端
     </br> ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/1.png?raw=true)
      - 在线程函数中创建Childthread(s1)对象
      </br>![enter description here](https://github.com/Z-J-T/items/blob/master/picture/4.png?raw=true)
      - 子线程对象构造时
         - 先创建libevent
         - 把s1监听起来
        ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/3.png?raw=true)
   - 创建套接字，绑定端口号，listen
   - 创建event_new,把套接字监听起来，当有客户端来连接的时候，fd上有事件发生，就会调回调函数
  </br> ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/2.png?raw=true)

当客户端connect:
   - 回调函数accept处理产生一个文件描述符cli_fd
   - 先在map<s1,int> 中找一个负载量最小的sockpair端口
   - 把文件描述符通过sockpair传给子线程对象
  </br> ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/5.png?raw=true)
   - 子线程对象s1端读到数据，调回调函数接收到cli_fd
   - 把接收到的文件描述符监听起来，至此之后该客户端所有的交互都由该线程来处理
   - 最后把自己当前负载量返回给主线程Ser对象
 </br>  ![enter description here](https://github.com/Z-J-T/items/blob/master/picture/6.png?raw=true)
#### MVC
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

## 四、六大业务模块

### 1、Login
- 在数据库user表中search，用户名和密码是否存在，不存在send一个failed，给客户端
- 存在online表中insert,name和fd
- 在offline表中search有没有该用户的数据
- 有send给客户端

### 2、Register
- 在数据库user表中insert用户名和密码，
- 由于设置了主键，insert相同用户名会插入失败

### 3、Exit
- 把数据库online表中该用户的记录删除

### 4、Getlist
- 把整个online表中的数据提取
- 按固定格式拼接，send给客户端

### 5、Talkone
- 在online表中查询接收方是否在线
- 在线，获取该用户的fd,直接send数据
- 不在线，把buff中的data存入数据库offline表

### 6、Talkgroup
- 遍历数据库user表信息
- 判断该用户是否在线
- 在线，获得fd,直接send数据
- 不在，直接存入offline表中
