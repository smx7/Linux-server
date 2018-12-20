#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/epoll.h>

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        printf("usage: ./name port");
    }
    int i,j;
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr=INADDR_ANY;
    socklen_t len=sizeof(server_addr);
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
    if(ret<0)
    {
        perror("listen error!");
        return 1;
    }
    int epoll_fd=epoll_create(10);
    if(epoll_fd<0)
    {
        perror("epoll create error!");
        return 1;
    }
    struct epoll_event event;
    event.data.fd=listen_fd;
    event.events=EPOLLIN;
    ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&event);
    if(ret<0)
    {
        perror("epoll ctl error!");
        return 1;
    }
    while(1)
    {
        struct epoll_event events[10];
        int size=epoll_wait(epoll_fd,events,10,-1);
        if(size<0)
        {
            perror("epoll wait error!");
            continue;
        }
        if(size==0)
        {
            printf("time out!\n");
            continue;
        }
        printf("sockfd ready!\n");
        for(i=0;i<size;i++)
        {
            if(events[i].events!=EPOLLIN)
            {
                continue;
            }
            if(events[i].data.fd==listen_fd)
            {
                struct sockaddr_in client_addr;
                int connect_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&len);
                if(connect_fd<0)
                {
                    perror("accept error!");
                    continue;
                }
                printf("a new client connected!\n");
                struct epoll_event ev;
                ev.data.fd=connect_fd;
                ev.events=EPOLLIN;
                ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,connect_fd,&ev);
                if(ret<0)
                {
                    perror("epoll ctl error!");
                    continue;
                }
            }
            else
            {
                char buff[1024]={0};
                int rlen=recv(events[i].data.fd,buff,1023,0);
                if(rlen<0)
                {
                    perror("recv error!");
                    continue;
                }
                else if(rlen==0)
                {
                    printf("client close!\n");
                    close(events[i].data.fd);
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
                    continue;
                }
                printf("client: %s\n",buff);
                send(events[i].data.fd,buff,1023,0);
            }
        }
    }
    close(listen_fd);
    return 0;
}
