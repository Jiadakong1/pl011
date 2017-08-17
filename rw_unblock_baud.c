#include <common.h>
#include <command.h>
#define FALSE  -1
#define TRUE   0

/*********************************************************************
 * CONSTANTS
 */
#define SDRAM_ADDRESS           0X42000000

#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)
#define PACKET_TIMEOUT          (3000000)   //超时时间
#define MAX_TIMEOUT_NUM         (15)       //最大超时次数

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
#define UART0_REG_BASE			0x12080000

volatile unsigned int * p_uart_pl011_dr =       (volatile unsigned int *)(UART0_REG_BASE);   //第3位和第5位改不了，可能被定义为const了
volatile unsigned int * p_uart_pl011_ecr =      (volatile unsigned int *)(UART0_REG_BASE + 0x4);
volatile unsigned int * p_uart_pl011_fr=        (volatile unsigned int *)(UART0_REG_BASE + 0x18);    //加了volatile之后不能间接寻址？
volatile unsigned int * p_uart_pl011_ilpr =     (volatile unsigned int *)(UART0_REG_BASE + 0x20);
volatile unsigned int * p_uart_pl011_ibrd =     (volatile unsigned int *)(UART0_REG_BASE + 0x24);
volatile unsigned int * p_uart_pl011_fbrd =     (volatile unsigned int *)(UART0_REG_BASE + 0x28);
volatile unsigned int * p_uart_pl011_rtlcr_h =  (volatile unsigned int *)(UART0_REG_BASE + 0x2c);
volatile unsigned int * p_uart_pl011_cr =       (volatile unsigned int *)(UART0_REG_BASE + 0x30);

typedef unsigned char uint8;

/* GLOBAL VARIABLES*/
uint8 receive_status = YMODEM_RX_IDLE;
unsigned int packet_size = 0;
unsigned int seek = 0;
unsigned int packet_total_length = 0;
char file_name[50] = {0};
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

 static unsigned long str16_to_u32(const char* str)
 {
     const char *s = str;
     unsigned long len;
     int c;
	 if(str == NULL)
	 	return -1;

     do {                //得到第一个字符，跳过空格
         c = *s++;
     } while (c == ' ');
     if(c == '0')
     {
         c = *s++;
     }
     else
     {
         printf("Wrong input number!\n");
         return -1;
     }

     if((c == 'x') || (c == 'X'))
     {
         c = *s++;
     }
     else
     {
         printf("Wrong input number!\n");
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


 static unsigned int str10_to_u32(const char* str)
 {
     const char *s = str;
     unsigned long len;
     int c;
	 if(str == NULL)
	 	return -1;
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


 static int pl011_strlen(char *str)
 {
     char *str1 = str;
     unsigned int len = 0;
     if(str1 == NULL)
         return -1;
     while(*str1++ != '\0')
         len++;
     return len;
 }


 static int pl011_strcpy(const char *src, char *des)
 {
     if((src == NULL) || (des == NULL))
         return -1;
     while(*des++ = *src++);
 }


static void pl011_init(unsigned int baudRate)
{
	unsigned int temp = 0;
	unsigned int divider = 0;
	unsigned int remainder = 0;
	unsigned int fraction = 0;
    //unsigned int baudRate = 115200;

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
	//  printf("divider = %d\n", divider );
	//  printf("fraction = %d\n", fraction);
}


static void pl011_putc_block (char c)
{
	/* Wait until there is space in the FIFO */
	while ((*p_uart_pl011_fr) & UART_PL01x_FR_TXFF)
	   ;
	/* Send the character */
	*p_uart_pl011_dr = c;
}


static void pl011_puts_block (const char *s)
{
	char *str = s;
	while((str!= NULL) && (*str != '\0') )
	{
		pl011_putc_block(*str);
		str++;
	}

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
	   return 1000;      //错误，字符不会出现1000,用来判断是否接收到字符
    else
	   return 	*p_uart_pl011_dr;
}


static void getbuf_block(char* buf, unsigned int len)
{
    int i = 0;
    printf("\ngetbuf_block:  data\n");
    for(i=0; i<len; i++)
    {
        buf[i] = pl011_getc_block();
    }
}


static void getbuf_unblock(char* buf, int len)
{
    int nread = 0;
    int be_left = len;
    int i = 0;
    int temp = 0;
    while(1)
    {
        temp = pl011_getc_unblock();
        if(temp != 1000)
        {
            buf[i++] = (char)temp;
            time_count = PACKET_TIMEOUT;//设置超时时间
            be_left = be_left - 1;
        }
        else
        {
            //超时
            time_count--;
            if(time_count <= 0)
            {
                time_out = TRUE;
                time_out_count++;
                if(time_out_count >= MAX_TIMEOUT_NUM)
                {
                    printf("Timed Out !\n");
                    end_receive = TRUE;
                    return;
                }
                break;
            }
        }
        if(be_left == 0)
           break;
    }
}


static int packet_check(const char *buf, int len)
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


static int packet_if_empty(const char *buf, int len)
{
    int offset=0;
    while( ((buf[offset]==0x20)||(buf[offset]==0x30) || (buf[offset]==0x00)) &&  ++offset<len); //CRT结尾并不都是0x1a，注意
    if( offset == len )
        return TRUE;
    else
        return FALSE;         //printf("not 0 packet!\n");
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


static void packet_processing(char *buf){
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
                        //printf("first packet!\n");
                        pl011_putc_block( ACK );
						//printf("first packet!\n");
                        seek = 0;                               //初始化变量，用于接收新文件
                        pl011_putc_block( 'C' );

                        pl011_strcpy(buf+3,file_name);                    //解析文件名
                        file_name_len = pl011_strlen(buf+3);
                        file_size = (unsigned int)str10_to_u32( buf+3+file_name_len+1 ); //解析文件长度

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
                    crc1 = crc16( (unsigned char*)(buf+3), packet_size );
                    crc2 = ((unsigned short)(buf[packet_total_length-2]))*256+(unsigned short)buf[packet_total_length-1];
                    if( crc1 != crc2)
                    {
                        printf("crc wrong!\n");
						receive_status = YMODEM_RX_ERR;
	                    goto err;
                    }

                    if( file_size < seek + packet_size )      //最后一包处理
                        packet_size = file_size - seek;

                    for(i = 3; i < packet_size+3; i++)
                    {
                        *psdram_address = buf[i];
                        psdram_address++;
                    }
                    seek += packet_size;
                    pl011_putc_block( ACK );
                    break;

                case EOT:  //指令包
                    pl011_putc_block( NAK );
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
                    receive_status = YMODEM_RX_IDLE;
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
            receive_status = YMODEM_RX_IDLE;
            end_receive = TRUE;
            return;
err:
        case YMODEM_RX_ERR:
            printf("error!\n");
            receive_status = YMODEM_RX_IDLE;
            end_receive = TRUE;
            return;

        default:
            printf("wrong!\n");
			return;
    }
}


static void packet_reception(char * buf)
{
    int hdr_found = FALSE;
    //得到第一个字
    getbuf_unblock(buf, 1);
    if(time_out == TRUE)   //超时要返回
    {
        return;
    }

	if(buf[0] == 3)  // ctrl + c 结束传输
	{
		end_receive = TRUE;
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
            return;   //return ymodem起来之后不管按什么键都没用，   break:按任意键退出
    }

    //得到剩下的字符
    if(hdr_found == TRUE)
    {
        packet_size = (unsigned int)(buf[0])==SOH ? PACKET_SIZE : PACKET_1K_SIZE;
        getbuf_unblock(buf+1, packet_size+4);
        packet_total_length = packet_size + 5;
    }
    else
    {
        packet_total_length = 1;
    }

    start_receive = FALSE;
}


static void data_init(void)
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


void ymodem(void){
	char buf[1029] = {'0'};
	unsigned int baudrate = 115200;
	data_init();
	psdram_address = (char *)sdram_address;
 	pl011_init(baudrate);

	while(1)
	{
		packet_processing(buf);
		if(end_receive == TRUE)
		{
			break;
		}
		packet_reception(buf);
	}

}

static int do_ymodem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char buf[1029] = {'0'};
    unsigned int baudrate = 115200;
	data_init();

	if(argc > 1)
		sdram_address = (unsigned int)str16_to_u32(argv[1]);
	psdram_address = (char *)sdram_address;

	if(argc > 2)
		baudrate = (unsigned int)str10_to_u32(argv[2]);
	pl011_init(baudrate);

    printf ("## Ready for binary (ymodem) download "
        "to 0x%08lX at %d bps...\n",
        sdram_address,
        baudrate);

    while(1)
    {
        packet_processing(buf);
        if(end_receive == TRUE)
        {
            printf("\n\n");
            printf("## File Name = %s\n", file_name);
            if(file_size < 1024)
                printf("## Total Size = %dB\n", (int)file_size);
            else
                printf("## Total Size = %dKB\n", file_size/1024);
            break;
        }
        packet_reception(buf);
    }
    return 0;
}


U_BOOT_CMD(
    ymodem,    5,  0,  do_ymodem,
    "load binary file over serial line (ymodem mode)",
	"[ off ] [ 115200 ]\n"
	"    - load binary file over serial line"
	" with offset 'off' and baudrate '115200'"
);

