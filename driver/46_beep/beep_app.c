/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	beep_app.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.11.19
描  述: 蜂鸣器应用程序
备  注:
修改记录:

  1.  日期: 2026.11.19
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define GPIO_LED_NAME   "/dev/beep"

int main(int argc, char* argv[])
{
    int ret = 0;
    int fd = -1;
    int data = 0;

    if(argc < 2)
    {
        printf("Please input:\n \txxx.elf cmd\n\teg: xxx.elf 1\n");
        return -1;
    }

    data = atoi(argv[1]);

    fd = open(GPIO_LED_NAME, O_RDWR);
    if(fd < 0)
    {
        printf("%s: open is failed\n", __func__);
        return -1;
    }

    if(write(fd, &data, 1) != 1)
    {
        printf("%s: write is failed\n", __func__);
    }

    close(fd);
    fd = -1;

    return 0;
}

