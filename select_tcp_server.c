#include<stdio.h>
#include<stdlib.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

int main(int argc,char* argv[])
{
    int i,j;
    if(argc!=2)
    {
        printf("./name port!");
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr=INADDR_ANY;

    int listen_sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_sockfd<0)
    {
        perror("create socket error!");
        return 1;
    }
    if(bind(listen_sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("bind error!");
        return 1;
    }
    if(listen(listen_sockfd,5)<0)
    {
        perror("listen error!");
        return 1;
    }
    fd_set read_fds;
    int fd_list[1024];
    int max_fd;
    for(i=0;i<1024;i++)
    {
        fd_list[i]=-1;
    }
    fd_list[0]=listen_sockfd;
    while(1)
    {
        max_fd=listen_sockfd;
        FD_ZERO(&read_fds);
        for(i=0;i<1024;i++)
        {
            if(fd_list[i]!=-1)
            {
                FD_SET(fd_list[i],&read_fds);
                max_fd=max_fd>fd_list[i]?max_fd:fd_list[i];
            }
        }
        struct timeval tv;
        tv.tv_sec=3;
        tv.tv_usec=0;
        int ret=select(max_fd+1,&read_fds,NULL,NULL,&tv);
        if(ret<0)
        {
            perror("select error!");
            continue;
        }
        else if(ret==0)
        {
            printf("no sockfd ready\n");
            continue;
        }

        for(i=0;i<max_fd+1;i++)
        {
            if(FD_ISSET(i,&read_fds))
            {
                if(i==listen_sockfd)
                {
                    struct sockaddr_in client_addr;
                    socklen_t len =sizeof(client_addr);
                    int new_fd=accept(listen_sockfd,(struct sockaddr*)&client_addr,&len);
                    if(new_fd<0)
                    {
                        perror("accepr error!");
                        continue;
                    }
                    printf("a new client connect!\n");
                    for(j=0;j<1024;j++)
                    {
                        if(fd_list[j]==-1)
                        {
                            fd_list[j]=new_fd;
                            max_fd=max_fd>new_fd?max_fd:new_fd;
                            break;
                        }
                    }

                }
                else
                {
                    char buff[1024]={0};
                    int rlen=recv(i,buff,1023,0);
                    if(rlen<=0)
                    {
                        if(errno==EAGAIN||errno==EINTR)
                        {
                            continue;
                        }
                        perror("recv error!");
                        close(i);
                        for(j=0;j<1024;j++)
                        {
                            if(i==fd_list[j])
                            {
                                fd_list[j]=-1;
                            }
                        }
                    }
                    printf("client: %s\n",buff);
                }
            }
        }
    }


    close(listen_sockfd);
    return 0;
}
