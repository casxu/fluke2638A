#include <visa.h> 
#include <stdio.h> 
#include <string.h> 
#include <afx.h>
ViSession defaultRM; /* Resource manager id */ 
ViSession DataAcqu; /* Variable to identify an instrument */ 
char reply_string [256]= {0}; /* string returned from instrument */ 
char temp [100]= {0}; 
int count;
float t; 
int srqFlag = {0};
ViStatus status;


int Query_Instrument()
{
	status = viOpenDefaultRM (&defaultRM); 
	if (status < VI_SUCCESS) {
		printf ("Open Instrument failed!\n",reply_string);
		return -1;
	}

	status = viOpen (defaultRM,"ASRL07::INSTR",VI_NULL,VI_NULL, &DataAcqu); 
	if (status < VI_SUCCESS) {
		printf ("Open Instrument failed!\n",reply_string);
		return -1;
	}

	status = viQueryf(DataAcqu, "*IDN?\n","%t",reply_string);
	printf ("Instrument identification string: %s\n",reply_string);

	//查询仪器插槽模块的安装状态 返回值：2638A-100,1,2638A-100,0,NONE,0 
	// 2638A-001 模块代号1表示安装了 0表示未安装
	status = viQueryf(DataAcqu, "*OPT?\n","%t",reply_string);
	printf ("Slots status is: %s\n",reply_string);

	//查询仪器的校准日期 返回值：2013,1,1 
	viQueryf(DataAcqu, "CALibrate:DATE? \n","%t",reply_string);
	printf ("2638A calibration date is: %s\n",reply_string);

	//查询仪器插槽模块1的校准日期 返回值：2013,1,1 
	viQueryf(DataAcqu, "CALibrate:MODule:DATE? 1\n","%t",reply_string);
	printf ("Slot 1 calibration DATE is: %s\n",reply_string);

	viQueryf(DataAcqu, "CALibrate:MODule:DATE? 2\n","%t",reply_string);
	printf ("Slot 2 calibration DATE is: %s\n",reply_string);

	//查询仪器模块1的序列号 返回值：12345678
	viQueryf(DataAcqu, "SYSTem:MODule:CONFigure:SNUM? 1\n","%t",reply_string);
	printf ("Slot 1 serial Number is: %s\n",reply_string);

	viQueryf(DataAcqu, "SYSTem:MODule:CONFigure:SNUM? 2\n","%t",reply_string);
	printf ("Slot 2 serial Number is: %s\n",reply_string);


	viClose (DataAcqu); /* Close the communication port */ 
	viClose (defaultRM); 
	
}

void pressure () 
{ 
	/* Open communication with DataAcqu using usb-rs232 com address "7" */ 
	viOpenDefaultRM (&defaultRM); 
	viOpen (defaultRM,"ASRL07::INSTR",VI_NULL,VI_NULL, &DataAcqu); 
	
	viQueryf(DataAcqu, "*IDN?\n","%t",reply_string);
	printf ("Instrument identification string: %s\n",reply_string);
	
	/* Reset instrument to power-on and clear the Status Byte */ 
	viPrintf (DataAcqu, "*RST;*CLS\n");
	
	viPrintf (DataAcqu, "STAT:OPER:ENAB 272\n");
	viPrintf (DataAcqu, "*ESE 1\n");
	viPrintf (DataAcqu, "*SRE 255\n");
	
	viPrintf (DataAcqu, "CONF:CURR 0.1, 1e-5, (@121)\n");
	viPrintf (DataAcqu, "CALC:SCAL:GAIN 37500, (@121)\n");
	viPrintf (DataAcqu, "CALC:SCAL:OFFS -148.4, (@121)\n");	
	viPrintf (DataAcqu, "CALC:SCAL:UNIT 'KPA', (@121)\n");
	
	viPrintf (DataAcqu, "CALC:SCAL:STAT ON, (@121)\n");
	viPrintf (DataAcqu, "ROUT:SCAN (@121)\n");
	viPrintf (DataAcqu, "TRIG:SOUR TIM\n"); 
	viPrintf (DataAcqu, "TRIG:TIMER 5\n");
	viPrintf (DataAcqu, "TRIG:COUNT 5\n"); 
	viPrintf (DataAcqu, "INIT;*OPC\n"); 		
	
	for (count = 0; count <5; count++) 
	{
		
	loop:			
		viPrintf(DataAcqu, "STAT:OPER?\n");
		viScanf(DataAcqu,"%d",&status);
		//printf("stat:oper= %d ", status);
		bool value;
		value = status & 0x10;
		//printf("value = %d ",value);
		
		//viPrintf(DataAcqu, "*STB?\n");
		//viScanf(DataAcqu,"%d",&status);
		//printf("STB寄存器值 = %d\n", status);
		
	if ( !value )
		goto loop;
	else
	{
		viPrintf(DataAcqu, "SYST:TIME?\n");
		viScanf(DataAcqu,"%s",&reply_string);
		printf("当前时间 = %s ", reply_string);
		
		viPrintf (DataAcqu,"DATA:READ?\n"); /* Get all the readings */ 
		viScanf(DataAcqu,"%s",&temp);/* Put readings into an array */
		printf("压力读数 = %s\n",  temp); 
	}
	
	}
	
	viClose (DataAcqu); /* Close the communication port */ 
	viClose (defaultRM); 
}



void main () 
{
	int result;
	result = Query_Instrument();
	pressure();

}