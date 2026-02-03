import socket
import time
import threading

# 单个客户端请求处理函数
def handle_client(connectionSocket, address):
    print(f"[新连接] 客户端{address}已连接")
    try:
        # 读取客户端发送的HTTP请求消息，解码为字符串
        message = connectionSocket.recv(1024).decode()
        # 防止空请求（客户端异常关闭）
        if not message:  
            connectionSocket.close()
            print(f"[连接关闭] 客户端{address}请求为空，已断开")
            return

        # 提取请求的文件名
        filename = message.split()[1]
        # 打开请求的文件
        with open(filename[1:], "r", encoding="utf-8") as f:
            content = f.read()

        # 构造HTTP 200响应消息
        state_row = "HTTP/1.1 200 OK\r\n"
        response_headers = (
            f"Connection: close\r\n"
            f"Date: {time.strftime('%a, %d %b %Y %H:%M:%S GMT', time.gmtime())}\r\n"
            f"Server: Apache/1.3.0 (Windows)\r\n"
            f"Last-Modified: Mon, 28 Nov 2022 00:00:00 GMT\r\n"
            f"Content-Length: {len(content.encode('utf-8'))}\r\n"  # 按字节数计算长度
            f"Content-Type: text/html; charset=utf-8\r\n\r\n"  # 规范的Content-Type
        )
        outputdata = state_row + response_headers + content

        # 发送响应给客户端
        connectionSocket.send(outputdata.encode("utf-8"))
        print(f"[响应完成] 已处理客户端{address}的请求")
    except IOError:
        # 处理文件未找到异常
        print(f"[文件缺失] 客户端{address}请求的文件不存在")
        with open("error.html", "r", encoding="utf-8") as f:
            content = f.read()
        state_row = "HTTP/1.1 404 Not Found\r\n"
        response_headers = (
            f"Connection: close\r\n"
            f"Date: {time.strftime('%a, %d %b %Y %H:%M:%S GMT', time.gmtime())}\r\n"
            f"Server: Apache/1.3.0 (Windows)\r\n"
            f"Content-Length: {len(content.encode('utf-8'))}\r\n"
            f"Content-Type: text/html; charset=utf-8\r\n\r\n"
        )
        outputdata = state_row + response_headers + content
        connectionSocket.send(outputdata.encode("utf-8"))

    finally:
        # 关闭客户端连接
        connectionSocket.close()
        print(f"[连接关闭] 客户端{address}的连接已断开")

# 启动服务器并监听连接
def main():
    # 创建TCP服务器套接字
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverPort = 6121
    # 设置端口复用
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # 绑定服务器地址和端口号
    serverSocket.bind(("", serverPort))
    # 监听连接请求，最大挂起连接数为10
    serverSocket.listen(10)
    print(f"TCP服务器已启动，监听端口{serverPort}，等待客户端连接...")

    try:
        # 循环接收客户端连接
        while True:
            # 接受客户端连接请求
            connectionSocket, address = serverSocket.accept()
            # 创建子线程处理当前客户端请求
            # daemon=True：主线程退出时子线程自动退出
            client_thread = threading.Thread(
                target=handle_client, args=(connectionSocket, address), daemon=True
            )
            # 启动子线程
            client_thread.start()
    except KeyboardInterrupt:
        # 捕获Ctrl+C
        print("\n[服务器停止] 接收到退出信号，关闭服务器...")
    finally:
        # 关闭服务器套接字
        serverSocket.close()
        print("[服务器关闭] TCP服务器已停止运行")

if __name__ == "__main__":
    main()

    