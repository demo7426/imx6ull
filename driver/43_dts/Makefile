CROSS_COMPILE ?= arm-linux-gnueabihf-
ARCH ?= arm

ifneq ($(KERNELRELEASE),)
	obj-m := dts.o
else
	KERNELDIR ?= /home/qianrui/IMX6ULL/SystemPorting/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
	PWD := $(shell pwd)

.PHONY:clean

default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

endif
