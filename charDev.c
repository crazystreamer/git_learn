/*****************************************************************************
简    述：简单字符型驱动程序，手动静态分配设备号，手动创建设备节点
******************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/interrupt.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/atomic.h>

#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <linux/slab.h>
#define DEVICE_NAME		"charDev"
#define MAX_MMAP_BUFFER (9184)
#define ERROR (-1)
#define OK (0)

struct cdev *gDev;
struct file_operations *gFile;
dev_t  devNum;
unsigned int subDevNum = 1;//要申请的第一个次设备号
int reg_major  =  232;    
int reg_minor =   0;
int *date;


int charOpen(struct inode *p, struct file *f)
{
	int i; 
    struct page *mypage;
	printk(KERN_EMERG"charOpen\n");
	date = kzalloc(MAX_MMAP_BUFFER,GFP_KERNEL);
	for(i =0 ;i<MAX_MMAP_BUFFER;i++)
	{
		date[i] = i%255;	
	}
	for(i = 0 ; i<2;i++)
	{ 
		mypage = virt_to_page((void *)(date+i*PAGE_SIZE));
		SetPageReserved(mypage);
			
	}
	return 0;
}
//将用户的一块内存空间映射到驱动内核的一块内存区域对应的物理内存区域
//这样便形成了内核一块内存区域、物理内存空间、用户的一块内存空间的相互映射关系
int charmap(struct file *fil, struct vm_area_struct * vma)
{
	unsigned long phys,len;
	unsigned long pfn;
	phys = virt_to_phys((void *)date);
	len = vma->vm_end-vma->vm_start;
	pfn =phys>>PAGE_SHIFT;
	if(remap_pfn_range(vma,vma->vm_start,pfn, len,vma->vm_page_prot))
	{
		printk("remap error\n");
		return -1;
	}
	return 0;
		
}
int charDrvInit(void)
{
    
    devNum = MKDEV(reg_major, reg_minor);

    printk(KERN_EMERG"devNum is %d\r\n", devNum);
	if(OK == register_chrdev_region(devNum, subDevNum, DEVICE_NAME))
	{
        printk(KERN_EMERG"register_chrdev_region ok\r\n");
	}
    else
	{
        printk(KERN_EMERG"register_chrdev_region error\r\n");
        return ERROR;
	}
	
    gDev = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    gFile = kzalloc(sizeof(struct file_operations), GFP_KERNEL);
	
    gFile->open = charOpen;
//注册设备函数到file_operations结构体gFile

	gFile->mmap = charmap;
    gFile->owner = THIS_MODULE;
    cdev_init(gDev, gFile);
//在cdev结构体中添加指针指向file_operations结构体gFile
    cdev_add(gDev, devNum, 3);
//建立设备号与cdev结构体联系
    printk(KERN_EMERG"charDev driver initial done...\r\n");
    return 0;
}

void __exit charDrvExit(void)
{

    cdev_del(gDev);
    unregister_chrdev_region(devNum, subDevNum);
    return;
}
module_init(charDrvInit);//执行insmod时会执行此行代码并调用charDrvInit，驱动开始
module_exit(charDrvExit);//执行rmmod时，结束
MODULE_LICENSE("GPL");

