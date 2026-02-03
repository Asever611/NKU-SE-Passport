// main.cpp
#include <mpi.h>
#include <cstdio>
#include <cstring>
#include "utils.h"
#include "integral_mpi.h"
#include "sort_mpi.h"
#include "matrix_multiply_mpi.h"
using namespace std;

int main()
{
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /*
        实验时若出现通信错误（按老师提供的文档配置环境后，helloworld可正常输出，但无法通过下面通信测试）
        可能的解决方案（本人遇到）
        1. 在配置主机名和IP解析这一步时，执行vim /etc/hosts（需root用户）
        我们注释掉了其他所有，仅保留ip与主机名的映射（如192.168.0.94 ecs-hw-0001），
        这没毛病
        2. 在helloworld测试中，编写config文件这一步
        由于配置了上一条，有了ip与主机名的映射，故此处config文件中只需写主机名即可，
        （如ecs-hw-0001:2，其中2表示该主机上运行2个进程），
        这也没毛病
        3. 此时若你直接进行通信测试，应该是正常的
        4. 但是，当我们今晚睡觉关闭了服务器，第二天早上再打开时，你发现通信出错
        这是由于，服务器重启后，他会在你的/etc/hosts文件中自动生成本机的回环地址映射，
        （如127.0.0.1       ecs-hw-0001     ecs-hw-0001），
        这就是导致通信错误的原因
        5. 解决方案很简单
        重新vim /etc/hosts，删除自动生成的这一行，保存退出即可，
        但每当你睡了一觉之后，都需要重复这一步，除非你一天就写完了作业，
        因此也可以选择不管/etc/hosts，而是在config文件中直接写ip地址，
        （如192.168.0.94:2）
    */

    
    // char processor_name[MPI_MAX_PROCESSOR_NAME];
    // int name_len;
    // MPI_Get_processor_name(processor_name, &name_len);
    // printf("Hello from processor %s, rank %d / %d\n",
    //     processor_name, my_rank, size);

    // const char* msg = "Hello from rank 0!";
    // char recv_buf[100] = {0};
    // if (my_rank == 0) {
    //     // 主进程：向所有从进程发送消息
    //     for (int i = 1; i < size; i++) {
    //         MPI_Send(msg, strlen(msg)+1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
    //         printf("[Rank 0] Sent message to rank %d\n", i);
    //     }
    // } else {
    //     // 从进程：接收主进程的消息
    //     MPI_Recv(recv_buf, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //     printf("[Rank %d] Received message: %s\n", my_rank, recv_buf);
    // }

    // 以上用于点对点通信测试，正式测试下面任务时需注释上面代码，否则可能通信错误

    //test_integral_mpi();
    if (my_rank == 0) printf("\n");
    test_sort_mpi();
    if (my_rank == 0) printf("\n");
    test_matrix_multiply_mpi();

    MPI_Finalize();
    return 0;
}
