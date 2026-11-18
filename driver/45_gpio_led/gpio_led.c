/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	gpio_led.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.11.18
描  述: gpio led驱动
备  注:
修改记录:

  1.  日期: 2026.11.18
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

#define GPIO_LED_COUNT  1
#define GPIO_LED_NAME   "gpio_led"

struct gpio_led_info{
    dev_t dev_id;               //设备id
    struct cdev cdev;           //cdev
    struct class* class;        //类
    struct device *device;      //设备
    int major;                  //主设备号
    int minor;                  //次设备号

    int gpio_led;               //gpio led
};

static int gpio_led_open(struct inode *node, struct file *file);
static int gpio_led_release(struct inode *node, struct file *file);
static ssize_t gpio_led_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos);

const struct file_operations g_file_ops = {
    .owner = THIS_MODULE,
    .open = gpio_led_open,
    .write = gpio_led_write,
    .release = gpio_led_release,
};

static struct gpio_led_info g_gpio_led_info;

static int gpio_led_open(struct inode *node, struct file *file)
{
    file->private_data = &g_gpio_led_info;
    return 0;
}

static int gpio_led_release(struct inode *node, struct file *file)
{
    file->private_data = NULL;
    return 0;
}

static ssize_t gpio_led_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    int ret = 0;
    char data = 0;

    struct gpio_led_info *ptgpio_led_info = (struct gpio_led_info*)file->private_data;

    if(count <= 0 || ptgpio_led_info == NULL)
    {
		printk(KERN_ERR "%s: gpio_led_write failed, count=%d, ptgpio_led_info=%p\n", __func__, count, ptgpio_led_info);
        return -EINVAL;
    }

    ret = copy_from_user(&data, buffer, 1);
    if(ret)
    {
        printk("%s: copy_from_user failed\n", __func__);
        return -EFAULT;
    }

    if(data == 1)
        gpio_set_value(ptgpio_led_info->gpio_led, 0);        //点亮led
    else if(data == 0)
        gpio_set_value(ptgpio_led_info->gpio_led, 1);        //熄灭led
    else{}

    return 1;
}

static int __init gpio_init(void)
{
    int ret = 0;
    struct device_node *np = NULL;

    g_gpio_led_info.major = 0;
    g_gpio_led_info.gpio_led = -1;

    //创建设备号
    if(g_gpio_led_info.major)
    {
        g_gpio_led_info.dev_id = MKDEV(g_gpio_led_info.major, g_gpio_led_info.minor);
        ret = register_chrdev_region(g_gpio_led_info.dev_id, GPIO_LED_COUNT, GPIO_LED_NAME);

    }
    else
    {
        ret = alloc_chrdev_region(&g_gpio_led_info.dev_id, 0, GPIO_LED_COUNT, GPIO_LED_NAME);
        g_gpio_led_info.major = MAJOR(g_gpio_led_info.dev_id);
        g_gpio_led_info.minor = MINOR(g_gpio_led_info.dev_id);
    }

    if (ret)
    {
		printk(KERN_ERR "%s: Couldn't chrdev_region, ret=%d\n", __func__, ret);
        goto chrdev_region_error;
    }

    cdev_init(&g_gpio_led_info.cdev, &g_file_ops);                                      //初始化字符设备
    g_gpio_led_info.cdev.owner = THIS_MODULE;
    ret = cdev_add(&g_gpio_led_info.cdev, g_gpio_led_info.dev_id, GPIO_LED_COUNT);      //将字符设备添加到内核
	if (ret) {
		printk(KERN_ERR "%s: Couldn't cdev_add, ret=%d\n", __func__, ret);
        goto cdev_add_error;
	}

    //创建类
    g_gpio_led_info.class = class_create(THIS_MODULE, GPIO_LED_NAME);
    if (IS_ERR(g_gpio_led_info.class)) {
		printk(KERN_ERR "%s: Couldn't class_create, ret=%d\n", __func__, ret);
		ret = PTR_ERR(g_gpio_led_info.class);
		goto class_create_error;
	}

    //创建设备
    g_gpio_led_info.device = device_create(g_gpio_led_info.class, NULL, g_gpio_led_info.dev_id, NULL, GPIO_LED_NAME);
    if (IS_ERR(g_gpio_led_info.device)) {
		printk(KERN_ERR "%s: Couldn't device_create, ret=%d\n", __func__, ret);
		ret = PTR_ERR(g_gpio_led_info.class);
		goto device_create_error;
	}

    np = of_find_compatible_node(NULL, NULL, "atkalpha-gpioled");
    if(!np)
    {
        printk(KERN_ERR "%s: failed did not find atkalpha-led node\n", __func__);
        goto of_find_compatible_node_error;
    }

    g_gpio_led_info.gpio_led = of_get_named_gpio(np, "led-gpio", 0);
    if(g_gpio_led_info.gpio_led < 0)
    {
        printk(KERN_ERR "%s: of_get_named_gpio failed\n", __func__);
        goto of_get_named_gpio_error;
    }

    of_node_put(np);        //减少设备树节点（Device Tree Node）的引用计数

    printk(KERN_INFO "gpio_led = %d\n", g_gpio_led_info.gpio_led);

    ret = gpio_request(g_gpio_led_info.gpio_led, "gpio_led");
    if(ret)
    {
        printk(KERN_ERR "gpio_request failed\n");
        goto gpio_request_error;
    }

    ret = gpio_direction_output(g_gpio_led_info.gpio_led, 0);     //设置为输出,并且输出低电平，点亮led
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
    class_destroy(g_gpio_led_info.class);
class_create_error:
    cdev_del(&g_gpio_led_info.cdev);
cdev_add_error:
    unregister_chrdev_region(g_gpio_led_info.dev_id, GPIO_LED_COUNT);
chrdev_region_error:
    printk(KERN_INFO "%s: failed", __func__);

    return ret;
}

static void __exit gpio_exit(void)
{
    if(g_gpio_led_info.gpio_led >= 0)
    {
        gpio_set_value(g_gpio_led_info.gpio_led, 1);        //熄灭led

        gpio_free(g_gpio_led_info.gpio_led);
        g_gpio_led_info.gpio_led = -1;
    }

    device_del(g_gpio_led_info.device);
    class_destroy(g_gpio_led_info.class);

    cdev_del(&g_gpio_led_info.cdev);
    unregister_chrdev_region(g_gpio_led_info.dev_id, GPIO_LED_COUNT);

    printk(KERN_INFO "%s: successed", __func__);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("this is a gpio led driver");

