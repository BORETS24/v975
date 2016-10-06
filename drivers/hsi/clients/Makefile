#
# Makefile for HSI clients
#
ccflags-y += -DDEBUG


obj-$(CONFIG_HSI_LOGICAL) += hsi_flash.o
obj-$(CONFIG_HSI_LOGICAL) += hsi_logical.o
obj-$(CONFIG_HSI_FFL_TTY) += hsi_ffl_tty.o
obj-$(CONFIG_HSI_DLP) 	  += hsi_dlp.o
ifeq ($(CONFIG_MARVELL_MODEM_HSI),y)
    hsi_dlp-objs := dlp_main_mvl.o dlp_ctrl_mvl.o dlp_tty_mvl.o
else
    hsi_dlp-objs := dlp_flash.o dlp_main.o dlp_ctrl.o dlp_tty.o dlp_net.o dlp_trace.o
endif

