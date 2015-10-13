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
		printf ("Open Instrument Failed!\n",reply_string);
		return -1;
	}

	/* "ASRL07::INSTR" where 07 is the usb com address */
	status = viOpen (defaultRM,"ASRL07::INSTR",VI_NULL,VI_NULL, &DataAcqu); 
	if (status < VI_SUCCESS) {
		printf ("Open Instrument failed!\n",reply_string);
		return -1;
	}

	/* *IDN? is the scpi command ask the Intrument serial number */
	status = viQueryf(DataAcqu, "*IDN?\n","%t",reply_string);
	printf ("Instrument identification string: %s\n",reply_string);

	// Query the slot status of Intrument which will return: 2638A-100,1,2638A-100,0,NONE,0 
	// 2638A-001 stands for the first slot 
	status = viQueryf(DataAcqu, "*OPT?\n","%t",reply_string);
	printf ("Slots status is: %s\n",reply_string);

	// Query the calibration date of 2638A, return formats:2013,1,1 
	viQueryf(DataAcqu, "CALibrate:DATE? \n","%t",reply_string);
	printf ("2638A calibration date is: %s\n",reply_string);

	// Query the slot's calibration date of 2638A, return format:2013,1,1 
	viQueryf(DataAcqu, "CALibrate:MODule:DATE? 1\n","%t",reply_string);
	printf ("Slot 1 calibration DATE is: %s\n",reply_string);

	viQueryf(DataAcqu, "CALibrate:MODule:DATE? 2\n","%t",reply_string);
	printf ("Slot 2 calibration DATE is: %s\n",reply_string);

	// Query the slot's serial nubmer, return format:12345678
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
	/* Query the Status of Standard Operation Register
	   after the sweep done, the five bit position will set to 1 */		
	loop:			
		viPrintf(DataAcqu, "STAT:OPER?\n");
		viScanf(DataAcqu,"%d",&status);
		//printf("stat:oper= %d ", status);
		bool value;
		value = status & 0x10;
		//printf("value = %d ",value);
		
		
	if ( !value )  //if not set, keep query
		goto loop;
	else //show the query data
	{
		viPrintf(DataAcqu, "SYST:TIME?\n");
		viScanf(DataAcqu,"%s",&reply_string);
		printf("Time = %s ", reply_string);
		
		viPrintf (DataAcqu,"DATA:READ?\n"); /* Get all the readings */ 
		viScanf(DataAcqu,"%s",&temp);/* Put readings into an array */
		printf("Data = %s\n",  temp); 
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
