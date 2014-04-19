#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// Module Stuff
MODULE_LICENSE("Apache"); // Change this to "GPL" if you get annoyed about
                          // the kernal playing a crying fit about non GPL stuff
MODULE_DESCRIPTION("A Markov device driver.");
MODULE_AUTHOR("Ben Cartwright-Cox");


static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

// Adding callbacks into the gig
static struct file_operations fops =
{
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_rls,
};

int init_module(void) {
    int t = register_chrdev(69,"mkov",&fops);
    if(t<0) {
        printk(KERN_ALERT "MKOV MODULE COULD NOT INIT AAAAAAAAA");
    } else {
        printk(KERN_ALERT "mkov module init'd. Insanity loaded into kernel. Good job hero.");
    }

    return t;
}

void cleanup_module(void) {
    unregister_chrdev(69,"mkov");
}