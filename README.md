## 程序说明
- **linux_ymodem:** linux环境下通过Ymodem协议接收文件代码
- **pl011_driver:** pl011串口驱动及纯c环境下Ymodem协议代码
- **uboot_ymodem:** uboot环境下通过添加命令的方式实现Ymodem协议，类似loady



# <center>串口总结</center >
## 1. 基本概念
### 1.1. 串口
&#8195;&#8195;串行接口简称串口，也称串行通信接口或串行通讯接口，是采用串行通信方式的扩展接口。Windows下串口设备抽象为COM口，Linux下串口设备抽象为tty。
### 1.2. UART
&#8195;&#8195;UART：universal asynchronous receiver and transmitter通用异步收发器，一般由波特率产生器(产生的波特率等于传输波特率的16倍)、UART接收器、UART发送器组成，硬件上由两根线，一根用于发送，一根用于接收。
&#8195;&#8195;既然是“器”，显然，它就是个设备而已，要完成一个特定的功能的硬件，它本身并不是协议。UART主要功能为 ：
+ **串并转换：**  
&#8195;&#8195;将由计算机内部传送过来的并行数据转换为输出的串行数据流。将计算机外部来的串行数据转换为字节，供计算机内部使用并行数据的器件使用。

+ **启停、校验：**  
&#8195;&#8195;在输出的串行数据 流中加入奇偶校验位，并对从外部接收的数据流进行奇偶校验。在输出数据流中加入启停标记，并从接收数据流中删除启停标记。

+ **缓冲：**  
&#8195;&#8195;有一些比较高档的UART还提供输入输出数据的缓冲区，现在比较新的UART 是16550，它可以在计算机需要处理数据前在其缓冲区内存储16字节数据。


### 1.3. USART
&#8195;&#8195;USART：universal synchronous asynchronous receiver and transmitter通用同步异步收发器。USART支持同步模式，因此USART 需要同步始终信号USART_CK（如STM32 单片机），通常情况同步信号很少使用，因此一般UART和USART使用方式是一样的，都使用异步模式。

### 1.4. RS232/RS485
&#8195;&#8195;RS-232与RS-485标准只对接口的电气特性做出规定，而不涉及接插件、电缆或协议，在此基础上用户可以建立自己的高层通信协议。
+ **RS-232特性：**  
    电压：-3到-15V为逻辑”1”，3到15V为逻辑”0”  
    最大距离：数十米  
    最大波特率：115200bps  
    是否差分信号：否  
    工作模式：全双工（两根线）  
    TTL电平转换芯片：MAX232  

+ **RS-485特性：**  
电压：2到6V为逻辑”1”，-2到-6V为逻辑”0”  
最大距离：数千米  
最大波特率：10Mbps  
是否差分信号：是  
工作模式：半双工（两根线）  
TTL电平转换芯片：MAX485  

+ **RS-232 需要用TTL转换原因:**  
&#8195;&#8195;RS-232 是用正负电压来表示逻辑状态，与TTL以高低电平表示逻辑状态的规定不同。因此，为了能够同计算机接口或终端的TTL器件连接，必须在EIA RS-232C 与TTL电路之间进行电平和逻辑关系的变换。

+ **TTL电平信号：**  
&#8195;&#8195;+5V等价于逻辑“1”，0V等价于逻辑“0”，这被称做TTL（晶体管-晶体管逻辑电平）信号系统，是计算机处理器控制的设备内部各部分之间通信的标准技术。

+ **RS485最高速率比RS232快的原因：**  
&#8195;&#8195;RS485采用差分信号，抗干扰性好。RS232也能设置更高波特率，但数据传输会出错。

+ **UART与RS232区别：**  
&#8195;&#8195;UART就相当于车站，而RS232则对应于公路的规则。
### 1.5. DB9接口
&#8195;&#8195;按照接口数量细分为A型（15针），B型（25针），C型（37针），D型（50针），E型（9针）。因此常见的计算机并口即为DB25针的连接器。而串口则应为DE9针连接器。  
&#8195;&#8195;由于早期的计算机的串口与并口都是使用DB25针连接器，而人们则习惯把字母B与D合在一起记了下来，当作D型接口的共同名字，以至于后来计算机串口改用9针接口以后，人们更多的使用DB9而不是DE9来称呼9针的接口。   
\
**电脑上常见的DB接口有：**   
+ 显示器 VGA 15针 -- DB15  
+ 并行打印机 LPT 25孔 -DB25  
+ 串行通信接口RS232 9针 --DB9 一般通讯只用到3口，TXD, RXD, GND  

![](http://img.blog.csdn.net/20170825120248919)


<font color=#ff0000 size=4>注意：</font>DB9转换头有公头母头区别，也有直连和交叉区别。交叉会使引脚2和3上的信号互换，若接口引脚如图所示，经过交叉转换头之后引脚2成为接收端，引脚3成为发送端。
### 1.6. 数据格式
![](http://img.blog.csdn.net/20170825120228900)
+ **数据位**  
&#8195;&#8195;这是衡量通信中实际数据位的参数。当计算机发送一个信息包，实际的数据不会是8位的，标准的值是5、6、7和8位。如何设置取决于你想传送的信息。比如，标准的ASCII码是0～127（7位）。扩展的ASCII码是0～255（8位）。

+ **停止位**  
&#8195;&#8195;用于表示单个包的最后一位。典型的值为1，1.5和2位。由于数据是在传输线上定时的，并且每一个设备有其自己的时钟，很可能在通信中两台设备间出现了小小的不同步。因此停止位不仅仅是表示传输的结束，并且提供计算机校正时钟同步的机会。适用于停止位的位数越多，不同时钟同步的容忍程度越大，但是数据传输率同时也越慢。

- **奇偶校验位**  
&#8195;&#8195;在串口通信中一种简单的检错方式。有四种检错方式：偶、奇、高和低。当然没有校验位也是可以的。对于偶和奇校验的情况，串口会设置校验位（数据位后面的一位），用一个值确保传输的数据有偶个或者奇个逻辑高位。
### 1.7. 串口通信流控制
&#8195;&#8195;“流”指的是数据流。数据在两个串口之间传输时，常常会出现丢失数据的现象，或者两台计算机的处理速度不同，如台式机与单片机之间的通信，接收端数据缓冲区已满，则此时继续发送来的数据就会丢失。  
&#8195;&#8195;流控制能解决这个问题，当接收端数据处理不过来时，就发出“不再接收”的信号，发送端就停止发送流控制，直到收到“可以继续发送”的信号再发送数据。因此流控制可以控制数据传输的进程，防止数据的丢失。
PC机中常用的两种是硬件流控制（包括RTS/CTS、DTR/CTS等）和软件流控制XON/XOFF（继续/停止），下面分别说明。

+ **硬件流控制**  
&#8195;&#8195;硬件流控制常用的有RTS/CTS流控制和DTR/DSR（数据终端就绪/数据设置就绪）流控制。 硬件流控制必须将相应的电缆线连上，用RTS/CTS（请求发送/清除发送）流控制时，应将通讯两端的RTS、CTS线对应相连，数据终端设备（如计算机）使用RTS来起始调制解调器或其它数据通讯设备的数据流，而数据通讯设备（如调制解调器）则用CTS来起动和暂停来自计算机的数据流。  
&#8195;&#8195;这种硬件握手方式的过程为：我们在编程时根据接收端缓冲区大小设置一个高位标志（可为缓冲区大小的75％）和一个低位标志（可为缓冲区大小的25％），当缓冲区内数据量达到高位时，我们在接收端将CTS线置低电平（送逻辑0），当发送端的程序检测到CTS为低后，就停止发送数据，直到接收端缓冲区的数据量低于低位而将CTS置高电平。RTS则用来标明设备有没有准备好接收数据。

+ **软件流控制**  
&#8195;&#8195;由于电缆线的限制，我们在普通的控制通讯中一般不用硬件流控制，而用软件流控制。一般通过XON/XOFF来实现软件流控制。  
&#8195;&#8195;常用方法是：当接收端的输入缓冲区内数据量超过设定的高位时，就向数据发送端发出XOFF字符，发送端收到XOFF字符后就立即停止发送数据；当接收端的输入缓冲区内数据量低于设定的低位时，就向数据发送端发出XON字符，发送端收到XON字符后就立即开始发送数据。应该注意，若传输的是二进制数据，标志字符也有可能在数据流中出现而引起误操作，这是软件流控制的缺陷，而硬件流控制不会有这个问题。

## 2. Linux下串口编程
### 2.1. 打开串口
```
fd = open( "/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
```
- **fd:**          文件描述符  
- **/dev/ttyS1:**    设备文件  
- **O_RDWR:**    读写打开  
- **O_NOCTTY:**  不将串口作为控制终端，用于数据传输。  
- **O_NDELAY:**   IO操作设置为非阻塞方式，例如：read函数直接返回，如果buffer中数据小于要读取的数据长度，将剩余的数据读取出来，返回读取的长度，不会等待接收完才返回。

### 2.2. 设置串口
#### 2.2.1. Termio结构体
&#8195;&#8195;Linux使用 termio结构体对串口进行配置。在<termios.h>中，有如下定义
```
struct termios {
     tcflag_t c_iflag;  /* input mode flags */
     tcflag_t c_oflag;  /* output mode flags */
     tcflag_t c_cflag;  /* control mode flags */
     tcflag_t c_lflag;  /* local mode flags */
     cc_t c_line;   /* line discipline */
     cc_t c_cc[NCCS];  /* control characters */
};
```  



- **c_iflag:** 由终端设备驱动程序用来控制输入特性（剥除输入字节的第8位，允许输入奇偶校验等等）  
- **c_oflag:** 则控制输出特性（执行输出处理，将新行映射为C R / L F等）  
- **c_cflag:** 影响到UART串行线（忽略调制解调器的状态线，每个字符的一个或两个停止位等等）  
- **c_lflag:** 影响驱动程序和用户之间的界面（回送的开或关，可视的擦除符，允许终端产生的信号，对后台作业输出的控制停止信号等）。
#### 2.2.2. 设置波特率
```
void set_speed(int fd, int speed)
{
       int   i;
       int   status;
       struct termios   Opt;
       tcgetattr(fd, &Opt);  //得到设备对应的termios，保存到opt,修改opt，再写回。
       for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
       {
             if  (speed == name_arr[i])
             {
                  tcflush(fd, TCIOFLUSH);
                  cfsetispeed(&Opt, speed_arr[i]);  //改变opt中表示输入速度元素的值
                  cfsetospeed(&Opt, speed_arr[i]);
                  status = tcsetattr(fd, TCSANOW, &Opt);  //根据opt设置设备termios值
                  if  (status != 0)
                  {
                           perror("tcsetattr fd");
                           return;
                  }
                  tcflush(fd,TCIOFLUSH);
             }
       }
}
```
#### 2.2.3. 设置参数
```
int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    if  ( tcgetattr( fd,&options)  !=  0)
    {
         perror("SetupSerial 1");
         return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*///设置为普通模式，当串口作为中断时，设置为标准模式
    options.c_oflag  &= ~OPOST;   /*Output*/

    //options.c_iflag &= ~ (IXON | IXOFF| BRKINT | ISTRIP  | IXANY | ICRNL | IGNCR);//自己加的，屏蔽软件流控  如果不屏蔽，0x11接收不到
    options.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    switch (databits)    /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size/n"); return (FALSE);
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;break;
        default:
            fprintf(stderr,"Unsupported parity/n");
            return (FALSE);
    }
    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
             options.c_cflag &= ~CSTOPB;
             break;
        case 2:
             options.c_cflag |= CSTOPB;
             break;
        default:
              fprintf(stderr,"Unsupported stop bits/n");
              return (FALSE);
    }
  /* Set input parity option */
    if ((parity != 'n') && (parity != 'N'))
        options.c_iflag |= INPCK;
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 0; /* 设置超时15 seconds*/
    options.c_cc[VMIN] = 0;//13; /* define the minimum bytes data to be readed*/

    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
        return (TRUE);
}
```


### 2.3. 读写数据
&#8195;&#8195;打开串口时将IO操作设置为非阻塞模式，read读取的数据长度可能小于期望读取的数据长度，因此需要循环读取数据。  
&#8195;&#8195;循环读取数据，超时退出的代码如下：  
```

int   __getbuf(char* buf, size_t len)
{
    int nread = 0;
    int be_left = len;
    int i = 0;
    //printf("\n__getbuf:  data\n");
    while(1)
    {
        //usleep(5000);
        nread = read(fd, &buf[len-be_left], be_left);
        if(nread > 0)
        {
            time_count = PACKET_TIMEOUT;//设置超时时间
            // for(i=len-be_left; i<len-be_left+nread; i++)//打印已经接收的数据
            // {
            //     printf("%x ", buf[i]);     //%x以16进制显示
            // }
            be_left = be_left - nread;
        }
        else
        {
            //超时
            time_count--;
            if(time_count <= 0)
            {
                time_out = TRUE;
                //printf("time out !      len = %d\t be_left = %d\n", len, be_left);
                time_out_count++;
                if(time_out_count >= MAX_TIMEOUT_NUM)
                {
                    printf("time out !\n");
                    exit(1);
                }
                break;
            }
        }
        if(be_left == 0)
           break;
    }
    //printf("\n");
    return len - be_left;
}
```


### 2.4. 关闭串口
```
    close(fd);
```

### 2.5. 参考资料
+ **《UNIX坏境高级编程》第3版：** 第3章介绍了open、read、write、close等函数。第18章详细介绍了termio结构体，对串口参数进行配置时可以参考这一章内容。
+ http://blog.csdn.net/destinyhot/article/details/6704795


## 3. PL011驱动
### 3.1. 寄存器
&#8195;&#8195;串口寄存器被映射到某个逻辑地址区域，通过对这些地址区域可对串口进行配置。驱动中用到的寄存器地址如下：（可以通过结构体实现）
```
#define UART0_REG_BASE			0x12080000

volatile unsigned int * p_uart_pl011_dr =       (volatile unsigned int *)(UART0_REG_BASE);  
volatile unsigned int * p_uart_pl011_ecr =      (volatile unsigned int *)(UART0_REG_BASE + 0x4);
volatile unsigned int * p_uart_pl011_fr=        (volatile unsigned int *)(UART0_REG_BASE + 0x18);    
volatile unsigned int * p_uart_pl011_ilpr =     (volatile unsigned int *)(UART0_REG_BASE + 0x20);
volatile unsigned int * p_uart_pl011_ibrd =     (volatile unsigned int *)(UART0_REG_BASE + 0x24);
volatile unsigned int * p_uart_pl011_fbrd =     (volatile unsigned int *)(UART0_REG_BASE + 0x28);
volatile unsigned int * p_uart_pl011_rtlcr_h =  (volatile unsigned int *)(UART0_REG_BASE + 0x2c);
volatile unsigned int * p_uart_pl011_cr =       (volatile unsigned int *)(UART0_REG_BASE + 0x30);
```
&#8195;&#8195;各寄存器的意义可参考pl011的技术手册。
### 3.2. 初始化
### 3.3. 发送接收函数
接收函数，阻塞
```
static int pl011_getc_block (void)
{
	/* Wait until there is data in the FIFO */
	while ((*p_uart_pl011_fr)  & UART_PL01x_FR_RXFE)
		;
	return 	*p_uart_pl011_dr;
}
```

接收函数，非阻塞
```
static int pl011_getc_unblock (void)
{
	/* Wait until there is data in the FIFO */
	if((*p_uart_pl011_fr)  & UART_PL01x_FR_RXFE)
	   return 1000;      //错误，字符不会出现1000,用来判断是否接收到字符
    else
	   return 	*p_uart_pl011_dr;
}
```

发送函数，阻塞
```
static void pl011_putc_block (char c)
{
	/* Wait until there is space in the FIFO */
	while ((*p_uart_pl011_fr) & UART_PL01x_FR_TXFF)
	   ;
	/* Send the character */
	*p_uart_pl011_dr = c;
}	/* Wait until there is space in the FIFO */

```

### 3.4. 汇编函数
初始化
```
.text
.align	2
.global	uart_pl011_init
.type	uart_pl011_init, %function
uart_pl011_init:
	ldr	a4, =0x12080000@uart_base_addr_L0
	mov	a3, #0
	/* Disable UART */
	str	a3, [a4, #48]
	/* Set baud rate to 115200, uart clock: ????打印出来的 */
	add	a3, a3, #67
	str	a3, [a4, #36]
	mov	a3, #52
	str	a3, [a4, #40]
	/* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled. */
	movw	a3, #112
	str	a3, [a4, #44]
	/* Enable UART */
	movw	a3, #769
	str	a3, [a4, #48]
	bx	lr
```

发送函数
```
.align	2
.global	uart_pl011_putc
.type	uart_pl011_putc, %function
uart_pl011_putc:
	ldr	a2, =0x12080000 @uart_base_addr_L3
wait3:
	ldr	a4, [a2, #24]
	tst	a4, #32
	bne	wait3
	str	a1, [a2, #0]

	bx	lr
```



## 4. Xmodem, Ymodem, Zmodem

### 4.1. Xmodem
&#8195;&#8195;Xmodem协议是一种串口通信中广泛用到的异步文件传输协议。分为标准Xmodem和1k-Xmodem两种，前者以128字节块的形式传输数据，后者字节块为1k即1024字节，并且每个块都使用一个校验和过程来进行错误检测。在校验过程中如果接收方关于一个块的校验和与它在发送方的校验和相同时，接收方就向发送方发送一个确认字节 (ACK)。由于Xmodem需要对每个块都进行认可，这将导致性能有所下降，特别是延时比较长的场合，这种协议显得效率更低。

### 4.2. Ymodem
&#8195;&#8195;Ymodem是使用更大数据块以追求更高效率的调制解调器所使用的一种纠错协议。使用这种Ymodem协议的调制解调器以每块1024个字节来发送数据。采用Ymodem协议的调制解调器以1024字节数的块发送数据。成功接收的不会被确认。有错误的块被确认（NAK），并重发。Ymodem很类似于Xmodem-1K，但是它还提供一种批模式。在这种批模式下可以只用一条命令来发送许多文件。

### 4.3. Zmodem
&#8195;&#8195;与上两种协议不同，可以连续的数据流发送数据，效率更高。Zmodem采用了串流式（streaming）传输方式，传输速度较快，而且还具有自动改变区段大小和断点续传、快速错误侦测等功能。  
&#8195;&#8195;Zmodem协议非常有效，它不是等待肯定的确认后再发送下一个块，而是快速连续地发送块另外，Zmodem协议包含一种名为检查点重启的特性，如果通信链接在数据传输过程中中断，能从断点处而不是从开始处恢复传输。


### 4.4. Ymodem协议流程
#### 4.4.1. Ymodem通信发送数据包的说明
- **数据包的格式**  
&#8195;&#8195;数据包的格式是: 类型 + 序号 + 序号反码 + 数据区(128或1024) + 校验和(两字节)

- **序号增长规律**  
&#8195;&#8195;数据包的序号从00开始直到255，然后又从00开始。不大于255K字节的数据包，所有发送数据包的序号是唯一的。大于255K的数据包，按照1K字节分包，第一个数据包序号是1，第二个数据包的序号是2，第255个数据包的序号是255，第256个数据包的序号，又从0开始。

- **大小数据包**  
&#8195;&#8195;Ymodem通信有128字节和1024字节两种类型的数据包，数据包开头有3字节(类型+ 序号+序号反码),末尾crc16是2个字节，所以可以说有133字节的小数据包和1029字节的大数据包。

- **133字节数据包的特点、字符填充规则**  
&#8195;&#8195;以SOH(0x01)开始的数据包，数据区是128字节。发送端第一个含有文件信息的数据包是3+128+2 = 133字节。发送最后一个数据包时，剩余数据字节数若小于128，则以0x1A填充，仍发送133字节数据包。  

- **1029字节数据包的特点、字符填充规则**  
&#8195;&#8195;以STX(0x02)开始的数据包，数据区是1024字节。若发送的文件大于1024字节，文件信息包之后的第一个数据包则为1029字节，随后剩余的数据若不小于1024字节，则均以1029大数据包发送。

#### 4.4.2. 文件传输过程
&#8195;&#8195;YModem协议传输的完整的握手过程如下图所示：  


![](http://img.blog.csdn.net/20170825120205236?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvSmlhZGFrb25nMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
1. 开启是由接收方开启传输，它发一个大写字母C开启传输。然后进入等待（SOH）状态，如果没有回应，就会超时退出。
1. 发送方开始时处于等待过程中，等待C。收到C以后，发送（SOH）数据包开始信号，发送序号（00），补码（FF），“文件名”，“空格”“文件大小”“除去序号外，补满128字节”，CRC校验两个字节。进入等待（ACK）状态。第一包数据只能是128字节，不能是1024字节。
1. 接收方收到以后，CRC校验满足，则发送ACK。发送方接收到ACK，又进入等待“文件传输开启”信号，即重新进入等待“C”的状态。
1. 前面接收方只是收到了一个文件名，限制正式开启文件传输，Ymodem支持128字节和1024字节一个数据包。128字节以（SOH）开始，1024字节以（STX）开始。接收方又发出一个“C”信号，开始准备接收文件。进入等待“SOH”或者“STX”状态。

1. 发送接收到“C”以后，发送数据包，（SOH）（01序号）（FE补码）（128位数据）（CRC校验），等待接收方“ACK”。
1. 文件发送完以后，发送方发出一个“EOT”信号，接收方也以“ACK”回应。然后接收方会再次发出“C”开启另一次传输，若接着发送方会发出一个“全0数据包”，接收方“ACK”以后，本次通信正式结束。


## 5. 支持Ymodem协议的工具
+ Windows: 	xshell、SecureCRT
+ Linux:   	minicom、lrzsz
+ Uboot:  	loady（只能接收）
