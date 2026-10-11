/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	led.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.10.11
描  述: led驱动
备  注:
修改记录:

  1.  日期: 2026.10.11
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/init.h>

#define DEV_MAJOR 200               //主设备号
#define CHAR_DEV_NAME "led"         //设备名称

//寄存器物理地址
#define CCM_CCGR1_BASE              (0x020c406c)
#define SW_MUX_GPIO1_IO03_BASE      (0x020e0068)
#define SW_PAD_GPIO1_IO03_BASE      (0x020e02f4)
#define GPIO1_DR_BASE               (0x0209c000)
#define GPIO1_GDIR_BASE             (0x0209c004)

static void __iomem *g_vir_ccm_ccgr1_base = NULL;
static void __iomem *g_vir_sw_mux_gpio1_io03_base = NULL;
static void __iomem *g_vir_sw_pad_gpio1_io03_base = NULL;
static void __iomem *g_vir_gpio1_dr_base = NULL;
static void __iomem *g_vir_gpio1_gdir_base = NULL;

static void led_switch(bool enable)
{
    unsigned int val = 0;

    val = readl(g_vir_gpio1_dr_base);

    if(enable)
    {
        val &= ~(1 << 3);
    }
    else
    {
        val |= (1 << 3);
    }

    writel(val, g_vir_gpio1_dr_base);
}

static int led_open(struct inode *inode, struct file *filep)
{
    return 0;
}   

static int led_release(struct inode *inode, struct file *filep)
{
    return 0;
}   

static ssize_t led_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char data;

    if(count <= 0)
    {
		printk("led_write: input parameter is error.\n");
        return 0;
    }

    if(copy_from_user(&data, buffer, 1))
    {
        printk("led_write copy_from_user failed\n");
        return -EFAULT;
    }

    data? led_switch(true): led_switch(false);

    return 1;    
}

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

static const struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static int __init led_init(void)
{
    int ret = 0;
    unsigned int val = 0;

    //将物理地址映射到虚拟地址
    g_vir_ccm_ccgr1_base = ioremap(CCM_CCGR1_BASE, 4);
    g_vir_sw_mux_gpio1_io03_base = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    g_vir_sw_pad_gpio1_io03_base = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    g_vir_gpio1_dr_base = ioremap(GPIO1_DR_BASE, 4);
    g_vir_gpio1_gdir_base = ioremap(GPIO1_GDIR_BASE, 4);

    //使能gpio1时钟
    val = readl(g_vir_ccm_ccgr1_base);
    val &= ~(3 << 26);      //清除以前的设置
    val |= (3 << 26);       //设置新值
    writel(val, g_vir_ccm_ccgr1_base);  

    //设置GPIO1_IO03的复用功能，将其复用为GPIO1_IO03，最后设置IO属性
    writel(5, g_vir_sw_mux_gpio1_io03_base);

    //设置IO属性
    writel(0x10b0, g_vir_sw_pad_gpio1_io03_base);

    //设置GPIO1_IO03为输出功能
    val = readl(g_vir_gpio1_gdir_base);
    val &= ~(1 << 3);       //清除以前的设置
    val |= (1 << 3);        //设置为输出
    writel(val, g_vir_gpio1_gdir_base);

    //默认关闭led
    led_switch(false);

    ret = register_chrdev(DEV_MAJOR, CHAR_DEV_NAME, &g_fops);
    if (ret < 0) {
		printk("led_init: unable to register device\n");
		return ret;
	}

    ret = kernel_system("mknod /dev/led c 200 0");
    if (ret < 0)
    {
        printk("led_init: mknod failed\n");
        return ret;
    }

    printk("led_init\n");

    return 0;
}

static void __exit led_exit(void)
{
    int ret = 0;

    iounmap(g_vir_ccm_ccgr1_base);
    iounmap(g_vir_sw_mux_gpio1_io03_base);
    iounmap(g_vir_sw_pad_gpio1_io03_base);
    iounmap(g_vir_gpio1_dr_base);
    iounmap(g_vir_gpio1_gdir_base);

    g_vir_ccm_ccgr1_base = NULL;
    g_vir_sw_mux_gpio1_io03_base = NULL;
    g_vir_sw_pad_gpio1_io03_base = NULL;
    g_vir_gpio1_dr_base = NULL;
    g_vir_gpio1_gdir_base = NULL;

    unregister_chrdev(DEV_MAJOR, CHAR_DEV_NAME);

    ret = kernel_system("rm /dev/led");
    if (ret < 0)
    {
        printk("char_dev_exit rm failed\n");
    }

    printk("led_exit\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("led driver");
MODULE_VERSION("V1.0");
