CROSS_COMPILE ?= arm-linux-gnueabihf-
ARCH ?= arm

src_c_name ?= led_app

ifneq ($(KERNELRELEASE),)
	obj-m := led.o
else
	KERNELDIR ?= /home/qianrui/IMX6ULL/SystemPorting/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
	PWD := $(shell pwd)

.PHONY:clean

default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules
	$(CROSS_COMPILE)gcc $(src_c_name).c -o $(src_c_name).elf

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) -rf $(src_c_name).elf

endif
