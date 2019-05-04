/*****************************************************************************
��    �������ַ������������ֶ���̬�����豸�ţ��ֶ������豸�ڵ�
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
unsigned int subDevNum = 1;//Ҫ����ĵ�һ�����豸��
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
//���û���һ���ڴ�ռ�ӳ�䵽�����ں˵�һ���ڴ������Ӧ�������ڴ�����
//�������γ����ں�һ���ڴ����������ڴ�ռ䡢�û���һ���ڴ�ռ���໥ӳ���ϵ
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
//ע���豸������file_operations�ṹ��gFile

	gFile->mmap = charmap;
    gFile->owner = THIS_MODULE;
    cdev_init(gDev, gFile);
//��cdev�ṹ�������ָ��ָ��file_operations�ṹ��gFile
    cdev_add(gDev, devNum, 3);
//�����豸����cdev�ṹ����ϵ
    printk(KERN_EMERG"charDev driver initial done...\r\n");
    return 0;
}

void __exit charDrvExit(void)
{

    cdev_del(gDev);
    unregister_chrdev_region(devNum, subDevNum);
    return;
}
module_init(charDrvInit);//ִ��insmodʱ��ִ�д��д��벢����charDrvInit��������ʼ
module_exit(charDrvExit);//ִ��rmmodʱ������
MODULE_LICENSE("GPL");

