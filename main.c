 #include<reg52.h>
 #include <intrins.h>

 /*宏定义*/
 #define uchar unsigned char
 #define uint unsigned int

 /*端口定义部分*/
 sbit Rx = P3^2;      //回声接收端口
 sbit Tx = P1^0;      //超声触发端口
 sbit Beep = P2^0 ;	  // 蜂鸣器　

 /*变量及数组定义部分*/
 uchar code table[]={0x18,0x7b,0x2c,0x29,0x4b,0x89,0x88,0x3b,0x08,0x09,0x40,0x8e};
 uchar code wei_table[]={0xe9,0xd9,0xb9,0x79};
 uint outcomeH=0;outcomeL=0;
 uchar disbuff[4];
 uchar succeed_flag=0,flag_300ms=0;
 long int distance=0;

 
 /*函数声明部分*/
 void Init(void);
 void Delayms(uint);
 void Alarm(uchar t);
 void Display(uchar num[]);
 void Measure_T(void);
 long Measure_R(void);
 void Conversion(long int number,uchar wei[]);
 void delay_12us();


 /*主函数main()*/
 void main()
 {   
   Init();
   while(1)
   {
     if(flag_300ms==1)
	 {
	   flag_300ms=0;
	   Measure_T();
	   distance=Measure_R();
	   if(distance<100)
	     Alarm(1);
	   Conversion(distance,disbuff);
	 
	 }
	 
   }
 }
  /*系统初始化*/
 void Init(void)
 {											   
   	TMOD=0x11;
	TR0=1;        //启动定时器0
    IT0=0;        //由高电平变低电平，触发外部中断
    ET0=1;        //打开定时器0中断
    EX0=0;        //关闭外部中断
	TR0=1;
    EA=1;         //打开总中断0 
	Tx=0;
 }
  /*延时函数*/
 void Delayms(uint xms)
 {
   uint m,j;
   for(m=xms;m>0;m--)
     for(j=110;j>0;j--);
 }
  /*蜂鸣器*/
 void Alarm(uchar t)  // t表示发声的次数 
 {
   uchar n;
   for(n = 0;n < t;n++)
   {
     Beep = 0;
	 Delayms(200);
     Beep = 1;
   }
 }
 /*数码管显示*/
 void Display(uchar num[])
 {
   static uchar i;
  
   i++;
   if(i>=4)
     i=0;
   P2=wei_table[i];
   if(i==3)
   {  
      if(num[i]==3)
	    P0=0x21;
	  else if(num[i]==2)
	         P0=0x24;
	  	   else if(num[i]==1)
     	          P0=0x73;
	            else
	           	  P0=0x10;
   }
   else
   {
     P0=table[num[i]];
   }
 }
  /*超声波发送*/
 void Measure_T(void)
 {
   Tx=1;
   delay_12us();
   Tx=0;           //产生一个12us的脉冲，在Tx引脚  
 }
  /*回波检测*/
 long int Measure_R(void)
 {
   long int distance_data;
   while(Rx==0);   //等待Rx回波引脚变高电平
   succeed_flag=0; //清测量成功标志
   EX0=1;          //打开外部中断
   TH1=0;          //定时器1清零
   TL1=0;          //定时器1清零
   TF1=0;          //定时器1清除标志位
   TR1=1;          //启动定时器1
   EA=1;			 //启动总中断
   while(TH1 < 60);//等待测量的结果，周期65.535毫秒（可用中断实现）  
   TR1=0;          //关闭定时器1
   EX0=0;          //关闭外部中断
   if(succeed_flag==1)
   {  
     distance_data=outcomeH*256+outcomeL;                //测量结果的高8位
     distance_data/=6;                  //微秒的单位除以58等于厘米     
   }                                      
   if(succeed_flag==0)
   {
     distance_data=0;                    //没有回波则清零
   }

   return distance_data;
 }
  /*数字分解，用于数码管显示*/
 void Conversion(long int number,uchar wei[])
 {
   wei[3]=number/1000;
   wei[2]=number/100%10;
   wei[1]=number%100/10;
   wei[0]=number%10;
 }
  /*延时函数 12us*/
 void delay_12us()
 {
   _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
   _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 }
  /*外部中断0，用于检测回波*/
 INTO_()  interrupt 0   // 外部中断是0号（应该不是一个固定电平吧？）
 {    
     outcomeH=TH1;    //取出定时器的值
     outcomeL=TL1;    //取出定时器的值
     succeed_flag=1;   //至成功测量的标志
     EX0=0;            //关闭外部中断
 }
  /*定时器0中断 2ms*/
  Timer0() interrupt 1 //定时器0中断是1号
 {
   static uchar Flag_2ms,Beep_num;
   TH0=(65536-2000)/256;
   TL0=(65536-2000)%256;
   Flag_2ms++;
  
   Display(disbuff); 
  
   if(Flag_2ms>=150)
   {
     Flag_2ms=0;
	 flag_300ms=1;
   }

 }
  //定时器1中断,用做超声波测距计时
 Timer1() interrupt 3 //定时器0中断是1号
 {
   TH1=0;
   TL1=0;
 }