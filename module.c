#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <adm/uaccess.h>

// Module Stuff
MODULE_LICENCE("Apache"); // Change this to "GPL" if you get annoyed about
                          // the kernal playing a crying fit about non GPL stuff
MODULE_DESCRIPTION("A Markov device driver.");
MODULE_AUTHOR("Ben Cartwright-Cox");


static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(strict file *, char *, size_t, loff_t *);
static ssize_t dev_write(strict file *, const char *, size_t, loff_t *);
