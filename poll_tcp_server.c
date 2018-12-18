#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<poll.h>

int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        printf("Usage: ./name port!\n");
    }
    int i,j;
    struct sockaddr_in server_addr;
    socklen_t len=sizeof(server_addr);
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr=INADDR_ANY;
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0)
    {
        perror("create socket error!");
        return 1;
    }
    int ret=bind(listen_fd,(struct sockaddr*)&server_addr,len);
    if(ret<0)
    {
        perror("bind error!");
        return 1;
    }
    ret=listen(listen_fd,5);
    if(listen_fd<0)
    {
        perror("listen error!");
        return 1;
    }
    struct pollfd fd_list[1024];
    for(i=0;i<1024;i++)
    {
        fd_list[i].fd=-1;
        fd_list[i].events=0;
        fd_list[i].revents=0;
    }
    for(i=0;i<1024;i++)
    {
        if(fd_list[i].fd==-1)
        {
            fd_list[i].fd=listen_fd;
            fd_list[i].events=POLLIN;
            break;
        }
    }
    while(1)
    {
        ret=poll(fd_list,1024,2000);
        if(ret<0)
        {
            perror("poll error!");
            continue;
        }
        else if(ret==0)
        {
            printf("time out!\n");
            continue;
        }
        for(i=0;i<1024;i++)
        {
            if(fd_list[i].fd==-1)
            {
                continue;
            }
            if(fd_list[i].revents!=POLLIN)
            {
                continue;
            }
            if(fd_list[i].fd==listen_fd)
            {
                struct sockaddr_in client_addr;
                int new_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&len);
                if(new_fd<0)
                {
                    perror("accept error!");
                    continue;
                }
                printf("a new client connected!");
                for(j=0;j<1024;j++)
                {
                    if(fd_list[j].fd==-1)
                    {
                        fd_list[j].fd=new_fd;
                        fd_list[j].events=POLLIN;
                        break;
                    }
                }
            }
            else
            {
                char buff[1024]={0};
                int rlen=recv(fd_list[i].fd,buff,1023,0);
                if(rlen<0)
                {
                    perror("recv error!");
                    continue;
                }
                else if(rlen==0)
                {
                    printf("client close!\n");
                    close(fd_list[i].fd);
                    fd_list[i].fd=-1;
                    continue;
                }
                printf("client: %s\n",buff);
            }
        }
    }
    close(listen_fd);
    return 0;
}
