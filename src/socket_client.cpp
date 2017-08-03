/* File Name: client.c */  
  
#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<errno.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ros/ros.h" 
#include "std_msgs/String.h"
#include "std_msgs/Int16.h" 
  
#define MAXLINE 4096 

const char* server_ip = "192.168.3.117";
int    sockfd, n,rec_len;    
char   buf[MAXLINE];  
struct sockaddr_in    servaddr;


unsigned char open_send_cmd[]= {0xAA, 0x0F, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
unsigned char close_send_cmd[]= {0xAA, 0x10, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; 	  

int count = 0;
bool already_open_flag = false;
bool already_connect_server = false;

void MessageCallBack(const std_msgs::Int16& toggle_msg) // 5hz
{
	int ret = -1;
	
	if (already_connect_server == true)
	{
		if (toggle_msg.data == 1 && already_open_flag == false /*&& serialport.blueteeth_connect_status == true*/) // 1: open the door 0: close the door
		{
			printf("in data = 1, should open the door!\n");
			
				
			count = 0;
			// send open cmd to relay
			if( send(sockfd, &open_send_cmd, 20, 0) < 0)  
			{  
				printf("send open msg error: %s(errno: %d)\n", strerror(errno), errno);
				send(sockfd, &open_send_cmd, 20, 0);  
				//exit(0);  
			}
			//serialport.count++;
			already_open_flag = true;
		}
		else if (count < 5 )
		{
			count++;  
		}
		else if (count >= 5 )
		{
			count = 0;
			if (already_open_flag == true)
			{
				//send close cmd to relay
				if( send(sockfd, &close_send_cmd, 20, 0) < 0)  
				{  
					printf("send close msg error: %s(errno: %d)\n", strerror(errno), errno);
					send(sockfd, &close_send_cmd, 20, 0);  
					//exit(0);  
				}
				already_open_flag = false;
			}
		}	
	}
	
		
}
  
int main(int argc, char** argv)  
{  
	ros::init(argc, argv, "socket_relay");
	ros::NodeHandle n;
  
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
		printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);  
		exit(0);  
	}  
    //参数初始化
	memset(&servaddr, 0, sizeof(servaddr));  
	servaddr.sin_family = AF_INET;  
	servaddr.sin_port = htons(8080);  
	if( inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0){  
		printf("inet_pton error for %s\n",argv[1]);  
		exit(0);  
	}  
    //连接服务器
	if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){  
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);  
		exit(0);  
	}
	else
	{
		already_connect_server = true;	
	}  
   
	ros::Subscriber sub = n.subscribe("/door_enabler", 5, MessageCallBack);

	ros::Rate loop_rate(10);
	
	while (ros::ok())
	{	
		ros::spinOnce();

		loop_rate.sleep();
	}
/*  
	if((rec_len = recv(sockfd, buf, MAXLINE,0)) == -1) {  
	   perror("recv error");  
	   exit(1);  
	}  
	buf[rec_len]  = '\0';  
	printf("Received : %s ",buf);  
	close(sockfd);  
	exit(0); 
*/ 
}  

