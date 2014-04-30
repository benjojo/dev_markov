#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>

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

static struct MKovEnt
{
    int times;
    char word[20];
    int wordlen;
    char lastword[20];
    int lastwordlen;
};

static int RollingLimit = 0;

static struct MKovEnt Words[1024] = {};
static struct MKovEnt OptionsTable[1024] = {};
static int OptionsTableLimit = 0;

int init_module(void) {
    int t = register_chrdev(89,"mkov",&fops);
    if(t<0) {
        printk(KERN_ALERT "MKOV MODULE COULD NOT INIT AAAAAAAAA");
    } else {
        printk(KERN_ALERT "mkov module init'd. Insanity loaded into kernel. Good job hero.");
    }

    return t;
}

void cleanup_module(void) {
    unregister_chrdev(89,"mkov");
}

static int dev_open(struct inode *inod,struct file *fil) {
    return 0; // Who actually cares?!
}

static char msg[20]={0};
static short readPos=0;

static char lastword[20]={0};
static int lastwordsize = 0;
static int WordSize = 0;

static int DebugReadPoint = 0;


static char lastwordread[20]={0};

static ssize_t dev_read(struct file *foole,char *buff,size_t len,loff_t *off) {
    int i;
    int j;
    int matches;
    int matchlist[1024]={0};

    if(lastwordread[0] == 0x00) {
        printk(KERN_ALERT "[Read] I've got nothing to read. Scanning for words in the table");
        // Nothing to read?
        // Lets pick the first thing in the mkov list.
        for (j = 0; j < 1024; ++j) {
            if(Words[j].word[0] == 0x00) {

            } else {
                // Copy that into the lastwordread array.
                printk(KERN_ALERT "[Read] There is stuff in there, I will use that as my starting point.");
                for (i = 0; i < 19; ++i) {
                    lastwordread[i] = Words[j].word[i];
                }
                break;
            }
        }

    }
    if(lastwordread[0] == 0x00) {
        printk(KERN_ALERT "[Read] Nothing. I'm going to bugger off then.");
        // Okay so this means there has been literally nothing entered in yet.
        // thus there is nothing to give to the user.
        return 0;
    }
    // Right so at this point we can assume that we have somthing to base our prev knowlage off.
    // So we are going to build a options table and then use get_jiffies_64() to pick one.

    for (i = 0; i < 1024; ++i) {
        // Now we are going to scan the words table to see how many
        // and copy the matches into the table where we will pick the winner.
        int ismatch = 1;
        for (j = 0; i < 20; ++i) {
            if (Words[i].lastword[j] != lastword[j]) {
                ismatch == 0;
            }
        }
        if(ismatch) {
            // oh neat this one could work for us!
            matchlist[matches] = i;
            matches++;
        }
    }


    if(matches == 0) {
        printk(KERN_ALERT "[Read] No matches to word found. abort.");
        return 0;
    }

    int totalprobcount = 0;

    for (i = 0; i < matches; ++i) {
        totalprobcount += Words[matchlist[i]].times;
    }
    
    printk(KERN_ALERT "[Read] So explode maybe?");
    int target = get_jiffies_64() % totalprobcount; // Good lord what have I done.
    printk(KERN_ALERT "[Read] Na not that.");

    short count = 0;
    for (i = 0; i < matches; ++i) {
        target = target - Words[matchlist[i]].times;
        if(target < 0) {
            // WE HAVE GOT IT LADIES AND GENTLEMEN.
            while (len && (Words[i].word[readPos]!=0))
            {
                put_user(Words[i].word[readPos],buff++); //copy byte from kernel space to user space
                count++;
                len--;
                readPos++;
            }
        }
    }

    if(count != 0) {
        put_user(0x20,buff++); // " "
    }
    readPos = 0;
    return count+1;
}

static ssize_t dev_write(struct file *foole,const char *buff,size_t len,loff_t *off) {
    short bytesprocessed = 0;
    int index;
    for (index = 0; index < len; ++index) {
        bytesprocessed++;
        // what you want is in buff[index]
        char letter = buff[index];
        if(letter == 0x2E || letter == 0x20 || letter == 0x2C || letter == 0x0D || letter == 0x0A ) {
            // Check how much is in the word buffer
            if(WordSize == 0) {
                // Then this is useless
                continue;
            } else {
                int i; // C99 mode
                int j; // C99 mode
            // Then check if the word is in the system already
                int foundit = 0;
                for (i = 0; i < 1023; ++i) {
                    int correctwordmaybe = 1;
                    for (j = 0; j < 19; ++j) {
                        if (Words[i].word[j] != msg[j]) {
                            correctwordmaybe = 0;
                        }
                    }
                    if(correctwordmaybe) {
                        printk(KERN_ALERT "Found that word again... %s", Words[i].word);
                        // If it is then increment it.
                        Words[i].times++;
                        foundit = 1;
                        break;
                    }
                }
                if(foundit == 0) {
                    RollingLimit++;
                    if(RollingLimit == 1024) {
                        RollingLimit = 0;
                    }

                    int i;
                    for (i = 0; i < 19; ++i) {
                        Words[RollingLimit].word[i] = msg[i];
                    }
                    Words[RollingLimit].wordlen = WordSize;
                    for (i = 0; i < 19; ++i) {
                        Words[RollingLimit].lastword[i] = lastword[i];
                    }
                    Words[RollingLimit].lastwordlen = lastwordsize;

                    printk(KERN_ALERT "Added a new word. %s",msg);
                    Words[RollingLimit].times = 0;
                }
                // If not add it
                WordSize = 0;
            }

            // Then set the latest word var
            int i;
            for (i = 0; i < 19; ++i) {
                lastword[i] = msg[i];
            }
            lastwordsize = WordSize;
            for (i = 0; i < 19; ++i) {
                msg[i] = 0x00;
            }
        } else {
            if(WordSize != 20) {
                msg[WordSize] = buff[index];
                WordSize++;
            } else {
                printk(KERN_ALERT "Word 2 big 4 me");
                // WordSize = 0;
            }
        }
    }

    return bytesprocessed;

}

static int dev_rls(struct inode *inod,struct file *fil) {
    return 0;
}
