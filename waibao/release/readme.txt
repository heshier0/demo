内容说明
mtt_rec_sdk10       双目录像部分源码
jx_f23                    荣品sdk的 jx_f32 sensor源码，放到                             Hi3516CV500_SDK_V2.0.1.0/smp/a7_linux/mpp/component/isp/user/sensor/hi3516cv500/
                             下编译
kernel                   荣品sdk目录下的内核配置.config 支持wifi
wifi                       基于荣品sdk编译的wifi相关二进制
                             rtl8192eu.ko 驱动
                             wpa_supplicant/wpa_cli/wpa_passphrase  wifi有关的开源工具,使用方法可以百度
                             busybox 支持udhcpc的busysbox, 不替换原来busybox的方法 ./busybox udhcpc -i wlan0 分配内存
                             wpa_supplicant.conf wifi配置文件, 增加自己的用户密码放在这个文件里

uImage                 支持wifi的内核，（不影响麦淘淘原有文件系统内容使用）

gpio                      gpio_pinmux.sh (目前所用的管脚复用，所用gpio见脚本注释)
 
                             用命令行测试
                             echo gpio号 > /sys/class/gpio/export
                             
                             //输入
                             echo in > /sys/class/gpio/gpio[号]/direction
                             
                              //读输入状态
                              cat /sys/class/gpio/gpio[号]/value

                             //输出
                             echo out > /sys/class/gpio/gpio[号]/direction

                            //设置高
                            echo 1 > /sys/class/gpio/gpio[号]/value

                            //设置低
                            echo 0 > /sys/class/gpio/gpio[号]/value


GPIO使用代码, 依赖于libgpiod.a

#include <gpiod.h>

//使用代码初始化gpio

static struct gpiod_chip * chip0 = NULL;
static struct gpiod_chip * chip2 = NULL;
static struct gpiod_chip * chip9 = NULL;
static struct gpiod_chip * chip10 = NULL;

static struct gpiod_line * input_line_82  =  NULL;
static struct gpiod_line * input_line_84  =  NULL;

static struct gpiod_line * output_line_2  =  NULL;
static struct gpiod_line * output_line_5  =  NULL;
static struct gpiod_line * output_line_20 =  NULL;
static struct gpiod_line * output_line_21 =  NULL;
static struct gpiod_line * output_line_22 =  NULL;
static struct gpiod_line * output_line_23 =  NULL;
static struct gpiod_line * output_line_74 =  NULL;
static struct gpiod_line * output_line_78 =  NULL;
static struct gpiod_line * output_line_81 =  NULL;

int gpio_init()
{
	input_chip0  = gpiod_chip_open_by_name("gpiochip0");
	input_chip2  = gpiod_chip_open_by_name("gpiochip2");
	input_chip9  = gpiod_chip_open_by_name("gpiochip9");
	input_chip10 = gpiod_chip_open_by_name("gpiochip10");
	
	input_line_82 = gpiod_chip_get_line(chip9, 1);
	input_line_84 = gpiod_chip_get_line(chip9, 3);
	
	output_line_2  = gpiod_chip_get_line(chip0, 2);
	output_line_5  = gpiod_chip_get_line(chip0, 5);
	output_line_20 = gpiod_chip_get_line(chip2, 4);
	output_line_21 = gpiod_chip_get_line(chip2, 5);	
	output_line_22 = gpiod_chip_get_line(chip2, 6);
	output_line_23 = gpiod_chip_get_line(chip2, 7);
	output_line_74 = gpiod_chip_get_line(chip9, 2);
	output_line_78 = gpiod_chip_get_line(chip9, 4);
	output_line_81 = gpiod_chip_get_line(chip10, 1);
	

               return 0;
}


//使用代码设置GPIO状态
int gpio_led(struct gpiod_line * line, int value)
{
	int ret;
	
	gpiod_line_set_value(line, value);
	
	return 0;
}

//使用代码获取GPIO状态
int gpio_getvalue(struct gpiod_line * line)
{
     return	gpiod_line_get_value(line);
}

                             




                            


