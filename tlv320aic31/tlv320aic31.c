/* 
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * 
 * History: 
 *      10-April-2006 create this file
 */


#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
//#include <linux/smp_lock.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <mach/hardware.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#ifndef HI_GPIO_I2C
#define HI_GPIO_I2C
#endif

#ifdef HI_GPIO_I2C
#include "gpio_i2c.h"
#else
#include "i2c.h"
#endif


#include "tlv320aic31.h"
#include "tlv320aic31_def.h"


#define CHIP_NUM 1
#define DEV_NAME "tlv320aic31"
#define DEBUG_LEVEL 1
#define DPRINTK(level,fmt,args...) do{ if(level < DEBUG_LEVEL)\
    printk(KERN_INFO "%s [%s ,%d]: " fmt "\n",DEV_NAME,__FUNCTION__,__LINE__,##args);\
}while(0)

unsigned int IIC_device_addr[CHIP_NUM] = {0x30};

static unsigned int  open_cnt = 0;	


void tlv320aic31_write(unsigned char chip_addr,unsigned char reg_addr,unsigned char value)
{
#ifdef HI_GPIO_I2C
    gpio_i2c_write(chip_addr,reg_addr,value);
#else
    i2c_write(chip_addr,reg_addr,value);
#endif
}
int tlv320aic31_read(unsigned char chip_addr,unsigned char reg_addr)
{
#ifdef HI_GPIO_I2C
    return gpio_i2c_read(chip_addr,reg_addr);
#else    
    return i2c_read(chip_addr,reg_addr);
#endif
}
void tlv320aic31_reg_dump(unsigned int reg_num)
{
    unsigned int i = 0;
    for(i = 0;i < reg_num;i++)
    {
        printk("reg%d =%x,",i,tlv320aic31_read(IIC_device_addr[0],i));
        if((i+1)%8==0)
        {
            printk("\n");
        }
    }
}
void soft_reset(unsigned int chip_num)
{
       /*soft reset*/ 
        tlv320aic31_write(IIC_device_addr[chip_num],0x1,0x80);
        udelay(50);

#if 1
		/*CODEC_CLKIN uses CLKDIV_OUT*/
    	tlv320aic31_write(IIC_device_addr[chip_num], 101, 1);
		/*CLKDIV_IN uses MCLK*/
		tlv320aic31_write(IIC_device_addr[chip_num], 102, 0x32);
    	/*PLL disable and select Q value*/
    	tlv320aic31_write(IIC_device_addr[chip_num], 3, 0x10);//Q=2
#else
		/*MCLK采用26.04MHz*/
		
        /*PLL enable */
        tlv320aic31_write(IIC_device_addr[chip_num], 3, 0x82);/* P=2 */
        tlv320aic31_write(IIC_device_addr[chip_num], 4, 0x1c);/* J=7 */
        tlv320aic31_write(IIC_device_addr[chip_num], 5, 0x55);
        tlv320aic31_write(IIC_device_addr[chip_num], 6, 0xf8);/* reg 5 and 6 set D=5618  ,0x55,0xf8*/ 
        tlv320aic31_write(IIC_device_addr[chip_num], 11, 0x1);/* R=1 */

		/*PLLCLK_IN uses MCLK*/
		tlv320aic31_write(IIC_device_addr[chip_num],101,0x0);
		tlv320aic31_write(IIC_device_addr[chip_num], 102, 0xc2);
#endif
        /*left and right DAC open*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 7,  0xa);/* FSref = 48 kHz */

        /*sample*/
    	tlv320aic31_write(IIC_device_addr[chip_num], 2,  0xaa);/* FS = FSref/6 */
                
        /*ctrl mode*/
        tlv320aic31_write(IIC_device_addr[chip_num], 8,  0xf0);/* master mode */
                
        /*Audio Serial Data Interface Control*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 9,  0x7);/* I2S mode,16bit */

        /*Audio Codec Digital Filter Control Register*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 12,  0x50);

        //tlv320aic31_write(IIC_device_addr[chip_num], 25,  0x0);
        tlv320aic31_write(IIC_device_addr[chip_num], 25,  0x40);

        tlv320aic31_write(IIC_device_addr[chip_num], 15,  0x0);
        tlv320aic31_write(IIC_device_addr[chip_num], 16,  0x0);
		
    	tlv320aic31_write(IIC_device_addr[chip_num], 17,  0x0f);
	
		tlv320aic31_write(IIC_device_addr[chip_num], 19,  0x7c);

    	tlv320aic31_write(IIC_device_addr[chip_num], 24,  0x80);


		tlv320aic31_write(IIC_device_addr[chip_num], 22,  0x7C);

        //tlv320aic31_write(IIC_device_addr[chip_num], 19,  0x7c);
        //tlv320aic31_write(IIC_device_addr[chip_num], 22,  0x7c);


		//Left-AGC Control Register C
        tlv320aic31_write(IIC_device_addr[chip_num], 28,  0x0);
		//Right-AGC Control Register C
        tlv320aic31_write(IIC_device_addr[chip_num], 31,  0x0);
            	
        /*out ac-coupled*/	
		//Headset/Button Press Detection Register B，??
        tlv320aic31_write(IIC_device_addr[chip_num], 14, 0x80);


		/*left and right DAC power on*/	
    	tlv320aic31_write(IIC_device_addr[chip_num], 37, 0xc0);  

    	/*out common-mode voltage*/	
    	tlv320aic31_write(IIC_device_addr[chip_num], 40, 0x80);

        /*out path select*/	
    	tlv320aic31_write(IIC_device_addr[chip_num], 41, 0x0);    

        /*out path select*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 42, 0xa8);  
        
        /*left DAC not muted*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 43, 0x0);    

        /*right DAC not muted*/	
        tlv320aic31_write(IIC_device_addr[chip_num], 44, 0x0); 

		
		tlv320aic31_write(IIC_device_addr[chip_num], 47, 0x80); 

		tlv320aic31_write(IIC_device_addr[chip_num], 51, 0x9f); 

		tlv320aic31_write(IIC_device_addr[chip_num], 85, 0x80); 

		tlv320aic31_write(IIC_device_addr[chip_num], 86, 0x9f); 
		
	    tlv320aic31_write(IIC_device_addr[chip_num], 38, 0x3e);  
        
            
}        	

/*
 *	device open. set counter
 */
static int tlv320aic31_open(struct inode * inode, struct file * file)
{
	if(0 == open_cnt++)
		return 0;    	
	return -1 ;
}

/*
 *	Close device, Do nothing!
 */
static int tlv320aic31_close(struct inode *inode ,struct file *file)
{
	open_cnt--;
    	return 0;
}

//static int tlv320aic31_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
static long tlv320aic31_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int __user *argp = (unsigned int __user *)arg;
    unsigned int chip_num;
	Audio_Ctrl temp;
    Audio_Ctrl *audio_ctrl;
    Codec_Datapath_Setup_Ctrl codec_datapath_setup_ctrl;
    DAC_OUTPUT_SWIT_CTRL dac_output_swit_ctrl;
    DAC_POWER_CTRL dac_power_ctrl;
    In1_Adc_Ctrl in1_adc_ctrl ;
    In2_Adc_Ctrl_Sample in2_adc_ctrl_sample ;
    Adc_Pga_Dac_Gain_Ctrl adc_pga_dac_gain_ctrl;
    Line_Hpcom_Out_Ctrl line_hpcom_out_ctrl;
    Serial_Int_Ctrl serial_int_ctrl;
    Serial_Data_Offset_Ctrl serial_data_offset_ctrl;
    Ctrl_Mode ctrl_mode; 

	if(argp != NULL)
	{
        if(copy_from_user(&temp,argp,sizeof(Audio_Ctrl)))
	    {   
    	    return -EFAULT;
   	 	}
	}
    audio_ctrl = (Audio_Ctrl *)(&temp);
    chip_num = audio_ctrl->chip_num;
    switch(cmd)
    {
        case IN2LR_2_LEFT_ADC_CTRL:
            in2_adc_ctrl_sample.b8 = tlv320aic31_read(IIC_device_addr[chip_num],17);      
            in2_adc_ctrl_sample.bit.in2l_adc_input_level_sample = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],17,in2_adc_ctrl_sample.b8);
            break;
        case IN2LR_2_RIGTH_ADC_CTRL:
            in2_adc_ctrl_sample.b8 = tlv320aic31_read(IIC_device_addr[chip_num],18);      
            in2_adc_ctrl_sample.bit.in2r_adc_input_level_sample = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],18,in2_adc_ctrl_sample.b8);
             
            break;
        case IN1L_2_LEFT_ADC_CTRL:
            in1_adc_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],19);      
            in1_adc_ctrl.bit.in1_adc_input_level = audio_ctrl->input_level;
            in1_adc_ctrl.bit.adc_ch_power_ctrl = audio_ctrl->if_powerup;
            tlv320aic31_write(IIC_device_addr[chip_num],19,in1_adc_ctrl.b8);
            break;
        case IN1R_2_RIGHT_ADC_CTRL:
            in1_adc_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],22);      
            in1_adc_ctrl.bit.in1_adc_input_level = audio_ctrl->input_level;
            in1_adc_ctrl.bit.adc_ch_power_ctrl = audio_ctrl->if_powerup;
            tlv320aic31_write(IIC_device_addr[chip_num],22,in1_adc_ctrl.b8);
            break;
        case PGAL_2_HPLOUT_VOL_CTRL:
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],46);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],46,adc_pga_dac_gain_ctrl.b8);
            break;
        case DACL1_2_HPLOUT_VOL_CTRL:
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],47);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],47,adc_pga_dac_gain_ctrl.b8);
            break;
        case HPLOUT_OUTPUT_LEVEL_CTRL:
            line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],51); 
            line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
            line_hpcom_out_ctrl.bit. output_level =  audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],51,line_hpcom_out_ctrl.b8);
            break; 
        case PGAL_2_HPLCOM_VOL_CTRL:
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],53);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],53,adc_pga_dac_gain_ctrl.b8);
            break;
        case DACL1_2_HPLCOM_VOL_CTRL:
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],54);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],54,adc_pga_dac_gain_ctrl.b8);
            break;
        case HPLCOM_OUTPUT_LEVEL_CTRL:
           line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],58); 
           line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
           line_hpcom_out_ctrl.bit.output_level =  audio_ctrl->input_level;
           tlv320aic31_write(IIC_device_addr[chip_num],58,line_hpcom_out_ctrl.b8);
          break;
        case PGAR_2_HPROUT_VOL_CTRL:
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],63);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],63,adc_pga_dac_gain_ctrl.b8);
            break;
        case DACR1_2_HPROUT_VOL_CTRL: 
            adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],64);
            adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
            adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
            tlv320aic31_write(IIC_device_addr[chip_num],64,adc_pga_dac_gain_ctrl.b8);
            break;
        case HPROUT_OUTPUT_LEVEL_CTRL:
           line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],65); 
           line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
           line_hpcom_out_ctrl.bit. output_level =  audio_ctrl->input_level;
           tlv320aic31_write(IIC_device_addr[chip_num],65,line_hpcom_out_ctrl.b8);
           break;
        case PGAR_2_HPRCOM_VOL_CTRL: 
              adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],70);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],70,adc_pga_dac_gain_ctrl.b8);
              break;
        case DACR1_2_HPRCOM_VOL_CTRL:
             adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],71);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route; 
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],71,adc_pga_dac_gain_ctrl.b8);
                break;
        case HPRCOM_OUTPUT_LEVEL_CTRL:
              line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],72); 
               line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
               line_hpcom_out_ctrl.bit.output_level =  audio_ctrl->input_level;
               tlv320aic31_write(IIC_device_addr[chip_num],72,line_hpcom_out_ctrl.b8);
              break;
        case PGAL_2_LEFT_LOP_VOL_CTRL:
              adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],81);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],81,adc_pga_dac_gain_ctrl.b8);
                break;
        case DACL1_2_LEFT_LOP_VOL_CTRL:
             adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],82);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route; 
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],82,adc_pga_dac_gain_ctrl.b8);
                break;
        case LEFT_LOP_OUTPUT_LEVEL_CTRL:
              line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],86); 
               line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
               line_hpcom_out_ctrl.bit.output_level =  audio_ctrl->input_level;
               line_hpcom_out_ctrl.bit.power_status =  audio_ctrl->if_powerup;
               tlv320aic31_write(IIC_device_addr[chip_num],86,line_hpcom_out_ctrl.b8);
              break;
        case PGAR_2_RIGHT_LOP_VOL_CTRL:
              adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],91);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],91,adc_pga_dac_gain_ctrl.b8);
                break;
        case DACR1_2_RIGHT_LOP_VOL_CTRL:
             adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],92);
              adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route; 
              adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
              tlv320aic31_write(IIC_device_addr[chip_num],92,adc_pga_dac_gain_ctrl.b8);
                break;
        case RIGHT_LOP_OUTPUT_LEVEL_CTRL:
              line_hpcom_out_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],93); 
               line_hpcom_out_ctrl.bit.if_mute = audio_ctrl->if_mute_route;
               line_hpcom_out_ctrl.bit.output_level =  audio_ctrl->input_level;
               line_hpcom_out_ctrl.bit.power_status =  audio_ctrl->if_powerup;
               tlv320aic31_write(IIC_device_addr[chip_num],93,line_hpcom_out_ctrl.b8);
              break;
        case SET_ADC_SAMPLE:
                in2_adc_ctrl_sample.b8 = tlv320aic31_read(IIC_device_addr[chip_num],2);      
                in2_adc_ctrl_sample.bit.in2l_adc_input_level_sample = audio_ctrl->sample;
                tlv320aic31_write(IIC_device_addr[chip_num],2,in2_adc_ctrl_sample.b8);
                break;
        case SET_DAC_SAMPLE:
                in2_adc_ctrl_sample.b8 = tlv320aic31_read(IIC_device_addr[chip_num],2);      
                in2_adc_ctrl_sample.bit.in2r_adc_input_level_sample = audio_ctrl->sample;
                tlv320aic31_write(IIC_device_addr[chip_num],2,in2_adc_ctrl_sample.b8);
                //printk("set SET_DAC_SAMPLE,audio_ctrl->sample=%x\n",audio_ctrl->sample);
                break;
        case SET_DATA_LENGTH:
                serial_int_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],9);;                
                serial_int_ctrl.bit.data_length = audio_ctrl->data_length;
                //tlv320aic31_write(IIC_device_addr[chip_num],9,serial_int_ctrl.b8);
                break;
        case SET_TRANSFER_MODE:
                serial_int_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],9);    
                serial_int_ctrl.bit.transfer_mode = audio_ctrl->trans_mode;
                tlv320aic31_write(IIC_device_addr[chip_num],9,serial_int_ctrl.b8);
                break;              
        case SET_CTRL_MODE:
                //tlv320aic31_write(IIC_device_addr[chip_num],0x1,0x80);
                //udelay(50);  
                ctrl_mode.b8 = tlv320aic31_read(IIC_device_addr[chip_num],8);
                ctrl_mode.bit.bit_clock_dic_ctrl =  audio_ctrl->ctrl_mode;
                ctrl_mode.bit.work_clock_dic_ctrl =  audio_ctrl->ctrl_mode;
                ctrl_mode.bit.bit_work_dri_ctrl =  audio_ctrl->ctrl_mode;
                tlv320aic31_write(IIC_device_addr[chip_num],8,ctrl_mode.b8);

                /* 设置时钟 */
                if (1 == audio_ctrl->ctrl_mode 
                    || (AC31_SET_48K_SAMPLERATE != audio_ctrl->sample && AC31_SET_44_1K_SAMPLERATE != audio_ctrl->sample))
                {
                    /* aic31作主模式或者采样率不为44.1K/48KHZ的情况下，使用外部的12.288MHZ的晶振作为MCLK输入并产生内部工作主时钟 */
                    if ((1 == audio_ctrl->if_44100hz_series))
                    {
                        /*　如果为44.1KHZ系列的采样样 */
                        tlv320aic31_write(IIC_device_addr[chip_num],3,0x81);    /* P=1 */ 
                        tlv320aic31_write(IIC_device_addr[chip_num],4,0x1c);    /* J=7 */
                        tlv320aic31_write(IIC_device_addr[chip_num],5,0x36);    /* reg 5 and 6 set D=3500*/
                        tlv320aic31_write(IIC_device_addr[chip_num],6,0xb0);
                        codec_datapath_setup_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],7);
                        codec_datapath_setup_ctrl.b8 |= 0x80;   /* FSref = 44.1 kHz */
                        tlv320aic31_write(IIC_device_addr[chip_num],7,codec_datapath_setup_ctrl.b8);
                        tlv320aic31_write(IIC_device_addr[chip_num],11,0x1);    /* R=1 */
                        tlv320aic31_write(IIC_device_addr[chip_num],101,0x0);
                        tlv320aic31_write(IIC_device_addr[chip_num],102,0xc2);
                    }
                    else
                    {
                        /*　如果为非44.1KHZ系列的采样样 */
                        tlv320aic31_write(IIC_device_addr[chip_num],3,0x81);    /* P=1 */ 
                        tlv320aic31_write(IIC_device_addr[chip_num],4,0x20);    /* J=8 */
                        tlv320aic31_write(IIC_device_addr[chip_num],5,0x0);     /* reg 5 and 6 set D=0000*/
                        tlv320aic31_write(IIC_device_addr[chip_num],6,0x0);
                        codec_datapath_setup_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],7);
                        codec_datapath_setup_ctrl.b8 &= 0x7f;   /* FSref = 48 kHz */
                        tlv320aic31_write(IIC_device_addr[chip_num],7,codec_datapath_setup_ctrl.b8);
                        tlv320aic31_write(IIC_device_addr[chip_num],11,0x1);    /* R=1 */
                        tlv320aic31_write(IIC_device_addr[chip_num],101,0x0);
                        tlv320aic31_write(IIC_device_addr[chip_num],102,0xc2);
                    }
                }
                else
                {
                    /* aic31做从模式且采样率为44.1K/48KHZ的情况下，由BCLK产生内部工作主时钟 */
                    tlv320aic31_write(IIC_device_addr[chip_num],102,0x22);  /* uses PLLCLK and BCLK */
                    codec_datapath_setup_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],7);
                    if ((1 == audio_ctrl->if_44100hz_series))
                    {
                        codec_datapath_setup_ctrl.b8 |= 0x80;   /* FSref = 44.1 kHz */
                    }
                    else
                    {
                        codec_datapath_setup_ctrl.b8 &= 0x7f;   /* FSref = 48 kHz */
                    }
                    tlv320aic31_write(IIC_device_addr[chip_num],7,codec_datapath_setup_ctrl.b8);

                    tlv320aic31_write(IIC_device_addr[chip_num],3,0x81);    /* P=1 */ 
                    tlv320aic31_write(IIC_device_addr[chip_num],4,32<<2);   /* set PLL J to 32 */
                    tlv320aic31_write(IIC_device_addr[chip_num],5,0x0);     /* reg 5 and 6 set D=0000*/
                    tlv320aic31_write(IIC_device_addr[chip_num],6,0x0);
                    tlv320aic31_write(IIC_device_addr[chip_num],101,0x0);   /* CODEC_CLKIN uses PLLDIV_OUT */
                    tlv320aic31_write(IIC_device_addr[chip_num],11,0x2);    /* R = 2 */
                }
                break;
        case LEFT_DAC_VOL_CTRL:
                adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],43);
                adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route; 
                adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
                tlv320aic31_write(IIC_device_addr[chip_num],43,adc_pga_dac_gain_ctrl.b8);
                break;
        case RIGHT_DAC_VOL_CTRL:
                adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],44);
                adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route; 
                adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
                tlv320aic31_write(IIC_device_addr[chip_num],44,adc_pga_dac_gain_ctrl.b8);
                break;
        case LEFT_DAC_POWER_SETUP:
                codec_datapath_setup_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],7);
                codec_datapath_setup_ctrl.bit.left_dac_datapath_ctrl = audio_ctrl->if_powerup;
                tlv320aic31_write(IIC_device_addr[chip_num],7,codec_datapath_setup_ctrl.b8);
                dac_power_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],37);
                dac_power_ctrl.bit.left_dac_power_ctrl =  audio_ctrl->if_powerup;
                tlv320aic31_write(IIC_device_addr[chip_num],37,dac_power_ctrl.b8);
                 break;
        case RIGHT_DAC_POWER_SETUP:
                codec_datapath_setup_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],7);
                codec_datapath_setup_ctrl.bit.right_dac_datapath_ctrl = audio_ctrl->if_powerup;
                tlv320aic31_write(IIC_device_addr[chip_num],7,codec_datapath_setup_ctrl.b8);
                dac_power_ctrl.b8 =  tlv320aic31_read(IIC_device_addr[chip_num],37);
                dac_power_ctrl.bit.right_dac_power_ctrl =  audio_ctrl->if_powerup;
                tlv320aic31_write(IIC_device_addr[chip_num],37,dac_power_ctrl.b8);
                 break;
        case DAC_OUT_SWITCH_CTRL:  
                dac_output_swit_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],41);
                dac_output_swit_ctrl.bit.left_dac_swi_ctrl =  audio_ctrl->dac_path;
                dac_output_swit_ctrl.bit.right_dac_swi_ctrl = audio_ctrl->dac_path;
                tlv320aic31_write(IIC_device_addr[chip_num],41,dac_output_swit_ctrl.b8);
                 break;
        case LEFT_ADC_PGA_CTRL:
                adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],15);
                adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
                adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
                tlv320aic31_write(IIC_device_addr[chip_num],15,adc_pga_dac_gain_ctrl.b8);
                break;
        case RIGHT_ADC_PGA_CTRL:
                adc_pga_dac_gain_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],16);
                adc_pga_dac_gain_ctrl.bit.if_mute_route = audio_ctrl->if_mute_route;
                adc_pga_dac_gain_ctrl.bit.input_vol_level_ctrl = audio_ctrl->input_level;
                tlv320aic31_write(IIC_device_addr[chip_num],16,adc_pga_dac_gain_ctrl.b8);
                break;
        case SET_SERIAL_DATA_OFFSET:
                serial_data_offset_ctrl.b8 = tlv320aic31_read(IIC_device_addr[chip_num],10);
                serial_data_offset_ctrl.bit.serial_data_offset = audio_ctrl->data_offset;
                tlv320aic31_write(IIC_device_addr[chip_num],10,serial_data_offset_ctrl.b8);
                break;
        case SOFT_RESET:
                //printk("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "invalid attribute");
                soft_reset(chip_num); 
                break;
        case TLV320AIC31_REG_DUMP:
                tlv320aic31_reg_dump(102);
                break;
        default:
                break;
    }
    return 0;
}

/*
 *  The various file operations we support.
 */
 
static struct file_operations tlv320aic31_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl		= tlv320aic31_ioctl,
	.open		= tlv320aic31_open,
	.release	= tlv320aic31_close
};

static struct miscdevice tlv320aic31_dev = {
	MISC_DYNAMIC_MINOR,
	DEV_NAME,
	&tlv320aic31_fops,
};

static int tlv320aic31_device_init(unsigned int num)
{
#if 1    
        /* inite codec configs.*/
        unsigned char temp = 0;
        temp = tlv320aic31_read(IIC_device_addr[num],0x2);
        tlv320aic31_write(IIC_device_addr[0],0x2,0xaa);
        if( tlv320aic31_read(IIC_device_addr[num],0x2) != 0xaa)
        {
            DPRINTK(0,"init aic31(%d) error",num);
            return -1;
        }
        tlv320aic31_write(IIC_device_addr[num],0x2,temp);
#endif        
        soft_reset(num);
#if 0
        for (temp = 0;temp < 30;temp ++)
        {
            printk("0x%x, 0x%x\n",temp,tlv320aic31_read(IIC_device_addr[num],temp));
        }
#endif        
    	return 0;
}   	
static int chip_count = 1;
module_param(chip_count,int,0);
MODULE_PARM_DESC(chip_count,"the num we device uses the tlv320aic31,default 1");

static int __init tlv320aic31_init(void)
{
    	unsigned int i,ret;

    	ret = misc_register(&tlv320aic31_dev);
    	if(ret)
    	{
    		DPRINTK(0,"could not register tlv320aic31 device");
    		return -1;
    	}
        for(i = 0;i< chip_count;i++)
        {
            if(tlv320aic31_device_init(i) < 0)
            {
                goto init_fail;
            }
        }
    	DPRINTK(1,"tlv320aic31 driver init successful!");
    	return ret;
init_fail:
        misc_deregister(&tlv320aic31_dev);
        DPRINTK(0,"tlv320aic31 device init fail,deregister it!");
        return -1;
}     

static void __exit tlv320aic31_exit(void)
{
    misc_deregister(&tlv320aic31_dev);

    DPRINTK(1,"deregister tlv320aic31");
}

module_init(tlv320aic31_init);
module_exit(tlv320aic31_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");

