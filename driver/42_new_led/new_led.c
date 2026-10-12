/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	new_led.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.10.12
描  述: led驱动
备  注:
修改记录:

  1.  日期: 2026.10.12
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

#define LED_DEV_NAME "new_led"          //设备名称

#define LED_DEV_COUNT 1                 //led设备号个数

//寄存器物理地址
#define CCM_CCGR1_BASE              (0x020c406c)
#define SW_MUX_GPIO1_IO03_BASE      (0x020e0068)
#define SW_PAD_GPIO1_IO03_BASE      (0x020e02f4)
#define GPIO1_DR_BASE               (0x0209c000)
#define GPIO1_GDIR_BASE             (0x0209c004)

struct new_chr_led_dev
{
    dev_t dev_id;           //设备号
    struct cdev cdev;       //cdev
    struct class *class;    //类
    struct device *device;  //设备
    int major;              //主设备号
    int minor;              //次设备号 
};

static void __iomem *g_vir_ccm_ccgr1_base = NULL;
static void __iomem *g_vir_sw_mux_gpio1_io03_base = NULL;
static void __iomem *g_vir_sw_pad_gpio1_io03_base = NULL;
static void __iomem *g_vir_gpio1_dr_base = NULL;
static void __iomem *g_vir_gpio1_gdir_base = NULL;

struct new_chr_led_dev g_new_chr_led_dev = { 0 };

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

    //创建设备号
    if(g_new_chr_led_dev.major)
    {
        g_new_chr_led_dev.dev_id = MKDEV(g_new_chr_led_dev.major, 0);
		ret = register_chrdev_region(g_new_chr_led_dev.dev_id, LED_DEV_COUNT, LED_DEV_NAME);
    }
    else
    {   
        ret = alloc_chrdev_region(&(g_new_chr_led_dev.dev_id), 0, LED_DEV_COUNT, LED_DEV_NAME);     //申请设备号
		g_new_chr_led_dev.major = MAJOR(g_new_chr_led_dev.dev_id);
        g_new_chr_led_dev.minor = MINOR(g_new_chr_led_dev.dev_id);
    }

    if (ret < 0) {
		printk("register-chrdev failed: %d\n", ret);
		goto chrdev_region_error;
	}

    //初始化cdev
    g_new_chr_led_dev.cdev.owner = THIS_MODULE;
    cdev_init(&(g_new_chr_led_dev.cdev), &g_fops);

    //添加一个cdev
    ret = cdev_add(&(g_new_chr_led_dev.cdev), g_new_chr_led_dev.dev_id, LED_DEV_COUNT);
    if (ret < 0) {
		printk("cdev_add failed: %d\n", ret);
		goto cdev_add_error;
	}

    //创建类
    g_new_chr_led_dev.class = class_create(THIS_MODULE, LED_DEV_NAME);
    if(IS_ERR(g_new_chr_led_dev.class))
    {
        printk("class_create failed, error code = %ld\n", PTR_ERR(g_new_chr_led_dev.class));
		goto class_create_error;
    }

    //创建设备
    g_new_chr_led_dev.device = device_create(g_new_chr_led_dev.class, NULL, g_new_chr_led_dev.dev_id, NULL, LED_DEV_NAME);
    if(IS_ERR(g_new_chr_led_dev.device))
    {
        printk("device_create failed, error code = %ld\n", PTR_ERR(g_new_chr_led_dev.device));
		goto device_create_error;
    }

    printk("led init\n");

    return 0;

device_create_error:
    class_destroy(g_new_chr_led_dev.class);
class_create_error:
    cdev_del(&(g_new_chr_led_dev.cdev));
cdev_add_error:
    unregister_chrdev_region(g_new_chr_led_dev.dev_id, LED_DEV_COUNT);
chrdev_region_error:
    printk("led init error\n");

    return -EPERM;
}

static void __exit led_exit(void)
{
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

    //注销字符设备

    device_del(g_new_chr_led_dev.device);
    class_destroy(g_new_chr_led_dev.class);

    cdev_del(&(g_new_chr_led_dev.cdev));
    unregister_chrdev_region(g_new_chr_led_dev.dev_id, LED_DEV_COUNT);

    printk("led exit\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("led driver");
MODULE_VERSION("V1.0");
