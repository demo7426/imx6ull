/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	led_app.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.10.11
描  述: 字符设备驱动应用
备  注:
修改记录:

  1.  日期: 2026.10.11  
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CHAR_DEV_NAME "/dev/led"         //设备名称

int main(int argc, char* argv[])
{
    int ret = 0;
    int fd = 0;
    char cmd = 0;

    if(argc < 2)
    {
        printf("Please input:\n \txxx.elf cmd\n\teg: xxx.elf 1\n");
        return -1;
    }

    cmd = atoi(argv[1]);

    fd = open(CHAR_DEV_NAME, O_RDWR);
    if(fd < 0)
    {
        printf("open is failed\n");
        return -1;
    }

    write(fd, &cmd, sizeof cmd);

    close(fd);
    fd = 0;

    return 0;
}

