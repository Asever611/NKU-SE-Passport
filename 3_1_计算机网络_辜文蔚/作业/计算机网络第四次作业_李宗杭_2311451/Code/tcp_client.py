import time
import socket

address = "127.0.0.1"
# address = "192.168.31.36"
port = 6121

# 创建TCP客户端套接字
clientsocket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
# 连接服务器
clientsocket.connect((address,port))

# 构造HTTP请求消息
fetch_file = "/myfile.html"  # 请求的文件名
requestRow = "Get " + fetch_file + " HTTP/1.1\r\n"
firstRow = "Host:localhost\r\nUser-agent:Microsoft Edge/100.0.1185.36\r\nConnection:keep-alive\r\nAccept-language:ch\r\n\r\n"
requestMessages = requestRow + firstRow

start = time.perf_counter()  # 开始计时
# 发送HTTP请求消息给服务器
clientsocket.send(requestMessages.encode())
# 接收服务器的响应消息
responseMessage = clientsocket.recv(1024)
end = time.perf_counter()  # 结束计时

print("RTT:", end - start, "s")
print("响应报文：\n", responseMessage.decode("utf-8"), sep = "")

# 将响应内容写入本地文件
f = open('localHtml.html','w')
message = responseMessage.decode()
message = message.split("\r\n")
content = message[-1]
f.write(content)
f.close()

# 关闭客户端套接字
clientsocket.close()

