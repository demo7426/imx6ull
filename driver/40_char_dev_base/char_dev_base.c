/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	char_dev.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.14
描  述: 字符设备驱动
备  注:
修改记录:

  1.  日期: 2026.08.14
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define CHAR_DEV_MAJOR 200  // 主设备号
#define CHAR_DEV_NAME "char_dev_base" // 设备名

static char g_char_dev_buf[8096] = {0}; // 字符设备缓冲区

/**
 * kernel_system - 内核中执行 shell 命令，等价于用户空间 system()
 * @cmd: 要执行的命令字符串
 * 返回: 进程退出码，负值表示失败
 */
static int kernel_system(const char *cmd)
{
    int ret;
    char *argv[] = {
        "/bin/sh",
        "-c",
        (char *)cmd,
        NULL
    };
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
        NULL
    };

    /* 等待子进程完全结束并获取退出码 */
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    return ret;
}

static int char_dev_open(struct inode *inode, struct file *filep)
{
    return 0;
}

static ssize_t char_dev_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    if (count > sizeof(g_char_dev_buf))
    {
        return 0;
    }

    ret = copy_to_user(buf, g_char_dev_buf, count);
    if (ret)
    {
        printk("char_dev_read copy_to_user failed\n");
        return -EFAULT;
    }

    return count;
}

static ssize_t char_dev_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    if (count > sizeof(g_char_dev_buf))
    {
        return 0;
    }

    ret = copy_from_user(g_char_dev_buf, buf, count);
    if (ret)
    {
        printk("char_dev_write copy_from_user failed\n");
        return -EFAULT;
    }

    return count;
}

static int char_dev_release(struct inode *inode, struct file *filep)
{
    return 0;
}

static struct file_operations char_dev_fops = {
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_release,
    .read = char_dev_read,
    .write = char_dev_write,
};


static int __init char_dev_init(void)
{
    int ret;

    ret = register_chrdev(CHAR_DEV_MAJOR, CHAR_DEV_NAME, &char_dev_fops);
    if (ret < 0)
    {
        printk("char_dev_init register_chrdev failed\n");
        return ret;
    }

    ret = kernel_system("mknod /dev/char_dev_base c 200 0");
    if (ret < 0)
    {
        printk("char_dev_init mknod failed\n");
        return ret;
    }

    printk("char_dev_init\n");
    return 0;
}

static void __exit char_dev_exit(void)
{
    unregister_chrdev(CHAR_DEV_MAJOR, CHAR_DEV_NAME);

    int ret = kernel_system("rm /dev/char_dev_base");
    if (ret < 0)
    {
        printk("char_dev_exit rm failed\n");
    }

    printk("char_dev_exit\n");
}

module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("char_dev driver");
