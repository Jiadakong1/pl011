#include <common.h>
#include <command.h>
#define FALSE  -1
#define TRUE   0
#define FR_OK  0

/*********************************************************************
 * CONSTANTS
 */
#define SDRAM_ADDRESS           0X0c100000

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)     /* start, block, block-complement */
#define PACKET_TRAILER          (2)     /* CRC bytes */
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)
#define PACKET_TIMEOUT          (800000)   //超时时间
#define MAX_TIMEOUT_NUM         (40)       //最大超时次数


#define INITIAL

#define FILE_NAME_LENGTH (64)
#define FILE_SIZE_LENGTH (16)


#define YMODEM_RX_IDLE          0
#define YMODEM_RX_ACK           1
#define YMODEM_RX_EOT           2
#define YMODEM_RX_ERR           3
#define YMODEM_RX_EXIT          4

/* ASCII control codes: */
#define SOH (0x01)      /* start of 128-byte data packet */
#define STX (0x02)      /* start of 1024-byte data packet */
#define EOT (0x04)      /* end of transmission */
#define ACK (0x06)      /* receive OK */
#define NAK (0x15)      /* receiver error; retry */
#define CAN (0x18)      /* two of these in succession aborts transfer */
#define CNC (0x43)      /* character 'C' */


#define UART_PL01x_RSR_OE               0x08
#define UART_PL01x_RSR_BE               0x04
#define UART_PL01x_RSR_PE               0x02
#define UART_PL01x_RSR_FE               0x01

#define UART_PL01x_FR_TXFE              0x80
#define UART_PL01x_FR_RXFF              0x40
#define UART_PL01x_FR_TXFF              0x20
#define UART_PL01x_FR_RXFE              0x10
#define UART_PL01x_FR_BUSY              0x08
#define UART_PL01x_FR_TMSK              (UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

#define UART_PL01x_DR                   0x00	 /*  Data read or written from the interface. */
#define UART_PL01x_RSR                  0x04	 /*  Receive status register (Read). */
#define UART_PL01x_ECR                  0x04	 /*  Error clear register (Write). */
#define UART_PL01x_FR                   0x18	 /*  Flag register (Read only). */
#define UART_PL011_IBRD                 0x24
#define UART_PL011_FBRD                 0x28
#define UART_PL011_LCRH                 0x2C
#define UART_PL011_CR                   0x30
#define UART_PL011_IMSC                 0x38
#define UART_PL011_PERIPH_ID0           0xFE0

#define UART_PL011_LCRH_SPS             (1 << 7)
#define UART_PL011_LCRH_WLEN_8          (3 << 5)
#define UART_PL011_LCRH_WLEN_7          (2 << 5)
#define UART_PL011_LCRH_WLEN_6          (1 << 5)
#define UART_PL011_LCRH_WLEN_5          (0 << 5)
#define UART_PL011_LCRH_FEN             (1 << 4)
#define UART_PL011_LCRH_STP2            (1 << 3)
#define UART_PL011_LCRH_EPS             (1 << 2)
#define UART_PL011_LCRH_PEN             (1 << 1)
#define UART_PL011_LCRH_BRK             (1 << 0)

#define UART_PL011_CR_CTSEN             (1 << 15)
#define UART_PL011_CR_RTSEN             (1 << 14)
#define UART_PL011_CR_OUT2              (1 << 13)
#define UART_PL011_CR_OUT1              (1 << 12)
#define UART_PL011_CR_RTS               (1 << 11)
#define UART_PL011_CR_DTR               (1 << 10)
#define UART_PL011_CR_RXE               (1 << 9)
#define UART_PL011_CR_TXE               (1 << 8)
#define UART_PL011_CR_LPE               (1 << 7)
#define UART_PL011_CR_IIRLP             (1 << 2)
#define UART_PL011_CR_SIREN             (1 << 1)
#define UART_PL011_CR_UARTEN            (1 << 0)

#define UART_PL011_IMSC_OEIM            (1 << 10)
#define UART_PL011_IMSC_BEIM            (1 << 9)
#define UART_PL011_IMSC_PEIM            (1 << 8)
#define UART_PL011_IMSC_FEIM            (1 << 7)
#define UART_PL011_IMSC_RTIM            (1 << 6)
#define UART_PL011_IMSC_TXIM            (1 << 5)
#define UART_PL011_IMSC_RXIM            (1 << 4)
#define UART_PL011_IMSC_DSRMIM          (1 << 3)
#define UART_PL011_IMSC_DCDMIM          (1 << 2)
#define UART_PL011_IMSC_CTSMIM          (1 << 1)
#define UART_PL011_IMSC_RIMIM           (1 << 0)
// #define UART3_REG_BASE			0x120B0000
// #define UART2_REG_BASE			0x120A0000
// #define UART1_REG_BASE			0x12090000
// #define UART0_REG_BASE			0x12080000

volatile unsigned int * p_uart_pl011_dr =       (volatile unsigned int *)0x12080000;   //第3位和第5位改不了，可能被定义为const了
volatile unsigned int * p_uart_pl011_ecr =      (volatile unsigned int *)0x12080004;
volatile unsigned int * p_uart_pl011_fr=        (volatile unsigned int *)0x12080018;    //加了volatile之后不能间接寻址？
volatile unsigned int * p_uart_pl011_ilpr =     (volatile unsigned int *)0x12080020;
volatile unsigned int * p_uart_pl011_ibrd =     (volatile unsigned int *)0x12080024;
volatile unsigned int * p_uart_pl011_fbrd =     (volatile unsigned int *)0x12080028;
volatile unsigned int * p_uart_pl011_rtlcr_h =  (volatile unsigned int *)0x1208002c;
volatile unsigned int * p_uart_pl011_cr =       (volatile unsigned int *)0x12080030;



typedef unsigned char uint8;


/* GLOBAL VARIABLES*/
uint8 receive_status = YMODEM_RX_IDLE;
unsigned int packet_size = 0;
unsigned int seek = 0;
unsigned int packet_total_length = 0;
int file_name_len = 0;
unsigned long file_size = 0;
int start_receive = TRUE;
int end_receive = FALSE;
unsigned int time_out = FALSE;
unsigned int time_count = PACKET_TIMEOUT;
unsigned int time_out_count = 0;
unsigned int sdram_address = SDRAM_ADDRESS;
char * psdram_address = NULL;

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/
void packet_processing(char *buf);
void packet_reception(char * buf);
static int packet_check(char *buf, int len);

 //底层函数
static unsigned short crc16(const unsigned char *buf, unsigned long count);
static void getbuf_block(char* buf, unsigned int len);
static void pl011_putc_block( char ch );

static void pl011_init(void)
{
	unsigned int temp = 0;
	unsigned int divider = 0;
	unsigned int remainder = 0;
	unsigned int fraction = 0;
    unsigned int baudRate = 115200;

	/*
	 ** First, disable everything.
	 */
	*p_uart_pl011_cr = 0x0;

	/*
	 ** Set baud rate
	 **
	 ** IBRD = UART_CLK / (16 * BAUD_RATE)
	 ** FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
	temp = 16 * baudRate;
	divider = CONFIG_PL011_CLOCK / temp;
	remainder = CONFIG_PL011_CLOCK % temp;
	temp = (8 * remainder) / baudRate;
	fraction = (temp >> 1) + (temp & 1);

	*p_uart_pl011_ibrd = divider;
	*p_uart_pl011_fbrd = fraction;

	/*
	 ** Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled.
	 */
	*p_uart_pl011_rtlcr_h = UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN;

	/*
	 ** Finally, enable the UART
	 */
	*p_uart_pl011_cr = (UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE);

}


static void pl011_putc_block (char c)
{
	/* Wait until there is space in the FIFO */
	while ((*p_uart_pl011_fr) & UART_PL01x_FR_TXFF)
	   ;

	/* Send the character */
	*p_uart_pl011_dr = c;
}



static int pl011_getc_block (void)
{
	/* Wait until there is data in the FIFO */
	while ((*p_uart_pl011_fr)  & UART_PL01x_FR_RXFE)
		;

	return 	*p_uart_pl011_dr;
}


static void pl011_putc_unblock (char c)
{
	/* Wait until there is space in the FIFO */
	if((*p_uart_pl011_fr) & UART_PL01x_FR_TXFF)
	   ;
    else
	   *p_uart_pl011_dr = c;
}



static int pl011_getc_unblock (void)
{
	/* Wait until there is data in the FIFO */
	if((*p_uart_pl011_fr)  & UART_PL01x_FR_RXFE)
	   ;
    else
	   return 	*p_uart_pl011_dr;
}


// static int uart_putc(char c)
// {
//     while(1)
//     {
//         if ((*p_uart_zynq_channel) & ZYNQ_UART_SR_TXEMPTY)   //1可以发送
//         {
//             *p_uart_zynq_fifo = c;
//             break;
//         }
//     }
// 	return 0;
// }
//
//
// static int uart_getc(void)   //用crt或者xshell发送一个字符时，收到字符和换行符， x
// {
//     while(1)
//     {
//         if ((*p_uart_zynq_channel) & ZYNQ_UART_SR_RXEMPTY)
//             ;
//         else
//             return *p_uart_zynq_fifo;
//     }
// }


static void getbuf_block(char* buf, unsigned int len)
{
    int i = 0;
    printf("\ngetbuf_block:  data\n");
    for(i=0; i<len; i++){
        buf[i] = pl011_getc_block();
    }
}

static void getbuf_unblock(char* buf, unsigned int len)
{
    int i = 0;
    printf("\ngetbuf_block:  data\n");
    for(i=0; i<len; i++){
        buf[i] = pl011_getc_unblock();
    }
}

// void pl011_putc_block( char ch )
// {
//   putc(ch);
// }
//
// void getbuf(char* buf, unsigned int len)
// {
//     int i = 0;
//     printf("\ngetbuf:  data\n");
//     for(i=0; i<len; i++){
//         buf[i] = getc();
//     }
// }

static int packet_check(char *buf, int len)
{
    char ch = buf[0];

    //指令包
    if(len <= 1)
    {
        if( ch==EOT || ch==ACK || ch==NAK || ch==CAN || ch==CNC )
        {
            return (int)ch;
        }
        else
            return -1;
    }
    //数据包
    else
    {
        if( ch==SOH || ch==STX )
        {
            return (int)ch;
        }
        else
            return -1;
    }
}

static int packet_if_empty( char *buf, int len)
{
    int offset=0;
    while( ((buf[offset]==0x20)||(buf[offset]==0x30) || (buf[offset]==0x00)) &&  ++offset<len); //CRT结尾并不都是0x1a，注意
    if( offset == len )
    {
        printf("0 packet!\n");
        return TRUE;
        printf("not 0 packet!\n");
    }
    else
    return FALSE;
}


static unsigned short crc16(const unsigned char *buf, unsigned long count)
{
    unsigned short crc = 0;
    int i;

    while(count--)
    {
        crc = crc ^ *buf++ << 8;
        for (i=0; i<8; i++)
        {
            if(crc & 0x8000)
            {
                crc = crc << 1 ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc;
}


static unsigned long str16_to_u32(const char* str)
{
    const char *s = str;
    unsigned long len;
    int c;

    do {                //得到第一个字符，跳过空格
        c = *s++;
    } while (c == ' ');
    if(c == '0')
    {
        c = *s++;
    }
    else
    {
        printf("wrong number!\n");
        return -1;
    }


    if((c == 'x') || (c == 'X'))
    {
        c = *s++;
    }
    else
    {
        printf("wrong number!\n");
        return -1;
    }


    for (len = 0; ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))  ; c = *s++) {
        if(((c >= '0') && (c <= '9')) )
            c = c - '0';
        else if((c >= 'a') && (c <= 'f'))
            c = c - 'a' + 10;
        else if((c >= 'A') && (c <= 'F'))
            c = c - 'A' + 10;
        len *= 16;
        len += c;
    }
    return len;
}


static unsigned long str10_to_u32(const char* str)
{
    const char *s = str;
    unsigned long len;
    int c;

    do {                //得到第一个字符，跳过空格
        c = *s++;
    } while (c == ' ');

    for (len = 0; (c >= '0') && (c <= '9'); c = *s++) {
        c -= '0';
        len *= 10;
        len += c;
    }
    return len;
}



// static void uart_set_baudrate(unsigned long baud)
// {
// 	/* Calculation results. */
// 	unsigned int  calc_bauderror,bdiv, bgen;
// 	unsigned long calc_baud = 0;
//     unsigned long clock = 99999999;
//
// 	/* Covering case where input clock is so slow */
//
//
// 	/*                master clock
// 	 * Baud rate = ------------------
// 	 *              bgen * (bdiv + 1)
// 	 *
// 	 * Find acceptable values for baud generation.
// 	 */
// 	for (bdiv = 4; bdiv < 255; bdiv++) {
// 		bgen = clock / (baud * (bdiv + 1));
// 		if (bgen < 2 || bgen > 65535)
// 			continue;
//
// 		calc_baud = clock / (bgen * (bdiv + 1));
//
// 		/*
// 		 * Use first calculated baudrate with
// 		 * an acceptable (<3%) error
// 		 */
// 		if (baud > calc_baud)
// 			calc_bauderror = baud - calc_baud;
// 		else
// 			calc_bauderror = calc_baud - baud;
// 		if (((calc_bauderror * 100) / baud) < 3)
// 			break;
// 	}
// 	//115200 bdiv = 4, bgen = 173
// 	//9600   bdiv = 4, bgen = 2083
//     printf("bdiv = %d\t bgen = %d\n",  bdiv, bgen);
//     *p_uart_zynq_bgen = bgen;
//     *p_uart_zynq_bdiv = bdiv;
//
// 	// writel(bdiv, &regs->baud_rate_divider);
// 	// writel(bgen, &regs->baud_rate_gen);
// }

void packet_processing(char *buf){
    int i = 0;
    unsigned short crc1 = 0;
    unsigned short crc2 = 0;
    //开始接收，发送字符c
    if( TRUE == start_receive){                 //packet_reception函数接收到数据之后将start_receive置为FALSE
        time_out = FALSE;                       //开始新一次计时
        time_count = PACKET_TIMEOUT;
        pl011_putc_block('C');
        return;
    }
    //printf("status = %d\n", (int)receive_status);
    switch (receive_status)
    {
        case YMODEM_RX_IDLE:                                    //接收第一包之前的状态
            switch( packet_check( buf, packet_total_length) )   //检查当前包是否合法,并返回包的类型
            {
                case SOH:
                case STX:
                    if( TRUE == packet_if_empty( buf+3, packet_size ) )   //判断是否是空包
                    {
                        pl011_putc_block( ACK );
                        //pl011_putc_block( 0x4f);//xshell结束使用  crt不用
                        receive_status = YMODEM_RX_EXIT;
                        goto receive_exit;                      //这是在本循环必须完成的操作，所以需要用到 goto 语句
                    }
                    else                                        //如果不是空包，则认为是第一个包（包含文件名和文件大小）
                    {
                        printf("first packet!\n");
                        pl011_putc_block( ACK );
                        seek = 0;                               //初始化变量，用于接收新文件
                        pl011_putc_block( 'C' );
                        // //file_open(buf+3);                     //解析文件名
                        // file_name_len = strlen(buf+3);
                        // file_size = (unsigned int)str_to_u32( buf+3+file_name_len+1 ); //解析文件长度
                        // if(file_size < 1024)
                        //     printf("file_size = %dB\n", (int)file_size);
                        // else if(file_size/1024 < 1024)
                        //     printf("file_size = %.1fKB\n", (float)file_size/1024);
                        // else
                        //     printf("file_size = %.1fMB\n", (float)file_size/1024/1024);
                        // //printf("file_name_len = %d\t file_size = %d", file_name_len, (int)file_size);
                        receive_status = YMODEM_RX_ACK;
                    }
                    break;

                case EOT:
                    receive_status = YMODEM_RX_ERR;
                    goto err;
                    break;

                default:
                    goto err;
                    break;
            }
            break;

        case YMODEM_RX_ACK:                                      //接收到第一个包后————>接收到EOT之前
            switch (packet_check( buf, packet_total_length))
            {
                case SOH:  //数据包
                case STX:
                    //packet_size = (unsigned int)(buf[0])==SOH? PACKET_SIZE:PACKET_1K_SIZE;
                    crc1 = crc16( (unsigned char*)(buf+3), packet_size );
                    crc2 = ((unsigned short)(buf[packet_total_length-2]))*256+(unsigned short)buf[packet_total_length-1];
                    if( crc1 != crc2)
                    {
                        printf("crc wrong!\n");
                        //exit(0);
                    }


                    for(i = 3; i < packet_size+3; i++)
                    {
                        //printf("%c\t", buf[i]);
                        //if(0x1a == buf[i])   //不能传测试所有字符的文件，否则会提前退出   ??????????可以多开辟一点空间，不最后一包不处理？
                            //break;
                        *psdram_address = buf[i];
                        //printf("%c\n", *psdram_address);
                        psdram_address++;
                    }

                    //write_buf_to_file( buf+3, seek, packet_size );  //将接收的包保存
                    seek += packet_size;
                    pl011_putc_block( ACK );
                    break;

                case EOT:  //指令包
                    pl011_putc_block( NAK );
                    printf("YMODEM_RX_ACK: send NAK!\n" );
                    receive_status = YMODEM_RX_EOT;
                    break;
                case CAN:
                    receive_status = YMODEM_RX_ERR;
                    printf("YMODEM_RX_ACK: recieve CAN!\n" );
                    goto err;
                    break;
                default:
                    receive_status = YMODEM_RX_ERR;
                    goto err;
                    printf("YMODEM_RX_ACK: default!\n" );
                    break;
            }
            break;


        case YMODEM_RX_EOT:                                      //接收到第一个EOT之后————>接收到第二个EOT之前   状态变为YMODEM_RX_IDLE
            switch(packet_check( buf, packet_total_length))      //检查当前包是否合法,并返回包的类型
            {
                //指令包
                case EOT:
                    //printf("receive 2nd EOT:\n" );
                    receive_status = YMODEM_RX_IDLE;
                    //file_close();                               //文件接收完毕，关闭文件
                    pl011_putc_block( ACK );
                    pl011_putc_block( 'C' );
                    break;                    receive_status = YMODEM_RX_IDLE;

                default:
                    goto err;
                    break;
            }
            break;

receive_exit:
        case YMODEM_RX_EXIT:                                    //在YMODEM_RX_IDLE状态下收到全0数据包
            printf("receive_exit: YMODEM_RX_EXIT\n");
            receive_status = YMODEM_RX_IDLE;
            end_receive = TRUE;
            return;

        //错误状态static void uart_set_baudrate(unsigned long baud)
err:
        case YMODEM_RX_ERR:
            printf("error!\n");
            receive_status = YMODEM_RX_IDLE;
            end_receive = TRUE;
            break;

        default:
            printf("wrong!\n");
            //exit(1);
    }
}


void packet_reception(char * buf)
{
    int hdr_found = FALSE;
    //得到第一个字
    getbuf_block(buf, 1);
    if(time_out == TRUE)   //超时要返回
    {
        return;
    }
    switch (buf[0])
    {
        case SOH:
        case STX:
          hdr_found = TRUE;
          break;

        case CAN:
            packet_size = 0;
            break;

        case EOT:
            packet_size = 0;
            break;

        default:
            break;
    }

    //得到剩下的字符
    if(hdr_found == TRUE)
    {
        packet_size = (unsigned int)(buf[0])==SOH ? PACKET_SIZE : PACKET_1K_SIZE;
        getbuf_block(buf+1, packet_size+4);
        packet_total_length = packet_size + 5;
    }
    else
    {
        packet_total_length = 1;
    }
    //printf("packet_total_length = %d\n", packet_total_length );

    start_receive = FALSE;
}


void data_init(void)
{
    receive_status = YMODEM_RX_IDLE;
    packet_size = 0;
    seek = 0;
    packet_total_length = 0;
    file_name_len = 0;
    file_size = 0;
    start_receive = TRUE;
    end_receive = FALSE;
    time_out = FALSE;
    time_count = PACKET_TIMEOUT;
    time_out_count = 0;
    sdram_address = SDRAM_ADDRESS;
    psdram_address = NULL;
}

static int do_ymodem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char buf[1029] = {'0'};
    int delay = 100;  //s
    unsigned int baudrate = 115200;
    data_init();



    if(argv[1] != NULL)
        sdram_address = (unsigned int)str16_to_u32(argv[1]);
    psdram_address = (char *)sdram_address;
    printf("sdram_address = %x\n", sdram_address);

    if(argv[2] != NULL)
        baudrate = (unsigned int)str10_to_u32(argv[2]);
    printf("baudrate = %u\n", baudrate);
    //uart_set_baudrate(baudrate);

    udelay(delay * 1000000);
    printf("start:\n");
    while(1)
    {
        packet_processing(buf);
        if(end_receive == TRUE)
            break;
        packet_reception(buf);
    }
    //uart_set_baudrate(115200);
    return 0;
}


U_BOOT_CMD(
    ymodem,    5,  0,  do_ymodem,
    "ymodem, ---just for test\n",
    "ymodem, for long help.................................\n"
);

