/*
    N_NZNET.c Created on: Oct 29, 2012, Zhongmin Wu <zmwu@marvell.com>

    Marvell  CCI net driver for Linux
    Copyright (C) 2010 Marvell International Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>


#include "ccinet.h"

#define N_NZNET 9

#define HEX_PACKET
static void hex_packet(unsigned char *p, int len)
{
#ifdef HEX_PACKET
	int i;
	for (i = 0; i < len; i++) {
		if (i && (i % 16) == 0)
			printk("\n");
		printk("%02X ", *p++);
	}
	printk("\n");
#endif
}
static struct tty_struct *lltty;

static int ci_ldisc_open(struct tty_struct *tty)
{
	lltty = tty;
	tty->receive_room = 65536;
	printk("**********************lltty index = %d \n", tty->index);
	return 0;
}

static void ci_ldisc_close(struct tty_struct *tty)
{
	lltty = NULL;
}

static void ci_ldisc_wake(struct tty_struct *tty)
{
	printk("ts wake\n");
}

static ssize_t ci_ldisc_read(struct tty_struct *tty, struct file *file,
					unsigned char __user *buf, size_t nr)
{
	return 0;
}

static ssize_t ci_ldisc_write(struct tty_struct *tty, struct file *file,
					const unsigned char *data, size_t count)
{
	return 0;
}

static void ci_ldisc_rx(struct tty_struct *tty, const unsigned char *data, char *flags, int size)
{
	int len = size;
	unsigned char *pkhead = data;

	int padding_size;
	char * ippacket;
	struct ccinethdr	*hdr;
	int iplen;
	
	while( len > 0)
	{
		hdr = (struct ccinethdr	*)pkhead;
		iplen = be16_to_cpu(hdr->iplen);
		ippacket = pkhead+sizeof(*hdr)+hdr->offset_len;
		//printk(KERN_DEBUG"push up data to cid %d, len %d\n", hdr->cid, iplen);  //del by jjp 0219
		//if(hdr->cid != 4)
		if ((hdr->cid < 0) || (hdr->cid >7)) //modify by jjp,20130402
		{
			printk("cid is error :%d, the whole packet len is %d\n", hdr->cid, size);
			hex_packet(data, size > 100? 100:size);
			printk("current packet len is :%d\n", iplen);
			hex_packet(hdr, (iplen + 8) > 100 ? 100:(iplen + 8)); 
		}
		data_rx(ippacket, iplen, hdr->cid);

		padding_size = rx_padding_size(sizeof(*hdr) + iplen + hdr->offset_len);
		pkhead += sizeof(*hdr) + hdr->offset_len + iplen + padding_size;
		len -= sizeof(*hdr) + hdr->offset_len + iplen + padding_size;
		if(len < 0)
		 printk("some packet is lost!\n");
	}
}

static int ci_ldisc_ioctl(struct tty_struct * tty, struct file * file,unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct tty_ldisc_ops nz_ldisc = {
	.owner		= THIS_MODULE,
	.magic		= TTY_LDISC_MAGIC,
	.name		= "net over HSI",
	.open		= ci_ldisc_open,
	.close		= ci_ldisc_close,
	.read		= ci_ldisc_read,
	.write		= ci_ldisc_write,
	.receive_buf	= ci_ldisc_rx,
	.write_wakeup	= ci_ldisc_wake,
	.ioctl 		=  ci_ldisc_ioctl,
};

void registerNZNETldisc(void)
{
	if (tty_register_ldisc(N_NZNET, &nz_ldisc))
		printk("oops. cant register ldisc\n");
}

ssize_t sendDataLowLevel(char * data, int len)
{
//	printk(KERN_DEBUG"push down data len %d\n", len);   //del by jjp
//	hex_packet(data,len);
	if(lltty)
		return	lltty->ops->write(lltty, data, len);
	else
		return -ENODEV;
}
