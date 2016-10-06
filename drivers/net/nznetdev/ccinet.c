/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * (C) Copyright 2006 Marvell International Ltd.
 * All Rights Reserved
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

//#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>

#include "ccinet.h"

#define MAX_CID_NUM    8

#define	INTFACE_NAME "ccinetnz"



//#define STATUS_TIMER_ENABLE 1   // add by jjp for test 0225
struct ccinet_priv {
	unsigned char cid;
	int status;                                             /* indicate status of the interrupt */
	spinlock_t lock;                                        /* spinlock use to protect critical session	*/
	struct  net_device_stats net_stats;                     /* status of the network device	*/
	#ifdef STATUS_TIMER_ENABLE
	int timer_enable;                                         // add by jjp for test 0225
  struct  timer_list   ccinet_timer;                         // add by jjp for test 0225
  #endif
};
static struct net_device *net_devices[MAX_CID_NUM];


struct buff_list{
	struct sk_buff_head buff_head;
	spinlock_t buff_lock;
};

static struct buff_list tx_list;

static struct work_struct tx_worker;
static struct workqueue_struct * ccinet_work_queue;

#ifdef DATA_IND_BUFFERLIST
struct ccirx_packet {
	char  *pktdata;
	int length;
	unsigned char cid;
};

typedef struct _cci_rxind_buf_list
{
	struct ccirx_packet rx_packet;
	struct _cci_rxind_buf_list *next;
}RXINDBUFNODE;

//static RXINDBUFNODE *rxbuflist = NULL;
struct completion dataChanRxFlagRef;
#define rxDataMask 0x0010
struct task_struct *cinetDataRcvTaskRef;
#endif


#if  1
#define DPRINT(fmt, args ...)     printk(fmt, ## args)
#define DBGMSG(fmt, args ...)     printk(KERN_DEBUG "CIN: " fmt, ## args)
#define ENTER()                 printk(KERN_DEBUG "CIN: ENTER %s\n", __FUNCTION__)
#define LEAVE()                 printk(KERN_DEBUG "CIN: LEAVE %s\n", __FUNCTION__)
#define FUNC_EXIT()                     printk(KERN_DEBUG "CIN: EXIT %s\n", __FUNCTION__)
#define ASSERT(a)  if (!(a)) { \
		while (1) { \
			printk(KERN_ERR "ASSERT FAIL AT FILE %s FUNC %s LINE %d\n", __FILE__, __FUNCTION__, __LINE__); \
		} \
}

#else
#define DPRINT(fmt, args ...)     printk("CIN: " fmt, ## args)
#define DBGMSG(fmt, args ...)     do {} while (0)
#define ENTER()                 do {} while (0)
#define LEAVE()                 do {} while (0)
#define FUNC_EXIT()                     do {} while (0)
#define ASSERT(a) if (!(a)) { \
		while (1) { \
			printk(KERN_CRIT "ASSERT FAIL AT FILE %s FUNC %s LINE %d\n", __FILE__, __FUNCTION__, __LINE__); \
		} \
}
#endif
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
extern void registerNZNETldisc(void);
ssize_t sendDataLowLevel(char * data, int len);

static void ccinet_tx_work(struct work_struct* work)
{
	struct sk_buff* tx_skb;

	while( (tx_skb = skb_dequeue(&tx_list.buff_head)) != NULL)
	{
		sendDataLowLevel(tx_skb->data, tx_skb->len);
		dev_kfree_skb(tx_skb);
	}
}

struct sk_buff * tx_fixup(struct net_device* netdev, struct sk_buff *skb, gfp_t flags)
{
	struct ccinethdr	*hdr;
	struct sk_buff			*skb2;
	unsigned	len;
	unsigned	tailpad;
	struct ccinet_priv* devobj = netdev_priv(netdev);
	unsigned cid = devobj->cid;

	/* handle ARP request */
#ifdef ARP_SUPPORT
	if(skb->protocol == htons(ETH_P_ARP)) {
		//netdev_dbg(dev->net, "process arp request\n");
		arp_process(dev, skb);
		dev_kfree_skb_any(skb);
		return NULL;
	}
#endif

	if(skb->protocol != htons(ETH_P_IP)
#ifdef IPV6_SUPPORT
		&& skb->protocol != htons(ETH_P_IPV6)
#endif
		) {
		//netif_dbg(dev, tx_err, dev->net, "drop non-ip packet\n");
		dev_kfree_skb_any(skb);
		return NULL;
	}

	/* strip Ethernet header */
	skb_pull(skb, ETH_HLEN);

	len = skb->len;
	tailpad = tx_padding_size(sizeof *hdr + len);

	if (likely(!skb_cloned(skb))) {
		int	headroom = skb_headroom(skb);
		int tailroom = skb_tailroom(skb);

		/* enough room as-is? */
		if (unlikely(sizeof *hdr + tailpad <= headroom + tailroom)) {
			/* do not need to be readjusted */
			if(sizeof *hdr <= headroom && tailpad <= tailroom)
				goto fill;

			skb->data = memmove(skb->head + sizeof *hdr,
				skb->data, len);
			skb_set_tail_pointer(skb, len);
			goto fill;
		}
	}

	/* create a new skb, with the correct size (and tailpad) */
	skb2 = skb_copy_expand(skb, sizeof *hdr, tailpad + 1, flags);
	dev_kfree_skb_any(skb);
	if (unlikely(!skb2))
		return skb2;
	skb = skb2;

	/* fill out the Nezha header and expand packet length to 8 bytes
	 * alignment.  we won't bother trying to batch packets;
	 */
fill:
	hdr = (void *) __skb_push(skb, sizeof *hdr);
	memset(hdr, 0, sizeof *hdr);
	hdr->iplen = cpu_to_be16(len);
	hdr->cid = cid;
	hdr->offset_len = 0;
	memset(skb_put(skb, tailpad), 0, tailpad);

	return skb;
}
/* add by jjp for test 0225 ++ */
#ifdef STATUS_TIMER_ENABLE
static void ccinet_print_status(unsigned long data)
{
	static unsigned long rx_last;
	static unsigned long tx_last;

	struct ccinet_priv* devobj = (struct ccinet_priv*)data;
	unsigned long rx_delta = devobj->net_stats.rx_bytes - rx_last;
	unsigned long tx_delta = devobj->net_stats.tx_bytes - tx_last;
	unsigned long tx_speed = ((tx_delta * 8) / 5);
	unsigned long rx_speed = ((rx_delta * 8) / 5);
	printk("CCinet %d tx %ld bytes in 5s, speed %ld bit/s, rx %ld bytes in 5s, speed %ld bit/s\n", devobj->cid,
		tx_delta, tx_speed,
		rx_delta, rx_speed);

   rx_last = devobj->net_stats.rx_bytes;
	tx_last = devobj->net_stats.tx_bytes;
	mod_timer(&devobj->ccinet_timer, jiffies + 5*HZ);
	
}
#endif
/* add by jjp for test 0225 -- */
///////////////////////////////////////////////////////////////////////////////////////
// Network Operations
///////////////////////////////////////////////////////////////////////////////////////
static int ccinet_open(struct net_device* netdev)
{
	#ifdef STATUS_TIMER_ENABLE
	struct ccinet_priv* devobj; // add by jjp for test 0225
	#endif
	ENTER();

//	netif_carrier_on(netdev);
	netif_start_queue(netdev);
	/* add by jjp for test 0225 ++  */
	#ifdef STATUS_TIMER_ENABLE
	devobj = netdev_priv(netdev);
	if(devobj->timer_enable)
	{
		devobj->ccinet_timer.data = (unsigned long)devobj;
		devobj->ccinet_timer.expires = jiffies + 5*HZ;
		devobj->ccinet_timer.function = ccinet_print_status;
		add_timer(&devobj->ccinet_timer);
	}
	#endif
/* add by jjp for test 0225 -- */
	LEAVE();
	return 0;
}

static int ccinet_stop(struct net_device* netdev)
{
	struct ccinet_priv* devobj;
	ENTER();
	devobj = netdev_priv(netdev);
	#ifdef STATUS_TIMER_ENABLE
	/* add by jjp for test 0225 ++ */
	if(devobj->timer_enable)
	{
		del_timer(&devobj->ccinet_timer);
	}
	/* add by jjp for test 0225 -- */
	#endif
	netif_stop_queue(netdev);
//	netif_carrier_off(netdev);
	memset(&devobj->net_stats, 0, sizeof(devobj->net_stats));
	LEAVE();
	return 0;
}

static int ccinet_tx(struct sk_buff* skb, struct net_device* netdev)
{
	struct ccinet_priv* devobj = netdev_priv(netdev);
	struct sk_buff* tx_skb;

	netdev->trans_start = jiffies;
	tx_skb = tx_fixup(netdev,skb,GFP_ATOMIC);

	if(tx_skb != NULL)
	{
	//	DBGMSG("********tx_list.buffhead %p, tx_skb :%p\n",&tx_list.buff_head, tx_skb);
		skb_queue_tail(&tx_list.buff_head, tx_skb);
		queue_work(ccinet_work_queue, &tx_worker);

	}

	//sendData(devobj->cid, skb->data + ETH_HLEN, skb->len - ETH_HLEN);

	/* update network statistics */
	devobj->net_stats.tx_packets++;
	devobj->net_stats.tx_bytes += skb->len;
	//dev_kfree_skb(skb);
	return 0;

}

static void ccinet_tx_timeout(struct net_device* netdev)
{
	struct ccinet_priv* devobj = netdev_priv(netdev);

	ENTER();
	devobj->net_stats.tx_errors++;
//	netif_wake_queue(netdev); // Resume tx
	return;
}

static struct net_device_stats *ccinet_get_stats(struct net_device *dev)
{
	struct ccinet_priv* devobj;

	devobj = netdev_priv(dev);
	ASSERT(devobj);
	return &devobj->net_stats;
}

static int ccinet_rx(struct net_device* netdev, char* packet, int pktlen)
{

	struct sk_buff *skb;
	struct ccinet_priv *priv = netdev_priv(netdev);
	struct iphdr* ip_header = (struct iphdr*)packet;
	struct ethhdr ether_header;

	if (ip_header->version == 4) {
		ether_header.h_proto = htons(ETH_P_IP);
	} else if (ip_header->version == 6) {
		ether_header.h_proto = htons(ETH_P_IPV6);
	} else {
		printk(KERN_ERR "ccinet_rx: invalid ip version: %d\n", ip_header->version);
		priv->net_stats.rx_dropped++;
		hex_packet(packet,pktlen > 100?100:pktlen);
		goto out;
	}
	memcpy(ether_header.h_dest, netdev->dev_addr, ETH_ALEN);
	memset(ether_header.h_source, 0, ETH_ALEN);

	//ENTER();
	//DBGMSG("ccinet_rx:pktlen=%d\n", pktlen);
	skb = dev_alloc_skb(pktlen + NET_IP_ALIGN + sizeof(ether_header));
	ASSERT(skb);

	if (!skb)
	{

		if (printk_ratelimit(  ))

			printk(KERN_NOTICE "ccinet_rx: low on mem - packet dropped\n");

		priv->net_stats.rx_dropped++;

		goto out;

	}
	skb_reserve(skb, NET_IP_ALIGN);
	memcpy(skb_put(skb, sizeof(ether_header)), &ether_header, sizeof(ether_header));

	memcpy(skb_put(skb, pktlen), packet, pktlen);

	/* Write metadata, and then pass to the receive level */

	skb->dev = netdev;
	skb->protocol = eth_type_trans(skb, netdev); //htons(ETH_P_IP);//eth_type_trans(skb, netdev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	priv->net_stats.rx_packets++;
	priv->net_stats.rx_bytes += pktlen + sizeof(ether_header);
	// jjp modefy for test speed
	if(netif_rx(skb)!= NET_RX_SUCCESS)
	{
		printk(KERN_NOTICE "ccinet_rx: netif_rx failed - packet dropped\n");
		priv->net_stats.rx_dropped++;
	} 
	
	//where to free skb?

	return 0;

 out:
	return -1;
}

static int validate_addr(struct net_device* netdev)
{
	ENTER();
	return 0;
}

#ifdef DATA_IND_BUFFERLIST
void addrxnode(RXINDBUFNODE *newBufNode)
{
	RXINDBUFNODE *pCurrNode;

	if (rxbuflist == NULL)
	{
		rxbuflist = newBufNode;
	}
	else
	{
		pCurrNode = rxbuflist;
		while (pCurrNode->next != NULL)
			pCurrNode = pCurrNode->next;

		pCurrNode->next = newBufNode;

	}
	return;
}

int sendDataIndtoInternalList(unsigned char cid, char* buf, int len)
{
	RXINDBUFNODE* rxnode=NULL;
	UINT32 flag;
	int ret=0;

	rxnode = kmalloc(sizeof(RXINDBUFNODE), GFP_KERNEL);
	if (!rxnode) {
		ret = -ENOMEM;
		goto err;
	}
	rxnode->rx_packet.pktdata = kmalloc(len, GFP_KERNEL);
	if (!(rxnode->rx_packet.pktdata)) {
		ret = -ENOMEM;
		goto err;
	}
	memcpy(rxnode->rx_packet.pktdata, buf, len);
	rxnode->rx_packet.length = len;
	rxnode->rx_packet.cid = cid;
	rxnode->next = NULL;
	addrxnode(rxnode);
#if 0
	OSAFlagPeek(dataChanRxFlagRef, &flag);

	if (!(flag & rxDataMask))
	{
		OSAFlagSet(dataChanRxFlagRef, rxDataMask, OSA_FLAG_OR);
	}
#endif
	complete(&dataChanRxFlagRef);
	return 0;

err:
	if (rxnode)
		free(rxnode);
	return ret;
}
void recvDataOptTask(void *data)
{
	CINETDEVLIST *pdrv;
	int err;
	RXINDBUFNODE *tmpnode;
	UINT32 flags;
	OS_STATUS osaStatus;

	//while(!kthread_should_stop())
	while (1)
	{

		if (rxbuflist != NULL)
		{
			err = search_list_by_cid(rxbuflist->rx_packet.cid, &pdrv);
			ASSERT(err == 0);

			ccinet_rx(pdrv->ccinet, rxbuflist->rx_packet.pktdata, rxbuflist->rx_packet.length);
			tmpnode = rxbuflist;
			rxbuflist = rxbuflist->next;
			kfree(tmpnode->rx_packet.pktdata);
			kfree(tmpnode);

		}
		else
		{
			wait_for_completion_interruptible(&dataChanRxFlagRef);
		}
	}

}
void initDataChanRecv()
{

	ENTER();

	init_completion(&dataChanRxFlagRef);

	cinetDataRcvTaskRef = kthread_run(recvDataOptTask, NULL, "recvDataOptTask");
	LEAVE();
}

#endif

int data_rx(char* packet, int len, unsigned char cid)
{
	if (cid >= MAX_CID_NUM)
		return -1;
#ifndef DATA_IND_BUFFERLIST
	ccinet_rx(net_devices[cid], packet, len);
#else
	int err = sendDataIndtoInternalList(cid, packet, len);
	if (err != 0)
		return -1;
#endif
	return len;
}

///////////////////////////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////////////////////////

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
static const struct net_device_ops cci_netdev_ops = {
	.ndo_open		= ccinet_open,
	.ndo_stop		= ccinet_stop,
	.ndo_start_xmit 	= ccinet_tx,
	.ndo_tx_timeout		= ccinet_tx_timeout,
	.ndo_get_stats 	= ccinet_get_stats,   
	.ndo_validate_addr	= validate_addr
};
#endif
static void ccinet_setup(struct net_device* netdev)
{
	ENTER();
	ether_setup(netdev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	netdev->netdev_ops = &cci_netdev_ops;
#else
	netdev->open = ccinet_open;
	netdev->stop = ccinet_stop;
	netdev->hard_start_xmit = ccinet_tx;
	netdev->tx_timeout = ccinet_tx_timeout;
	netdev->get_stats = ccinet_get_stats;
	netdev->validate_addr = validate_addr;
#endif

	netdev->watchdog_timeo = 5;  //jiffies
	netdev->flags |= IFF_NOARP;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21)
	netdev->hard_header_cache = NULL;      /* Disable caching */
#endif
	random_ether_addr(netdev->dev_addr);
	LEAVE();
}

static int __init ccinet_init(void)
{
	int i;
	for (i = 0; i < MAX_CID_NUM; i++) {
		char ifname[32];
		struct net_device *dev;
		struct ccinet_priv *priv;
		int ret;

		sprintf(ifname, INTFACE_NAME"%d", i);
		dev = alloc_netdev(sizeof(struct ccinet_priv), ifname, ccinet_setup);

		if (!dev) {
			printk(KERN_ERR "%s: alloc_netdev for %s fail\n", __FUNCTION__, ifname);
			return -ENOMEM;
		}
		ret = register_netdev(dev);
		if (ret) {
			printk(KERN_ERR "%s: register_netdev for %s fail\n", __FUNCTION__, ifname);
			free_netdev(dev);
			return ret;
		}
		priv = netdev_priv(dev);
		memset(priv, 0, sizeof(struct ccinet_priv));
	#ifdef STATUS_TIMER_ENABLE	
		init_timer(&priv->ccinet_timer);    // add by jjp for test
		priv->timer_enable = STATUS_TIMER_ENABLE;  // add by jjp for test
	#endif	
		spin_lock_init(&priv->lock);
		priv->cid = i;
		net_devices[i] = dev;
	}

	skb_queue_head_init(&tx_list.buff_head);
	spin_lock_init(&tx_list.buff_lock);

	ccinet_work_queue = create_workqueue("ccinet work queue");
	INIT_WORK(&tx_worker,ccinet_tx_work);

	registerNZNETldisc();

	//registerRxCallBack(PDP_DIRECTIP, data_rx);

#ifdef  DATA_IND_BUFFERLIST
	initDataChanRecv();
#endif
	return 0;
};

static void __exit ccinet_exit(void)
{
	int i;
	for (i = 0; i < MAX_CID_NUM; i++) {
		unregister_netdev(net_devices[i]);
		free_netdev(net_devices[i]);
		net_devices[i] = NULL;
	}
#ifdef  DATA_IND_BUFFERLIST
	kthread_stop(cinetDataRcvTaskRef);
#endif

}

module_init(ccinet_init);
module_exit(ccinet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marvell");
MODULE_DESCRIPTION("Marvell CI Network Driver");

