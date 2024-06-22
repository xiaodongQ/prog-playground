# 简单测试：起 server 服务，其中端口默认8080
# docker build -t test-tcp-connect .
# docker run -d -p 8080:8080 --name my-test-tcp-connect-container test-tcp-connect
# 使用一个带有g++的base image  
FROM gcc:latest  
  
# 设置工作目录  
WORKDIR /app  
  
# 将你的C++代码复制到Docker容器中  
COPY server.cpp client.cpp make.sh /app/

# 编译你的C++代码（假设你有一个名为main.cpp的文件）  
RUN bash make.sh 
  
# 运行编译后的程序  
CMD ["./server"]
