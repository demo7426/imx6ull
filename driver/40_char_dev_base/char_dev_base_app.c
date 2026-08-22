/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	char_dev_app.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.14
描  述: 字符设备驱动应用
备  注:
修改记录:

  1.  日期: 2026.08.14
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CHAR_DEV_NAME "/dev/char_dev_base" // 设备名

int main(void)
{
    int fd, i;
    char buf[1024] = {0};

    fd = open(CHAR_DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    // 写入字符设备缓冲区
    memset(buf, 0x01, sizeof(buf));
    ssize_t ret =  write(fd, buf, sizeof(buf));
    printf("write: ret: %d\n", ret);
    for (i = 0; i < ret; i++)
    {
        printf("%02x ", buf[i]);
    }    
    printf("\n");

    // 读取字符设备缓冲区
    memset(buf, 0, sizeof(buf));
    ret = read(fd, buf, sizeof(buf));
    printf("read: ret: %d\n", ret);
    for (i = 0; i < ret; i++)
    {
        printf("%02x ", buf[i]);
    }    
    printf("\n");

    close(fd);
    fd = -1;

    return 0;
}

