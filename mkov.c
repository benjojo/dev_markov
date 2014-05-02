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

static int rollingLimit = 0;

static struct MKovEnt words[1024] = {};


int init_module(void) {
    int t = register_chrdev(89,"mkov",&fops);
    if(t<0) {
        printk(KERN_ALERT "Markov module could not initalize.");
    } else {
        printk(KERN_ALERT "Markov module initalized.");
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
static int wordsize = 0;

static char lastwordread[20]={0};

static ssize_t dev_read(struct file *foole,char *buff,size_t len,loff_t *off) {
    int i = 0;
    int j = 0;
    int matches = 0;
    int matchlist[1024]={0};

    if(lastwordread[0] == 0x00) {
        printk(KERN_ALERT "[Read] I've got nothing to read. Scanning for words in the table");
        // Nothing to read?
        // Lets pick the first thing in the mkov list.
        for (j = 0; j < 1024; ++j) {
            if(words[j].word[0] == 0x00) {

            } else {
                // Copy that into the lastwordread array.
                printk(KERN_ALERT "[Read] I will use %s as my starting point.", words[j].word);
                for (i = 0; i < 19; ++i) {
                    lastwordread[i] = words[j].word[i];
                }
                break;
            }
        }
    }


    if(lastwordread[0] == 0x00) {
        printk(KERN_ALERT "[Read] There is nothing to start from. Not going to give anything.");
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
        for (j = 0; j < 19; ++j) {
            if (words[i].lastword[j] != lastwordread[j]) {
                ismatch = 0;
            }
        }
        if(ismatch) {
            int isrepeat = 1;
            for (j = 0; j < 19; ++j) {
                if (words[i].word[j] != lastwordread[j]) {
                    isrepeat = 0;
                }
            }

            if(isrepeat == 0) {
                matchlist[matches] = i;
                matches++;
            } else {
                printk(KERN_ALERT "[Read] Something happened %s -> %s",words[i].word,lastwordread);
            }
        }

    }


    if(matches == 0) {
        printk(KERN_ALERT "[Read] No matches to word found. abort.");
        // So I am now going to copy a random work (provided there is one in there) to the 
        // lastwordread array. to spice things up a tad.
        int pickR = get_jiffies_64();

        for (i = 0; i < 19; ++i) {
            lastwordread[i] = 0x00;
        }

        int attempts = 0;

        while(lastwordread[0] == 0x00) {
            int pick = pickR % 1023;
            printk(KERN_ALERT "[Read] Pick %d",pick);
            if(words[pick].word[0] != 0x00) {
                for (i = 0; i < 19; ++i) {
                    lastwordread[i] = 0x00;
                }
                printk(KERN_ALERT "[Read] Why not use %s for the next word? Pick %d",words[pick].word,pick);
                for (i = 0; i < 19; ++i) {
                    lastwordread[i] = words[pick].word[i];
                }
                
            }
            pickR++;
            attempts++;
            if(attempts == 1024) {
                printk(KERN_ALERT "[Read] There is literally nothing we can use");
                break;
            }
        }


        return 0;
    }

    int totalprobcount = 0;

    for (i = 0; i < matches; ++i) {
        totalprobcount += words[matchlist[i]].times;
    }

    int target = get_jiffies_64() % totalprobcount; // Good lord what have I done.
    short count = 0;
    for (i = 0; i < matches; ++i) {
        target = target - words[matchlist[i]].times;

        if(target < 0) {
            // WE HAVE GOT IT LADIES AND GENTLEMEN.
            for (j = 0; j < 19; ++j) {
                lastwordread[j] = 0x00;
            }

            while (len && (words[matchlist[i]].word[readPos]!=0))
            {
                put_user(words[matchlist[i]].word[readPos],buff++); //copy byte from kernel space to user space
                lastwordread[readPos] = words[matchlist[i]].word[readPos];
                count++;
                len--;
                readPos++;
            }
            break;
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
            if(wordsize == 0) {
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
                        if (words[i].word[j] != msg[j]) {
                            correctwordmaybe = 0;
                        }
                        if (words[i].lastword[j] != lastword[j]) {
                            correctwordmaybe = 0;
                        }
                    }
                    if(correctwordmaybe) {
                        printk(KERN_ALERT "Found that word again... %s->%s",lastword, words[i].word);
                        // If it is then increment it.
                        words[i].times++;
                        foundit = 1;
                        break;
                    }
                }
                if(foundit == 0) { 
                    rollingLimit++;
                    if(rollingLimit == 1024) {
                        rollingLimit = 0;
                    }

                    int i;
                    for (i = 0; i < 19; ++i) {
                        words[rollingLimit].word[i] = msg[i];
                    }
                    words[rollingLimit].wordlen = wordsize;
                    for (i = 0; i < 19; ++i) {
                        words[rollingLimit].lastword[i] = lastword[i];
                    }
                    words[rollingLimit].lastwordlen = lastwordsize;

                    printk(KERN_ALERT "Added a new word. %s->%s",lastword,msg);
                    words[rollingLimit].times = 1;
                }
                // If not add it
                wordsize = 0;
            }

            // Then set the latest word var
            int i;
            for (i = 0; i < 19; ++i) {
                lastword[i] = msg[i];
            }
            lastwordsize = wordsize;
            for (i = 0; i < 19; ++i) {
                msg[i] = 0x00;
            }
        } else {
            if(wordsize != 20) {
                msg[wordsize] = buff[index];
                wordsize++;
            }
        }
    }

    return bytesprocessed;

}

static int dev_rls(struct inode *inod,struct file *fil) {
    return 0;
}
