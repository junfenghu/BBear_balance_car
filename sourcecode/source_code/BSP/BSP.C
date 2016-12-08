#include"BSP.H"

/****************************************public values*********************************************************************/
float radian_filted = 0;									//the angle which is filted
int16_t buf[3] = {0, 0, 0};									//the array to store the mpu6050 data
uint32_t distance = 0;							   			//the car's total run distance
int16_t acc_x, acc_z;										//acceleration on the x,z axis
int16_t gyro_y = 0;											//angular speed ont the y axis
float radian_pt = 0, radian_temp1 = 0, radian_temp2 = 0;	//radian_pt:the angle which is accumulated; radian_temp1:temporary of angle calculate
															//radian_temp2:temporary of angle calculate
float radian = 0;											//the angle which is calculated by the acceleration
int16_t *p;													//a pointer point to the array which store the mpu6050 data
int16_t leftspeed = 0,rightspeed = 0;						//the car's left wheel and right wheel	
pid_s sPID;													//struct to store the PID data

uint8_t flag_l = 1, flag_r = 1;
uint8_t heart_flag = 0;
uint8_t remote_flag = 0x00;
int balan_pwm;

unsigned char control_data = 0x00;
int32_t run_l = 0x00,run_r = 0x00;

char first_time_flag = 1;									//for the first time to calculate the angle of the car use the acc_x
/************************************************************************************************ 
Name��RCC_Configration 
Function:	
		  	initialize the system clock
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
void RCC_Configuration()
{
	ErrorStatus HSEStartUpStatus;
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if(HSEStartUpStatus == SUCCESS){
	    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //����FLASHԤ�����幦�ܣ�����FLASH�Ķ�ȡ�����г����б�����÷�.λ�ã�RCC��ʼ���Ӻ������棬ʱ������֮��
    	FLASH_SetLatency(FLASH_Latency_2);                    //flash��������ʱ

    	RCC_HCLKConfig(RCC_SYSCLK_Div1);  
    	RCC_PCLK2Config(RCC_HCLK_Div1);		   
		RCC_PCLK1Config(RCC_HCLK_Div2);	 	  
    	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	
    	RCC_PLLCmd(ENABLE); 
    	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)	  
       	{
       	}
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08)	  
       	{ 
       	}
	}
}

/************************************************************************************************ 
Name��GPIO_Configuration 
Function:	
		  	initialize the GPIO
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
void GPIO_Configuration()
{
/*****************************************I2C GPIO**********************************************/
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;						//SCL
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;						//SDA
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  

/***********************************PWM GPIO*******************************************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 				//pwm-left-forward
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		 				//pwm-left-back
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		 				//pwm-right-forward
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		 				//pwm-right-back
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
/***********************************SPEED**********************************************************/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;					 		//left speed
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIO_SetBits(GPIOB, GPIO_Pin_0);
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;							//right speed
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

/*************************************HEART LED***************************************************/
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;  
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
//	GPIO_Init(GPIOC, &GPIO_InitStructure); 
//	GPIO_WriteBit(GPIOC,GPIO_Pin_8,Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	//GPIO_WriteBit(GPIOB,GPIO_Pin_8,Bit_RESET);

}

/************************************************************************************************ 
Name��USART_Configuration 
Function:	
		  	initialize the usart
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
void USART_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;		//TX
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  	= GPIO_Pin_10;		//RX	
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		                           // A10 USART1_Rx  
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;                           //��������-RX
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate   = 115200;	                                //������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	                    //8λ����
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;		                //1��ֹͣλ
	USART_InitStructure.USART_Parity     = USART_Parity_No;				            //����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//Ӳ�������ƽ�ֹ
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                //���ͽ���ʹ��

	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);	
    USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);	                                                    // Enable the USARTx 
}

/************************************************************************************************ 
Name��NVIC_Configuration 
Function:	
		  	configuration the NVIC
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;
  
   /* Set the Vector Table base location at 0x08000000 */
   //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  
   /* Enable the USART1 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       //ͨ������Ϊ����1�ж�
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//ռ���ȼ� 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//used to be 0	   //�ж���Ӧ���ȼ�0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //���ж�
   NVIC_Init(&NVIC_InitStructure); 						   //��ʼ��
  
   NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//ռ���ȼ� 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //�����ȼ� 
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
   NVIC_Init(&NVIC_InitStructure); 

  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
//  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;		//ָ���ж�Դ
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //ָ����Ӧ���ȼ���2
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	        //ʹ���ⲿ�ж�ͨ��
//  NVIC_Init(&NVIC_InitStructure);
}

/************************************************************************************************ 
Name��EXTI_Configuration 
Function:	
		  	PA6,PA7 interrupt configuration
Parameters��
		   	void
Returns��
			void 
Description:
			the interrupt configuration of PA6,PA7
************************************************************************************************/
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 

  EXTI_ClearITPendingBit(EXTI_Line10);//(EXTI_Line10);

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;//EXTI_Line10;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

  EXTI_ClearITPendingBit(EXTI_Line11);

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;//EXTI_Line10;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);//
}

/************************************************************************************************ 
Name��EXTI15_10_IRQHandler 
Function:	
		  	PB10,PB11 interrupt function
Parameters��
		   	void
Returns��
			void 
Description:
			the interrupt function of PB10,PB11
************************************************************************************************/
void EXTI15_10_IRQHandler(void)
{
  
  	if(EXTI_GetITStatus(EXTI_Line10) != RESET)  //
    {
     //�����жϴ�������
	 leftspeed++;
	 /*if(leftspeed%2)	 
	 	GPIO_ResetBits(GPIOB , GPIO_Pin_0);
	 else
		GPIO_SetBits(GPIOB , GPIO_Pin_0);*/	 
	 //printf("left = %d\r\n",left);
	 EXTI_ClearFlag(EXTI_Line10);			       //����жϱ�־�����룩
     EXTI_ClearITPendingBit(EXTI_Line10);//	 
     }
	 if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	 {
	 rightspeed++;
	 if(rightspeed%2)	 
	 	GPIO_ResetBits(GPIOB , GPIO_Pin_0);
	 else
		GPIO_SetBits(GPIOB , GPIO_Pin_0);
	 //printf("right = %d\r\n",right);
	 EXTI_ClearFlag(EXTI_Line11);
	 EXTI_ClearITPendingBit(EXTI_Line11);
	 }
	
}

/************************************************************************************************ 
Name��USART1_IRQHandler 
Function:	
		  	USART1 interrupt function
Parameters��
		   	void
Returns��
			void 
Description:
			receive cmd from the bluetooth module
************************************************************************************************/
void USART1_IRQHandler(void)            
{
  u8 dat;
   
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)    //���������ݼĴ�����
  {     
    dat = USART_ReceiveData(USART1); 
	//control_data = dat;      
     //if(dat == 0x63) //���ڵ��������з�������ʱע����ʮ�����Ʒ��ͣ���ʾҲҪ��ʮ��������ʾ���������ݾͻ�������                   
	 //{  			
	  //dat = 0;
	  //printf("hello\r\n");                         			  
	 //}                                          			   
  }
} 

/************************************************************************************************ 
Name��TIM3_IRQHandler 
Function:	
		  	TIM3 interrupt function
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
void TIM3_IRQHandler(void)
{
	uint16_t res_l,res_r;
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
		res_l = leftspeed * 2;
		leftspeed = 0;
		//printf("resl = %d\r\n",res_l); 
		res_r = rightspeed * 2;
		rightspeed = 0;
		//printf("resr = %d\r\n",res_r);

		remote_flag++;
		if(remote_flag >= 10){
			remote_flag = 0;
			control_data = Remote_Scan();
		}
			
		
//		if(heart_flag >= 100)
//			 heart_flag = 0;
//		if(heart_flag >= 0 && heart_flag <= 50)
//			GPIO_SetBits(GPIOB, GPIO_Pin_5);
//		if(heart_flag > 50 && heart_flag < 100)
//			GPIO_ResetBits(GPIOB, GPIO_Pin_5);
//		heart_flag++;
	}	
}

/************************************************************************************************ 
Name��TIM2_IRQHandler 
Function:	
		  	TIM2 interrupt function to calculate the angle of the car
Parameters��
		   	void
Returns��
			void 
Description:
			null
************************************************************************************************/
u8 	RmtSta=0;	  	  
u16 Dval;		//�½���ʱ��������ֵ
u32 RmtRec=0;	//������յ�������	   		    
u8  RmtCnt=0;	//�������µĴ���

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if(RmtSta&0x80)//�ϴ������ݱ����յ���
		{	
			RmtSta&=~0X10;						//ȡ���������Ѿ���������
			if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
			if((RmtSta&0X0F)<14)RmtSta++;
			else
			{
				RmtSta&=~(1<<7);//���������ʶ
				RmtSta&=0XF0;	//��ռ�����	
			}						 	   	
		}

		p = buf;
		READ_MPU6050(p);
		acc_x = p[0];
		acc_z = p[1];
		gyro_y = p[2];

		acc_x += acc_x_offset;
		acc_z += acc_z_offset;
		gyro_y += gyro_y_offset;

		radian = (float)((float)acc_x / 8192);
		radian = asin(radian);
		radian = (radian * 180.0) / 3.1415926;

		radian_temp1 = (((float)gyro_y) / 16.4) * 0.01;	//ͨ�����ٶ��ۼƼ���Ƕ�

		//if(radian_temp1 - radian_temp2 > 0.1 || radian_temp1 - radian_temp2 <= -0.1)//֮ǰС��ƽ���������޸������кܴ��ϵ
			radian_pt += radian_temp1;

		radian_filted = Kaerman_Filter(radian_filted, -radian, radian_temp1/*radian_pt*/);		// �����˲��õ�С������б�Ƕ�20161113
		balan_pwm = PID_Cal(&sPID, -radian_filted, radian_temp1);
		if(control_data == 0x18 && (radian_filted > -STOP_BT_ANGLE && radian_filted < STOP_BT_ANGLE))
		{
			run_l = F_B;
			run_r = F_B;
		}
		else if(control_data == 0x4a && (radian_filted > -STOP_BT_ANGLE && radian_filted < STOP_BT_ANGLE)){
			run_l = -F_B;
			run_r = -F_B;
		}
		else if(control_data == 0x10 && (radian_filted > -STOP_BT_ANGLE && radian_filted < STOP_BT_ANGLE)){
			run_l = 0;//L_R;
			run_r = -L_R;
		}
		else if(control_data == 0x5a && (radian_filted > -STOP_BT_ANGLE && radian_filted < STOP_BT_ANGLE)){
			run_l = 0;//-L_R;
			run_r = L_R;
		}
		else{
			run_l = 0;
			run_r = 0;
		}
		PWM_Control(balan_pwm + run_l, balan_pwm + run_r);
		//printf("%.1lf,%.1lf,%.1lf\r\n",radian_filted,-radian,radian_pt);
		TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
	} 
	 	
	if(TIM_GetITStatus(TIM2,TIM_IT_CC4)!=RESET)
	{	  
		if(RDATA)//�����ز���
		{

			TIM_OC4PolarityConfig(TIM2,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���				
	    	TIM_SetCounter(TIM2,0);	   	//��ն�ʱ��ֵ
			RmtSta|=0X10;					//����������Ѿ�������
		}else //�½��ز���
		{			
  			 Dval=TIM_GetCapture4(TIM2);//��ȡCCR1Ҳ������CC1IF��־λ
			 TIM_OC4PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC4P=0	����Ϊ�����ز���
 			
			if(RmtSta&0X10)					//���һ�θߵ�ƽ���� 
			{
 				if(RmtSta&0X80)//���յ���������
				{
					
					if(Dval>300&&Dval<800)			//560Ϊ��׼ֵ,560us
					{
						RmtRec<<=1;	//����һλ.
						RmtRec|=0;	//���յ�0	   
					}else if(Dval>1400&&Dval<1800)	//1680Ϊ��׼ֵ,1680us
					{
						RmtRec<<=1;	//����һλ.
						RmtRec|=1;	//���յ�1
					}else if(Dval>2200&&Dval<2600)	//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
					{
						RmtCnt++; 		//������������1��
						RmtSta&=0XF0;	//��ռ�ʱ��		
					}
 				}else if(Dval>4200&&Dval<4700)		//4500Ϊ��׼ֵ4.5ms
				{
					RmtSta|=1<<7;	//��ǳɹ����յ���������
					RmtCnt=0;		//�����������������
				}						 
			}
			RmtSta&=~(1<<4);
		}

		TIM_ClearFlag(TIM2,TIM_IT_Update|TIM_IT_CC4);				 		     	    					   
	}
}

uint8_t Remote_Scan(void)
{        
	u8 sta=0;       
    u8 t1,t2;  
	if(RmtSta&(1<<6))//�õ�һ��������������Ϣ��
	{ 
	    t1=RmtRec>>24;			//�õ���ַ��
	    t2=(RmtRec>>16)&0xff;	//�õ���ַ���� 
 	    if((t1==(u8)~t2)&&t1==REMOTE_ID)//����ң��ʶ����(ID)����ַ 
	    { 
	        t1=RmtRec>>8;
	        t2=RmtRec; 	
	        if(t1==(u8)~t2)sta=t1;//��ֵ��ȷ	 
		}   
		if((sta==0)||((RmtSta&0X80)==0))//�������ݴ���/ң���Ѿ�û�а�����
		{
		 	RmtSta&=~(1<<6);//������յ���Ч������ʶ
			RmtCnt=0;		//�����������������
		}
	}  
    return sta;
}