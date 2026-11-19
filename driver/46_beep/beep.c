/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	beep.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.11.19
描  述: 蜂鸣器驱动
备  注:
修改记录:

  1.  日期: 2026.11.19
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>

#define BEEP_COUNT  1
#define BEEP_NAME   "beep"

struct beep_info{
    dev_t dev_id;               //设备id
    struct cdev cdev;           //cdev
    struct class* class;        //类
    struct device *device;      //设备
    int major;                  //主设备号
    int minor;                  //次设备号

    int beep;                   //gpio beep
};

static int beep_open(struct inode *node, struct file *file);
static int beep_release(struct inode *node, struct file *file);
static ssize_t beep_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos);

const struct file_operations g_file_ops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
    .release = beep_release,
};

static struct beep_info g_beep_info;

static int beep_open(struct inode *node, struct file *file)
{
    file->private_data = &g_beep_info;
    return 0;
}

static int beep_release(struct inode *node, struct file *file)
{
    file->private_data = NULL;
    return 0;
}

static ssize_t beep_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    int ret = 0;
    char data = 0;

    struct beep_info *ptbeep_info = (struct beep_info*)file->private_data;

    if(count <= 0 || ptbeep_info == NULL)
    {
		printk(KERN_ERR "%s: beep_write failed, count=%d, ptbeep_info=%p\n", __func__, count, ptbeep_info);
        return -EINVAL;
    }

    ret = copy_from_user(&data, buffer, 1);
    if(ret)
    {
        printk("%s: copy_from_user failed\n", __func__);
        return -EFAULT;
    }

    if(data == 1)
        gpio_set_value(ptbeep_info->beep, 0);        //打开蜂鸣器
    else if(data == 0)
        gpio_set_value(ptbeep_info->beep, 1);        //关闭蜂鸣器
    else{}

    return 1;
}

static int __init gpio_init(void)
{
    int ret = 0;
    struct device_node *np = NULL;

    g_beep_info.major = 0;
    g_beep_info.beep = -1;

    //创建设备号
    if(g_beep_info.major)
    {
        g_beep_info.dev_id = MKDEV(g_beep_info.major, g_beep_info.minor);
        ret = register_chrdev_region(g_beep_info.dev_id, BEEP_COUNT, BEEP_NAME);

    }
    else
    {
        ret = alloc_chrdev_region(&g_beep_info.dev_id, 0, BEEP_COUNT, BEEP_NAME);
        g_beep_info.major = MAJOR(g_beep_info.dev_id);
        g_beep_info.minor = MINOR(g_beep_info.dev_id);
    }

    if (ret)
    {
		printk(KERN_ERR "%s: Couldn't chrdev_region, ret=%d\n", __func__, ret);
        goto chrdev_region_error;
    }

    cdev_init(&g_beep_info.cdev, &g_file_ops);                                      //初始化字符设备
    g_beep_info.cdev.owner = THIS_MODULE;
    ret = cdev_add(&g_beep_info.cdev, g_beep_info.dev_id, BEEP_COUNT);      //将字符设备添加到内核
	if (ret) {
		printk(KERN_ERR "%s: Couldn't cdev_add, ret=%d\n", __func__, ret);
        goto cdev_add_error;
	}

    //创建类
    g_beep_info.class = class_create(THIS_MODULE, BEEP_NAME);
    if (IS_ERR(g_beep_info.class)) {
		printk(KERN_ERR "%s: Couldn't class_create, ret=%d\n", __func__, ret);
		ret = PTR_ERR(g_beep_info.class);
		goto class_create_error;
	}

    //创建设备
    g_beep_info.device = device_create(g_beep_info.class, NULL, g_beep_info.dev_id, NULL, BEEP_NAME);
    if (IS_ERR(g_beep_info.device)) {
		printk(KERN_ERR "%s: Couldn't device_create, ret=%d\n", __func__, ret);
		ret = PTR_ERR(g_beep_info.class);
		goto device_create_error;
	}

    np = of_find_compatible_node(NULL, NULL, "atkalpha-beep");
    if(!np)
    {
        printk(KERN_ERR "%s: failed did not find atkalpha-beep node\n", __func__);
        goto of_find_compatible_node_error;
    }

    g_beep_info.beep = of_get_named_gpio(np, "beep-gpio", 0);
    if(g_beep_info.beep < 0)
    {
        printk(KERN_ERR "%s: of_get_named_gpio failed\n", __func__);
        goto of_get_named_gpio_error;
    }

    of_node_put(np);        //减少设备树节点（Device Tree Node）的引用计数

    printk(KERN_INFO "beep = %d\n", g_beep_info.beep);

    ret = gpio_request(g_beep_info.beep, "beep");
    if(ret)
    {
        printk(KERN_ERR "gpio_request failed\n");
        goto gpio_request_error;
    }

    ret = gpio_direction_output(g_beep_info.beep, 1);     //设置为输出,并且输出高电平，关闭蜂鸣器
    if(ret)
    {
        printk(KERN_ERR "gpio_direction_output failed\n");
        goto gpio_request_error;
    }

    printk(KERN_INFO "%s: successed\n", __func__);

    return 0;

//异常处理
gpio_request_error:
of_get_named_gpio_error:
    of_node_put(np);        //减少设备树节点（Device Tree Node）的引用计数
of_find_compatible_node_error:

device_create_error:
    class_destroy(g_beep_info.class);
class_create_error:
    cdev_del(&g_beep_info.cdev);
cdev_add_error:
    unregister_chrdev_region(g_beep_info.dev_id, BEEP_COUNT);
chrdev_region_error:
    printk(KERN_INFO "%s: failed\n", __func__);

    return ret;
}

static void __exit gpio_exit(void)
{
    if(g_beep_info.beep >= 0)
    {
        gpio_set_value(g_beep_info.beep, 1);        //关闭蜂鸣器

        gpio_free(g_beep_info.beep);
        g_beep_info.beep = -1;
    }

    device_del(g_beep_info.device);
    class_destroy(g_beep_info.class);

    cdev_del(&g_beep_info.cdev);
    unregister_chrdev_region(g_beep_info.dev_id, BEEP_COUNT);

    printk(KERN_INFO "%s: successed", __func__);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("this is a gpio led driver");

