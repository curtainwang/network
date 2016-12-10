/* echoserver.c */

#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>

#define BUFFSIZE 256

/*
Program: echoserver
Purpose: wait for a connection from an echoclient and echo data
Usage: echoserver <appnum>
*/

int
main(int argc, char *argv[]){
    connection conn;
    int len;
    char buff[BUFFSIZE];

    if(argc != 2){
        (void) fprintf(stderr, "usage: %s <appnum>\n", argv[0]);
        exit(1);
    }

    /* wait for a connection from an echo client */

    /* 利用标准c函数atoi完成将ASCII字符串表示的值转换为二进制形式，
       然后再将其作为参数传递给awati_contact,
       当调用await_contact返回后，程序循环调用recv从客户端接收数据，
       并调用send送回相同的数据给客户， 
    */
    conn = await_contact((appnum) atoi(argv[1]));
    if(conn < 0){
        exit(1);
    }

    /* iterate, echoing all data received until end of file */

    while((len = recv(conn, buff, BUFFSIZE, 0)) > 0){
        (void) send(conn, buff, len, 0);
    }
    /* 当recv发现end-of-file后，停止循环收发并返回0值，此时服务器给客户端也发送end-of-file */
    send_eof(conn);
    return 0;
}