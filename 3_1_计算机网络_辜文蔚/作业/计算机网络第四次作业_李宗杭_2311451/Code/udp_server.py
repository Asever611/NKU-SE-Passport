import socket
import random

# 创建UDP服务器套接字，AF_INET表示使用IPv4，SOCK_DGRAM表示使用UDP协议
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# 绑定服务器地址和端口号，第一个参数为空字符串表示监听所有可用的接口
serverSocket.bind(('', 10000))
print("UDP服务器已启动，等待接收消息...\n")

# 循环接收和处理客户端消息
while True:
    # 接收客户端消息，缓冲区大小为1024字节
    message, address = serverSocket.recvfrom(1024)
    print("接收的消息: {}".format(message.decode("utf-8")))
    print("接收的地址: {}\n".format(address))
    # 如果接收到的消息是"exit"，则退出循环，关闭服务器
    if message.decode("utf-8") == "exit":
        print("服务器已关闭。")
        serverSocket.close()
        break
    # 简单的消息处理：将消息转换为大写
    message = message.upper()
    # 模拟数据包丢失的情况，40%的概率丢失数据包
    if random.randint(0, 10) < 4:
        print("数据包丢失\n")
        continue
    # 发送处理后的消息回客户端
    serverSocket.sendto(message, address)

