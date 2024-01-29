#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

#define MYIOCTL_MAGIC 'k'
#define MYIOCTL_RESET _IO(MYIOCTL_MAGIC, 0)
#define MYIOCTL_GET_COUNT _IOR(MYIOCTL_MAGIC, 1, int)
#define MYIOCTL_INCREMENT _IOW(MYIOCTL_MAGIC, 2, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group17");
MODULE_DESCRIPTION("Simple IOCTL Example");

static int myioctl_major;
static int count = 0;

static int myioctl_open(struct inode *inode, struct file *filp);
static int myioctl_release(struct inode *inode, struct file *filp);
static long myioctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static int saveCount(const char *file_path, int counter_val)
{
    struct file *file_desc;
    loff_t position = 0;
    char buffer[256];
    int ret;

    snprintf(buffer, sizeof(buffer), "%d\n", counter_val);

    file_desc = filp_open(file_path, O_WRONLY|O_CREAT, 0644);
    if (IS_ERR(file_desc))
    {
        printk("filp_open error\n");
        return -1;
    }

    ret = kernel_write(file_desc, buffer, strlen(buffer), &position);

    filp_close(file_desc, NULL);
    return ret;
}

static int loadCount(const char *file_path, int *counter_val)
{
    struct file *file_desc;
    loff_t position = 0;
    char buffer[256];
    int ret;

    file_desc = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR(file_desc))
    {
        printk("filp_open error\n");
        return -1;
    }

    ret = kernel_read(file_desc, buffer, sizeof(buffer), &position);
    if (ret > 0) 
    {
        *counter_val = simple_strtol(buffer, NULL, 10);
    }

    filp_close(file_desc, NULL);
    return ret;
}


static const struct file_operations myioctl_fops = {
    .open = myioctl_open,
    .release = myioctl_release,
    .unlocked_ioctl = myioctl_ioctl,
};

static int __init myioctl_init(void){
    int ret;

    myioctl_major = register_chrdev(0, "myioctl", &myioctl_fops);

    if(myioctl_major < 0){
        pr_err("Failed to register character device\n");
        return myioctl_major;
    }

    ret = loadCount("/home/nachos/ioctl/counter_file.txt", &count); //remember to change the path!!!
    if (ret < 0)
    {
        pr_info("Failed to load counter value and started with zero\n");
        count = 0;
    }

    pr_info("myioctl module loaded. Major number: %d\n", myioctl_major);

    return 0;
};

static void __exit myioctl_exit(void){
    unregister_chrdev(myioctl_major, "myioctl");
    pr_info("myioctl module unloaded\n");
};

static int myioctl_open(struct inode *inode, struct file *filp){
    pr_info("myioctl device opened\n");
    return 0;
};

static int myioctl_release(struct inode *inode, struct file *filp){
    pr_info("myioctl device closeed\n");
    return 0;
};

static long myioctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    int err = 0;
    int tmp;

    if(_IOC_TYPE(cmd) != MYIOCTL_MAGIC){
        pr_err("Invalid magic number\n");
        return -ENOTTY;
    }

    switch (cmd){
        case MYIOCTL_RESET:
            pr_info("IOCTL: Resetting counter\n");
            count = 0;
            saveCount("/home/nachos/ioctl/counter_file.txt", count); //remember to change the path!!!
            break;
        case MYIOCTL_GET_COUNT:
            pr_info("IOCTL: Getting counter value\n");
            err = copy_to_user((int *)arg, &count, sizeof(int));
            break;
        case MYIOCTL_INCREMENT:
            pr_info("IOCTL: Incrementing counter\n");
            err = copy_from_user(&tmp, (int *)arg, sizeof(int));
            if(err == 0){
                count += tmp;
                saveCount("/home/nachos/ioctl/counter_file.txt", count); //remember to change the path!!!
            }
            break;
        default:
            pr_err("Unknown IOCTL command\n");
            return -ENOTTY;
    }

    return err;
};

module_init(myioctl_init);
module_exit(myioctl_exit);