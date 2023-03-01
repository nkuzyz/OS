#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define DEVICE_NAME "lkm_example"
#define EXAMPLE_MSG "Hello, 2013747 zhangyizhen!\n"
#define MSG_BUFFER_LEN 128

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static char *name = "lkm_example";
static struct file *filp;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];

module_param(name, charp, S_IRUGO);

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
    };

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *s, char *buffer, size_t len, loff_t *offset) {
    int i = 0;
    int res = 0;
    int j = 0;
    filp->f_pos = 0;
    while (1) {
        i = kernel_read(filp, msg_buffer, 128, &filp->f_pos);
        if (i == 0) {
            break;
        }
        j = 0;
        while (i > 0) {
            i--;
            put_user(msg_buffer[res++], buffer + (j++));
        }
    }
    return res;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *s, const char *buffer, size_t len,
                            loff_t *offset) {
    bool flag;
    flag=copy_from_user(msg_buffer, buffer, len);
    printk("write to file: %s\n", name);
    printk(KERN_INFO "%s\n", msg_buffer);
    filp->f_pos = file_inode(filp)->i_size;
    printk("file size: %lld\n", filp->f_pos);
    kernel_write(filp, msg_buffer, len, &filp->f_pos);
    return len;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
    /* If device is open, return busy */
    if (device_open_count) {
        return -EBUSY;
    }
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
    /* Decrement the open counter and usage count. Without this, the module wouldnot unload. */
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static int __init lkm_example_init(void) {
    // 打开文件
    filp = filp_open(name, O_RDWR, 0);
    printk("open file: %s", name);
    /* Try to register character device */
    major_num = register_chrdev(0, "lkm_example", &file_ops);
    if (major_num < 0) {
        printk(KERN_ALERT "Could not register device: %d\n", major_num);
        return major_num;
    } else {
        printk(KERN_INFO "lkm_example module loaded with device major number %d\n",
               major_num);
        return 0;
    }
}
static void __exit lkm_example_exit(void) {
    /* Remember — we have to clean up after ourselves. Unregister the character
    device. */
    unregister_chrdev(major_num, DEVICE_NAME);
    filp_close(filp, NULL);
    printk(KERN_INFO "Goodbye, 2013747 zhangyizhen!\n");
}
/* Register module functions */
module_init(lkm_example_init);
module_exit(lkm_example_exit);
