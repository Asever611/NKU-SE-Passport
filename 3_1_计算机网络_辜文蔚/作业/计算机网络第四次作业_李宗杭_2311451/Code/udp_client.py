import socket
import time

port = 10000
address = "127.0.0.1"
# address = "192.168.31.36"
count = 0  # 丢包数
start = 0  # 标记是否为第一次接收消息
totaltime = 0  # 总往返时延RTT

# 创建UDP客户端套接字
clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# 设置超时时间（往返时延RTT）为1秒
clientSocket.settimeout(1)

# 发送10次ping消息并计算RTT
for i in range(10):
    print("test_" + str(i) + "_RRT: ", end = '')
    t1 = time.time()  # 记录发送时间
    # 发送ping消息到服务器，Socket传输的是字节流，因此需要将字符串编码为字节
    clientSocket.sendto("ping".encode("utf-8"), (address, port))
    try:
        # 接收服务器的响应消息，缓冲区大小为1024字节
        message, recv_address = clientSocket.recvfrom(1024)
    except:
        # 超时未收到响应，视为丢包
        print("out of time!!!")
        count = count + 1
        continue
    t2 = time.time()  # 记录接收时间

    # 计算往返时延RTT并更新最小、最大和平均RTT
    if (start == 0):  # 第一次接收消息，初始化最小和最大RTT
        mintime = (t2 - t1)
        maxtime = (t2 - t1)
        start = 1
    elif ((t2 - t1) < mintime):
        mintime = (t2 - t1)
    elif ((t2 - t1) > maxtime):
        maxtime = (t2 - t1)
    totaltime = totaltime + (t2 - t1)
    print('%.15f' % (t2 - t1))

# 发送退出消息给服务器
clientSocket.sendto("exit".encode("utf-8"), (address, port))
# 关闭客户端套接字
clientSocket.close()

# 输出RTT统计信息和丢包率
print('min_RRT:' + str(mintime))
print('max_RRT:' + str(maxtime))
print('average_RRT:' + str(totaltime / (10 - count)))
print('packet loss rate:' + str(count / 10))

