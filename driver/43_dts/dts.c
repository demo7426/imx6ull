/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	dts.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.10.31
描  述: 设备树解析示例
备  注:
修改记录:

  1.  日期: 2026.10.31
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

/*
	backlight {
		compatible = "pwm-backlight";
		pwms = <&pwm1 0 5000000>;
		brightness-levels = <0 4 8 16 32 64 128 255>;
		default-brightness-level = <6>;
		status = "okay";
	};
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>

static int __init driver_init(void)
{
    struct device_node * np = NULL;
    int count = 0;
    u32 vals[32];
    u32 val_size = sizeof(vals) / sizeof(vals[0]);
    s32 ret = 0, i = 0;
    const char* status;
    struct of_phandle_args out_args;

    //查找节点
    np = of_find_compatible_node(NULL, NULL, "pwm-backlight");
    if(!np)
    {
        printk(KERN_ERR "failed did not find pwm-backlight node\n");
		return -ENODEV;
    }

    //解析pwms信息
    ret = of_parse_phandle_with_args(np, "pwms", "#pwm-cells", 0, &out_args);
    if (ret < 0) {
        printk(KERN_ERR "of_parse_phandle_with_args pwms failed\n");
		return -EINVAL;
	} 
    
    printk(KERN_CONT "%s ", out_args.np->name);
    for (i = 0; i < out_args.args_count; i++)
    {
        printk(KERN_CONT "%u ", out_args.args[i]);    
    }

    of_node_put(out_args.np);

    //读取brightness-levels信息
    count = of_property_count_elems_of_size(np, "brightness-levels", sizeof(u32));
    printk(KERN_INFO "brightness-levels count = %d\n", count);

    ret = of_property_read_u32_array(np, "brightness-levels", vals, count > val_size ? val_size: count);
    if(ret != 0)
    {
        printk(KERN_ERR "of_property_read_u32_array brightness-levels failed\n");
		return -EINVAL;
    }

    printk(KERN_CONT "brightness-levels = ");
    for (i = 0; i < (count > val_size ? val_size: count); i++)
    {
        printk(KERN_CONT "%u ", vals[i]);
    }
    printk(KERN_INFO "");
    
    //读取default-brightness-level信息
    ret = of_property_read_u32(np, "default-brightness-level", vals);
    if(ret != 0)
    {
        printk(KERN_ERR "of_property_read_u32 brightness-levels failed\n");
		return -EINVAL;
    }
    printk(KERN_INFO "default-brightness-level = %d\n", vals[0]);

    //读取status信息
    ret = of_property_read_string(np, "status", &status);
    if(ret != 0)
    {
        printk(KERN_ERR "of_property_read_string status failed\n");
		return -EINVAL;
    }
    printk(KERN_INFO "status = %s\n", status);

    of_node_put(np);        //减少设备树节点（Device Tree Node）的引用计数

    return 0;
}

static void __exit driver_exit(void)
{

}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qianrui");
MODULE_DESCRIPTION("this is a dts demo.");

