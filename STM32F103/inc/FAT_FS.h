/*
 * (c) Domen Puncer, Visionect, d.o.o.
 * BSD License
 *
 * v0.2 add support for SDHC
 */

#include "stdio.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

/*
 * Code is split into 3 parts:
 * - generic SPI code: adapt for your MCU
 * - sd card code, with crc7 and crc16 calculations
 *   there's lots of it, but it's simple
 * - fatfs interface. If you use anything else, look here for
 *   interface to SD card code
 */

struct hwif {
	int initialized;
	int sectors;
	int erase_sectors;
	int capabilities;
};
typedef struct hwif hwif;

#define CAP_VER2_00	(1<<0)
#define CAP_SDHC	(1<<1)


enum sd_speed { SD_SPEED_INVALID, SD_SPEED_400KHZ, SD_SPEED_25MHZ };

//_____________________________________________________________
//_________________________SPI FUNCTIONS________________________

/*** spi functions ***/

static void spi_set_speed(enum sd_speed speed);

/* SD card is connected to SPI1, PA4-7 */

#define SPI_SD SPI1

static void spi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	spi_set_speed(SD_SPEED_400KHZ);
}

#define spi_cs_low() do { GPIOA->BRR = GPIO_Pin_4; } while (0)
#define spi_cs_high() do { GPIOA->BSRR = GPIO_Pin_4; } while (0)

static void spi_set_speed(enum sd_speed speed)
{
	SPI_InitTypeDef SPI_InitStructure;
	int prescaler = SPI_BaudRatePrescaler_128;

	if (speed == SD_SPEED_400KHZ)
		prescaler = SPI_BaudRatePrescaler_128;
	else if (speed == SD_SPEED_25MHZ)
		prescaler = SPI_BaudRatePrescaler_2;
	/* ^ with /2 APB1 this will be 15mhz/234k at 60mhz
	 * 18/281 at 72. which is ok, 100<x<400khz, and <25mhz */

	SPI_Cmd(SPI_SD, DISABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = prescaler;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI_SD, &SPI_InitStructure);

	SPI_Cmd(SPI_SD, ENABLE);
}

static u8 spi_txrx(u8 data)
{
	/* RXNE always happens after TXE, so if this function is used
	 * we don't need to check for TXE */

	SPI_SD->DR = (vu16)data;

	while ((SPI_SD->SR & SPI_I2S_FLAG_RXNE) == 0);

	return (u8)SPI_SD->DR;

}


//_____________________________________________________________
//_________________________CRC HELPERS_________________________


/* crc helpers */
static u8 crc7_one(u8 t, u8 data)
{
	int i;
	const u8 g = 0x89;

	t ^= data;
	for (i=0; i<8; i++) {
		if (t & 0x80)
			t ^= g;
		t <<= 1;
	}
	return t;
}

u8 crc7(const u8 *p, int len)
{
	int j;
	u8 crc = 0;
	for (j=0; j<len; j++)
		crc = crc7_one(crc, p[j]);

	return crc>>1;
}

/* http://www.eagleairaust.com.au/code/crc16.htm */
static u16 crc16_ccitt(u16 crc, u8 ser_data)
{
	crc  = (u8)(crc >> 8) | (crc << 8);
	crc ^= ser_data;
	crc ^= (u8)(crc & 0xff) >> 4;
	crc ^= (crc << 8) << 4;
	crc ^= ((crc & 0xff) << 4) << 1;

	return crc;
}
u16 crc16(const u8 *p, int len)
{
	int i;
	u16 crc = 0;

	for (i=0; i<len; i++)
		crc = crc16_ccitt(crc, p[i]);

	return crc;
}


//_____________________________________________________________
//_________________________SD FUNCTIONS_________________________

/*** sd functions - on top of spi code ***/

static void sd_cmd(u8 cmd, u32 arg)
{

	u8 crc = (u8) 0;
	(void) spi_txrx((u8)0x40 | cmd);
	crc = crc7_one(crc, (u8)(0x40 | cmd));
	(void) spi_txrx((u8)(arg >> 24));
	crc = crc7_one(crc, (u8)(arg >> 24));
	(void) spi_txrx((u8)(arg >> 16));
	crc = crc7_one(crc, (u8)(arg >> 16));
	(void) spi_txrx((u8)(arg >> 8));
	crc = crc7_one(crc, (u8)(arg >> 8));
	(void) spi_txrx((u8)arg);
	crc = crc7_one(crc, (u8)arg);
	/*spi_txrx(0x95);*/	/* crc7, for cmd0 */
	(void) spi_txrx(crc | 0x1);	/* crc7, for cmd0 */
}

static u8 sd_get_r1()
{
	int tries = 1000;
	u8 r;

	while (tries-- != 0) {
		r = spi_txrx((u8)0xff);
		if ((r & 0x80) == (u8)0)
			return r;
	}
	return (u8) 0xff;
}

static u16 sd_get_r2()
{
	int tries = 1000;
	u16 r = (u16) 0;

	while (tries-- != 0) {
		r = (u16) spi_txrx((u8)0xff);
		if ((r & 0x80) == (u16)0)
			break;
	}
	if (tries < 0)
		return 0xff;
	r = r<<8 | spi_txrx((u8)0xff);

	return r;
}

/*
 * r1, then 32-bit reply... same format as r3
 */
static u8 sd_get_r7(/*@out@*/u32 *r7)
   /*@modifies *r7@*/
{
	u32 r;
   u8 s;
	s = sd_get_r1();
	if (s != (u8) 0x01) {
      *r7 = 0xffffffff; /* Don't let r7 be undefined */
		return s;
   }

	r = (u32) spi_txrx((u8)0xff) << 24;
	r |= spi_txrx((u8)0xff) << 16;
	r |= spi_txrx((u8)0xff) << 8;
	r |= spi_txrx((u8)0xff);

	*r7 = r;
	return (u8) 0x01;
}
#define sd_get_r3 sd_get_r7

static const char *r1_strings[7] = {
	"in idle",
	"erase reset",
	"illegal command",
	"communication crc error",
	"erase sequence error",
	"address error",
	"parameter error"
};

static void print_r1(u8 r)
{
	unsigned int i;
	printf("R1: ");printf((unsigned int)r); printf("\n");
	for (i=0; i<7; i++)
		if ((r & (1<<i)) != 0)
			printf(r1_strings[i]);
}

static const char *r2_strings[15] = {
	"card is locked",
	"wp erase skip | lock/unlock cmd failed",
	"error",
	"CC error",
	"card ecc failed",
	"wp violation",
	"erase param",
	"out of range | csd overwrite",
	"in idle state",
	"erase reset",
	"illegal command",
	"com crc error",
	"erase sequence error",
	"address error",
	"parameter error",
};

static void print_r2(u16 r)
{
	unsigned int i;
	printf("R2: ");printf((unsigned int)r);printf("\n");
	for (i=0; i<15; i++)
		if ((r & (1<<i)) != 0)
			printf(r2_strings[i]);
}

/* Nec (=Ncr? which is limited to [0,8]) dummy bytes before lowering CS,
 * as described in sandisk doc, 5.4. */
static void sd_nec()
{
	int i;
	for (i=0; i<8; i++)
		(void)spi_txrx((u8)0xff);
}


static int sd_init(hwif *hw)
{
	unsigned int i;
	int r;
	u32 r7;
	u32 r3;
	int tries;
	u32 hcs = 0;

	hw->capabilities = 0;

	/* start with 100-400 kHz clock */
	spi_set_speed(SD_SPEED_400KHZ);

	printf("cmd0 - reset.. ");
	spi_cs_high();
	/* 74+ clocks with CS high */
	for (i=0; i<10; i++)
		(void)spi_txrx((u8)0xff);

	/* reset */
	spi_cs_low();
	sd_cmd((u8)0, 0);
	r = (int) sd_get_r1();
	sd_nec();
	spi_cs_high();

	if (r == 0xff)
		{
			LCD_I2C_Clear();
			LCD_I2C_PrintC("fail1");
			//GPIOC -> ODR ^=  ( 1 << 13 );
			goto err_spi;

		}

		if (r != 0x01)
		{
			LCD_I2C_Clear();
			LCD_I2C_Goto(2,1);
			LCD_I2C_PrintC ("fail2");
			GPIOC -> ODR ^=  ( 1 << 13 );
			GPIOC -> ODR ^=  ( 1 << 13 );
			print_r1(r);
			goto err;
		}
	printf("success\n");


	printf("cmd8 - voltage.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd((u8)8, 0x1aa /* VHS = 1 */);
	r = (int) sd_get_r7(&r7);
	sd_nec();
	spi_cs_high();
	hw->capabilities |= CAP_VER2_00;
	if (r == 0xff)
		goto err_spi;
	if (r == 0x01)
		printf("success, SD v2.x\n");
	else if ((r & 0x4) != 0) {
		hw->capabilities &= ~CAP_VER2_00;
		printf("not implemented, SD v1.x\n");
	} else {
		printf("fail\n");
		print_r1((u8)r);
		return -2;
	}


	printf("cmd58 - ocr.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd((u8)58, 0);
	r = (int) sd_get_r3(&r3);
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r != 0x01 && ((r & 0x4) == 0)) { /* allow it to not be implemented - old cards */
		printf("fail\n");
		print_r1((u8)r);
		return -2;
	}
	else {
		for (i=4; i<=23; i++)
			if ((r3 & 1<<i) != 0)
				break;
		printf("Vdd voltage window: ");printf((unsigned int)12+i);
		for (i=23; i>=4; i--)
			if ((r3 & 1<<i) != 0)
				break;
		/* CCS shouldn't be valid here yet */
      printf((unsigned int)13+i);printf("deciVolt, CCS:");
      printf((unsigned int)(r3>>30)&1); printf(" power up status: ");printf((unsigned int)r3>>31);
		printf("\nsuccess\n");
	}


	printf("acmd41 - hcs.. ");
	tries = 1000;
	/* say we support SDHC */
	if ((hw->capabilities & CAP_VER2_00) != 0)
		hcs = (u32)(1<<30);

	/* needs to be polled until in_idle_state becomes 0 */
	do {
		/* send we don't support SDHC */
		spi_cs_low();
		/* next cmd is ACMD */
		sd_cmd((u8)55, 0);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		/* well... it's probably not idle here, but specs aren't clear */
		if ((r & 0xfe) != 0) {
			printf("fail\n");
			print_r1((u8)r);
			goto err;
		}

		spi_cs_low();
		sd_cmd((u8)41, hcs);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			printf("fail\n");
			print_r1((u8)r);
			goto err;
		}
	} while (r != 0 && tries-- != 0);
	if (tries == -1) {
		printf("timeouted\n");
		goto err;
	}
	printf("success\n");

	/* Seems after this card is initialized which means bit 0 of R1
	 * will be cleared. Not too sure. */


	if ((hw->capabilities & CAP_VER2_00) != 0) {
		printf("cmd58 - ocr, 2nd time.. ");
		/* ask about voltage supply */
		spi_cs_low();
		sd_cmd((u8)58, 0);
		r = (int) sd_get_r3(&r3);
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			printf("fail\n");
			print_r1((u8)r);
			return -2;
		}
		else {
#if 1
			for (i=4; i<=23; i++)
				if ((r3 & 1<<i) != 0)
					break;
			printf("Vdd voltage window: ");printf((unsigned int)12+i);
			for (i=23; i>=4; i--)
				if ((r3 & 1<<i) != 0)
					break;
			/* CCS shouldn't be valid here yet */
         printf((unsigned int)13+i);printf("deciVolt, CCS:");
         printf((unsigned int)(r3>>30)&1); printf(" power up status: ");printf((unsigned int)r3>>31);
			/* XXX power up status should be 1 here, since we're finished initializing, but it's not. WHY?
			that means CCS is invalid, so we'll set CAP_SDHC later */
#endif
			if ((r3>>30 & 1) != 0) {
				hw->capabilities |= CAP_SDHC;
			}

			printf("success\n");
		}
	}


	/* with SDHC block length is fixed to 1024 */
	if ((hw->capabilities & CAP_SDHC) == 0) {
		printf("cmd16 - block length.. ");
		spi_cs_low();
		sd_cmd((u8)16, 512);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			printf("fail\n");
			print_r1((u8)r);
			goto err;
		}
		printf("success\n");
	}


	printf("cmd59 - enable crc.. ");
	/* crc on */
	spi_cs_low();
	sd_cmd((u8)59, 0);
	r = (int) sd_get_r1();
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if ((r & 0xfe) != 0) {
		printf("fail\n");
		print_r1((u8) r);
		goto err;
	}
	printf("success\n");


	/* now we can up the clock to <= 25 MHz */
	spi_set_speed(SD_SPEED_25MHZ);

	return 0;

 err_spi:
	printf("fail spi\n");
	return -1;
 err:
	return -2;
}

static int sd_read_status(hwif *hw)
{
	u16 r2;

	spi_cs_low();
	sd_cmd(13, 0);
	r2 = sd_get_r2();
	sd_nec();
	spi_cs_high();
	if (r2 & 0x8000)
		return -1;
	if (r2)
		print_r2(r2);

	return 0;
}

/* 0xfe marks data start, then len bytes of data and crc16 */
static int sd_get_data(hwif *hw, u8 *buf, int len)
{
	int tries = 20000;
	u8 r;
	u16 _crc16;
	u16 calc_crc;
	int i;

	while (tries--) {
		r = spi_txrx(0xff);
		if (r == 0xfe)
			break;
	}
	if (tries < 0)
		return -1;

	for (i=0; i<len; i++)
		buf[i] = spi_txrx(0xff);

	_crc16 = spi_txrx(0xff) << 8;
	_crc16 |= spi_txrx(0xff);

	calc_crc = crc16(buf, len);
	if (_crc16 != calc_crc) {
		printf("%s, crcs differ: %04x vs. %04x, len:%i\n", __func__, _crc16, calc_crc, len);
		return -1;
	}

	return 0;
}

static int sd_put_data(hwif *hw, const u8 *buf, int len)
{
	u8 r;
	int tries = 10;
	u8 b[16];
	int bi = 0;
	u16 crc;

	spi_txrx(0xfe); /* data start */

	while (len--)
		spi_txrx(*buf++);

	crc = crc16(buf, len);
	/* crc16 */
	spi_txrx(crc>>8);
	spi_txrx(crc);

	/* normally just one dummy read in between... specs don't say how many */
	while (tries--) {
		b[bi++] = r = spi_txrx(0xff);
		if (r != 0xff)
			break;
	}
	if (tries < 0)
		return -1;

	/* poll busy, about 300 reads for 256 MB card */
	tries = 100000;
	while (tries--) {
		if (spi_txrx(0xff) == 0xff)
			break;
	}
	if (tries < 0)
		return -2;

	/* data accepted, WIN */
	if ((r & 0x1f) == 0x05)
		return 0;

	return r;
}

static int sd_read_csd(hwif *hw)
{
	u8 buf[16];
	int r;
	int capacity;

	spi_cs_low();
	sd_cmd(9, 0);
	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if (r & 0xfe) {
		spi_cs_high();
		printf("%s ", __func__);
		print_r1(r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		printf("failed to get csd\n");
		return -3;
	}

	if ((buf[0] >> 6) + 1 == 1) {
	/* CSD v1 */
	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "
		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "
		"c_size:%i, vdd_rmin:%i, vdd_rmax:%i, vdd_wmin:%i, vdd_wmax:%i, "
		"c_size_mult:%i, erase_blk_en:%i, erase_s_size:%i, "
		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), /* classes, read_bl_len */
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			(buf[6]&0x3)<<10 | buf[7]<<2 | buf[8]>>6, /* c_size */
			(buf[8]&0x38)>>3, buf[8]&0x07, buf[9]>>5, (buf[9]>>2)&0x7,
			1<<(2+(((buf[9]&3) << 1) | buf[10]>>7)), /* c_size_mult */
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, /* erase sector size */
			(buf[11]&0x7f) + 1, /* write protect group size */
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), /* write_bl_len */
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 /* file format */);

	capacity = (((buf[6]&0x3)<<10 | buf[7]<<2 | buf[8]>>6)+1) << (2+(((buf[9]&3) << 1) | buf[10]>>7)) << ((buf[5] & 0xf) - 9);
	/* ^ = (c_size+1) * 2**(c_size_mult+2) * 2**(read_bl_len-9) */

	} else {
	/* CSD v2 */
		/* this means the card is HC */
		hw->capabilities |= CAP_SDHC;

	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "
		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "
		"c_size:%i, erase_blk_en:%i, erase_s_size:%i, "
		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), /* classes, read_bl_len */
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			buf[7]<<16 | buf[8]<<8 | buf[9], /* c_size */
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, /* erase sector size */
			(buf[11]&0x7f) + 1, /* write protect group size */
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), /* write_bl_len */
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 /* file format */);

	capacity = buf[7]<<16 | buf[8]<<8 | buf[9]; /* in 512 kB */
	capacity *= 1024; /* in 512 B sectors */

	}

	printf("capacity = %i kB\n", capacity/2);
	hw->sectors = capacity;

	/* if erase_blk_en = 0, then only this many sectors can be erased at once
	 * this is NOT yet tested */
	hw->erase_sectors = 1;
	if (((buf[10]>>6)&1) == 0)
		hw->erase_sectors = ((buf[10]&0x3f)<<1 | buf[11]>>7) + 1;

	return 0;
}

static int sd_read_cid(hwif *hw)
{
	u8 buf[16];
	int r;

	spi_cs_low();
	sd_cmd(10, 0);
	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if (r & 0xfe) {
		spi_cs_high();
		printf("%s ", __func__);
		print_r1(r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		printf("failed to get cid\n");
		return -3;
	}

	printf("CID: mid:%x, oid:%c%c, pnm:%c%c%c%c%c, prv:%i.%i, psn:%02x%02x%02x%02x, mdt:%i/%i\n",
			buf[0], buf[1], buf[2],			/* mid, oid */
			buf[3], buf[4], buf[5], buf[6], buf[7],	/* pnm */
			buf[8] >> 4, buf[8] & 0xf,		/* prv */
			buf[9], buf[10], buf[11], buf[12],	/* psn */
			2000 + (buf[13]<<4 | buf[14]>>4), 1 + (buf[14] & 0xf));

	return 0;
}


static int sd_readsector(hwif *hw, u32 address, u8 *buf)
{
	int r;

	spi_cs_low();
	if (hw->capabilities & CAP_SDHC)
		sd_cmd(17, address); /* read single block */
	else
		sd_cmd(17, address*512); /* read single block */

	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if (r & 0xfe) {
		spi_cs_high();
		printf("%s\n", __func__);
		print_r1(r);
		r = -2;
		goto fail;
	}

	r = sd_get_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		r = -3;
		goto fail;
	}

	return 0;
 fail:
	printf("failed to read sector %li, err:%i\n", address, r);
	return r;
}

static int sd_writesector(hwif *hw, u32 address, const u8 *buf)
{
	int r;

	spi_cs_low();
	if (hw->capabilities & CAP_SDHC)
		sd_cmd(24, address); /* write block */
	else
		sd_cmd(24, address*512); /* write block */

	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if (r & 0xfe) {
		spi_cs_high();
		printf("%s\n", __func__);
		print_r1(r);
		r = -2;
		goto fail;
	}

	spi_txrx(0xff); /* Nwr (>= 1) high bytes */
	r = sd_put_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r != 0) {
		printf("sd_put_data returned: %i\n", r);
		r = -3;
		goto fail;
	}

	/* efsl code is weird shit, 0 is error in there?
	 * not that it's properly handled or anything,
	 * and the return type is char, fucking efsl */
	return 0;
 fail:
	printf("failed to write sector %li, err:%i\n", address, r);
	return r;
}


/*** public API - on top of sd/spi code ***/
//_________________________________________________________________________
//_________________________HW INIT_________________________________________


int hwif_init(hwif* hw)
{
	int tries = 10;

	if (hw->initialized != 0)
		return 0;

	spi_init();

	while (tries-- != 0) {
		if (sd_init(hw) == 0)
			break;
	}
	if (tries == -1)
		return -1;

	/* read status register */
	(void) sd_read_status(hw);

	(void) sd_read_cid(hw);

	if (sd_read_csd(hw) != 0)
		return -1;

	hw->initialized = 1;
	return 0;
}

int sd_read(hwif* hw, u32 address, u8 *buf)
{
	int r;
	int tries = 10;

	r = sd_readsector(hw, address, buf);

	while (r < 0 && tries--) {
		if (sd_init(hw) != 0)
			continue;

		/* read status register */
		sd_read_status(hw);

		r = sd_readsector(hw, address, buf);
	}
	if (tries == -1)
		printf("%s: couldn't read sector %li\n", __func__, address);

	return r;
}

int sd_write(hwif* hw, u32 address,const u8 *buf)
{
	int r;
	int tries = 10;

	r = sd_writesector(hw, address, buf);

	while (r < 0 && tries--) {
		if (sd_init(hw) != 0)
			continue;

		/* read status register */
		sd_read_status(hw);

		r = sd_writesector(hw, address, buf);
	}
	if (tries == -1)
		printf("%s: couldn't write sector %li\n", __func__, address);

	return r;
}


//_____________________________________________________________
//_________________________FAT FS______________________________

/*** fatfs code that uses the public API ***/

#include "diskio.h"

hwif hw;

DSTATUS disk_initialize(BYTE drv)
{
	if (hwif_init(&hw) == 0)
		return 0;

	return STA_NOINIT;
}


DSTATUS disk_status(BYTE drv)
{
	if (hw.initialized)
		return 0;

	return STA_NOINIT;
}


DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
	int i;

	for (i=0; i<count; i++)
		if (sd_read(&hw, sector+i, buff+512*i) != 0)
			return RES_ERROR;

	return RES_OK;
}


#if _READONLY == 0
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
	int i;

	for (i=0; i<count; i++)
		if (sd_write(&hw, sector+i, buff+512*i) != 0)
			return RES_ERROR;

	return RES_OK;
}
#endif /* _READONLY */



DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
	switch (ctrl) {
	case CTRL_SYNC:
		return RES_OK;
	case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
		return RES_OK;
	case GET_SECTOR_COUNT:
		*(DWORD*)buff = hw.sectors;
		return RES_OK;
	case GET_BLOCK_SIZE:
		*(DWORD*)buff = hw.erase_sectors;
		return RES_OK;
	}
	return RES_PARERR;
}


/*
 * FAT filestamp format:
 * [31:25] - year - 1980
 * [24:21] - month 1..12
 * [20:16] - day 1..31
 * [15:11] - hour 0..23
 * [10:5]  - minute 0..59
 * [4:0]   - second/2 0..29
 * so... midnight 2009 is 0x3a000000
 */
/*
DWORD get_fattime()
{
	int time = RTC_GetCounter();
	int y, m, d;
	epoch_days_to_date(time/DAY_SECONDS, &y, &m, &d);
	time %= DAY_SECONDS;
	return (y-1980)<<25 | m<<21 | d<<16 |
		(time/3600)<<11 | (time/60%60)<<5 | (time/2%30);
}
*/
