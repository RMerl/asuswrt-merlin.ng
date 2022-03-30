#include <sys/ioctl.h>
#include <errno.h>                                                              
#include <string.h>                                                             
#include <stdio.h>                                                              
#include <stdlib.h>                                                             
#include <unistd.h>                                                             
#include <fcntl.h>                                                              
#include <time.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define MAX_BLK_SIZE 64
#define EEPROM_SIZE 32768
#define READ 1
#define WRITE 0
#define ERASE 2
#define PHEADER 3
#define VER       "eepromer v 0.4 (c) Daniel Smolik 2001\n"
#define HEAD_SIZE   sizeof(struct mini_inode)
#define START_ADDR   0
#define FORCE        1
/*
To disable startup warning #undef WARNINC


*/

#define  WARNINC     
 

int block_write(int file,int dev_addr,int eeprom_addr,unsigned char *buf,int lenght);int block_write(int file,int dev_addr,int eeprom_addr,unsigned char *buf,int lenght);
int block_read(int file,int dev_addr,int eeprom_addr,unsigned char *buf);

/* block_read read block 64 bytes length and returns actual length of data*/
void help(void);
int init(char *device,int addr);
int content_write(int file, int addr);
int content_read(int file, int addr);
int inode_write(int file, int dev_addr, int lenght);
int inode_read(int file, int dev_addr, void *p_inode);
void pheader(int file, int addr);
void  erase(int file,int addr,int eeprom_size);	
void made_address(int addr,unsigned char *buf);
void warn(void);
void bar(void);


static int stav=0;



static	struct  mini_inode {
	
			time_t  timestamp;
			int		data_len;
			char    data[56];
	
	} m_ind,*p_ind;



void help(void)                                                                 
{                                                                               
  FILE *fptr;                                                                   
  char s[100];                                                                  
    
	fprintf(stderr,"Syntax: eepromer [-r|-w|-e|-p]  -f /dev/i2c-X  ADDRESS \n\n");   
	fprintf(stderr,"  ADDRESS is address of i2c device eg. 0x51\n");

	if((fptr = fopen("/proc/bus/i2c", "r"))) {                                    
	fprintf(stderr,"  Installed I2C busses:\n");                                
	while(fgets(s, 100, fptr))                                                  
		fprintf(stderr, "    %s", s);                                             
	fclose(fptr);                                                               
	}
}	 





int main(int argc, char *argv[]){

	int i, file, addr;
	int  action; //in this variable will be (-r,-w,-e)
	char device[45];
	int force;

	p_ind=&m_ind;
	force=0;



	
	for(i=1; i < argc;i++){
	
		
		if(!strcmp("-r",argv[i])) {
			 action=READ;
			 break;
		}	 
		if(!strcmp("-e",argv[i])) {
			 action=ERASE;
			 break;
		}	 
		if(!strcmp("-w",argv[i])) { 
			action=WRITE;
			break;
		}
		if(!strcmp("-p",argv[i])) { 
			action=PHEADER;
			break;
		}	
		if(!strcmp("-force",argv[i])) { 
			force=FORCE;
			break;
		}	
		if(!strcmp("-v",argv[i])) { 
			fprintf(stderr,VER);
			exit(0);
			break;
		}	
		else {
		
			fprintf(stderr,"Error: No action specified !\n");
			help();
			exit(1);
		}

	}	


#ifdef  WARNINC
	
	if(force!=FORCE) warn();
	
#endif
	

	if(argc < 5) {
		fprintf(stderr,"Error: No i2c address specified !\n");
		help();
		exit(1);
	
	}

	
	for(i=1; i < argc;i++){
	
		
		if(!strcmp("-f",argv[i])) {
			 strcpy(device,argv[i+1]);	 
			 break;
		}	 

	}	

	if(!strlen(device)) {

			fprintf(stderr,"Error: No device specified !\n");
			help();
			exit(1);
	}


	if(! (addr=strtol(argv[4],NULL,16))) {
	
		fprintf(stderr,"Error: Bad device address !\n");
		help();
		exit(1);
	}

	if(! (file=init(device,addr))){
	
		fprintf(stderr,"Error: Init failed !\n");
		exit(1);
	}	


	switch(action){
	
		case READ:  
						content_read(file,addr);
						break;
		
		case WRITE: 
						content_write(file,addr);
						break;
		
		case ERASE: 	erase(file,addr,EEPROM_SIZE);
						break;
		case PHEADER: 	pheader(file,addr);
						break;			
					
		default:
			fprintf(stderr,"Internal error!\n");
			exit(1); break;
	
	}


	close(file);
	exit(0);

}



/****************************************************************************/
/*            Low level function	                                        */	
/*																			*/
/****************************************************************************/





int block_write(int file,int dev_addr,int eeprom_addr,unsigned char *buf,int lenght){

		unsigned char buff[2];
		struct i2c_msg msg[2];
		
		struct i2c_ioctl_rdwr_data {
	
	 		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
	    	int nmsgs;             /* number of messages to exchange */ 
		} msgst;
	


		if ( lenght > (MAX_BLK_SIZE) ) {
		
			fprintf(stderr,                                                             
	                  "Error: Block too large:\n"); 
		
		}


		//bar();

		made_address(eeprom_addr,buff);
		
			
		msg[0].addr = dev_addr;
		msg[0].flags = 0;
		msg[0].len = 2;
		msg[0].buf = buff;
	
	
		msg[1].addr = dev_addr;
		msg[1].flags = I2C_M_NOSTART;
		msg[1].len = lenght;
		msg[1].buf = buf;


		msgst.msgs = msg;	
		msgst.nmsgs = 2;
	
		
		if (ioctl(file,I2C_RDWR,&msgst) < 0){

				fprintf(stderr,                                                             
	                  "Error: Transaction failed: %s\n",      
	                              strerror(errno)); 

			return 1;
	                                                                                
		}

       return 0;
	
}




int block_read(int file,int dev_addr,int eeprom_addr,unsigned char *buf){

	int ln;
	char buff[2]; //={0x0,0x0};
	
	struct i2c_msg msg[2];
		
	struct i2c_ioctl_rdwr_data {
	
	 		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
	    	int nmsgs;             /* number of messages to exchange */ 
	} msgst;


	
	made_address(eeprom_addr,buff);
	ln=0;
	//bar();	
	
	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buff;
	
	
	msg[1].addr = dev_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = MAX_BLK_SIZE;
	msg[1].buf = buf;

	
	

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	
	
	

	if ((ln = ioctl(file, I2C_RDWR, &msgst)) < 0) {

			fprintf(stderr,                                                             
	                  "Error: Read error:%d\n",ln);
	   return ln;                     
	}
	
	return ln;

}
















void made_address(int addr,unsigned char *buf){

		int k;
		
		//addr = addr & 0xFFFF; /*odstranim nepoterbne bity*/

		k=addr;
		buf[1]=(unsigned char) (k & 0xFF); //vyrobim druhy byte adresy
		k=addr & 0xFF00 ;
		buf[0]= ((unsigned char) (k >> 8)) & 0x7F;
	
		
	return;
}


int init(char *device,int addr) { 

	int file;	
	unsigned long funcs;

	if ((file = open(device,O_RDWR)) < 0) {
	
  	 	fprintf(stderr,"Error: Could not open file %s\n",                
		                    device);             
	
		return 0;
	}


	/* check adapter functionality */                                             
	if (ioctl(file,I2C_FUNCS,&funcs) < 0) {                                       
	      fprintf(stderr,                                                             
	                  "Error: Could not get the adapter functionality matrix: %s\n",      
	                              strerror(errno));                                                   
		 close(file);		
	     return 0;                                                                    
	}             

	/* The I2C address */                                        
	if (ioctl(file,I2C_SLAVE,addr) < 0) {                                         
	      /* ERROR HANDLING; you can check errno to see what went wrong */            
		fprintf(stderr,                                                             
	                  "Error: Cannot communicate with slave: %s\n",      
	                              strerror(errno));                                                   

		close(file);	      
		return 0;                                                                    
	}       

	return file;
}


int content_write(int file, int addr){

	unsigned char buf[MAX_BLK_SIZE];
	unsigned char pom; 
	int i, j, k, delka, addr_cnt;
	
	delka=0;
	addr_cnt=HEAD_SIZE;
	k=0;

	for(j=0;j<MAX_BLK_SIZE;j++)
		buf[j]=0;



	i=0;

	for(;;) {
	
		delka=fread(&pom,1,1,stdin);

		if( delka > 0 ){
			buf[i]=pom;
		}

		if(i==(MAX_BLK_SIZE-1) || (delka < 1)) {
			
				
						
	 		if(block_write(file,addr,addr_cnt,buf,delka<1?i:(i+1)) !=0) {
	 
	 			fprintf(stderr,"Block write failed\n");      
	 			return 1;
	 
	 		}
	 		//printf("i:%d\n",i);
			addr_cnt=addr_cnt + i + (delka==1?1:0); //+i
			
			for(j=0;j<MAX_BLK_SIZE;j++)
				buf[j]=0;

			i=0;
			if(delka<1) {
			
				//pisu EOF
				
				
	 			if(inode_write(file,addr,(addr_cnt-HEAD_SIZE)) !=0) {
					 
	 				fprintf(stderr,"Inode write failed\n");      
	 				return 1;
	 			
	 			}
				break;
			}			
			
			
		} else  i++;

	}

	return 0;
	
}


int content_read(int file, int addr){

	unsigned char buf[MAX_BLK_SIZE];
	int i, j, k, delka;
	
	delka=0;
	k=0;
	
	
	inode_read(file,addr,p_ind );


	for(i=HEAD_SIZE;i<= (HEAD_SIZE + p_ind->data_len);i=i+MAX_BLK_SIZE ) {
	
	
	 		if(block_read(file,addr,i,buf) !=0) {
	 
	 			fprintf(stderr,"Block read failed\n");      
	 			return 1;
	 
	 		}
		
			if( (HEAD_SIZE + p_ind->data_len - i) < MAX_BLK_SIZE ) {
				k= HEAD_SIZE + p_ind->data_len - i;
			}else {
				k=MAX_BLK_SIZE;
			}
			
					
			for(j=0;j<k ;j++){

					putc(buf[j],stdout);
				
			}
			
		
	}

	return 0;
	
}



void erase(int file, int addr,int eeprom_size){

	unsigned char buf[MAX_BLK_SIZE];
	int i, j, k, delka;
	
	delka=0;
	k=0;

	for(j=0;j<MAX_BLK_SIZE;j++)
		buf[j]=0;





	for(i=0;i<eeprom_size;i=i+MAX_BLK_SIZE) {
	

	 		if(block_write(file,addr,i,buf,MAX_BLK_SIZE) !=0) {
	 
	 			fprintf(stderr,"Block write failed\n");      
	 			return;
	 
	 		}

	}

	return;
	
}



void bar(void){


	if( stav > 70 ) stav=0;
	
	
		switch(stav) {

		
			case 10: fwrite("\\",1,1,stderr);
						fflush(stderr); 
						rewind(stderr);
						break;
			case 20: fwrite("|",1,1,stderr); 
						fflush(stderr);
						rewind(stderr); 
						break;
			case 30: fwrite("/",1,1,stderr); 
						fflush(stderr); 
						rewind(stderr);
						break;
			case 40: fwrite("-",1,1,stderr);
						fflush(stderr); 
						rewind(stderr);
						break;
			case 50: fwrite("\\",1,1,stderr);
						fflush(stderr); 
						rewind(stderr);
						break;
			case 60: fwrite("|",1,1,stderr);
						fflush(stderr); 
						rewind(stderr);
						break;
			case 70: fwrite("/",1,1,stderr);
						fflush(stderr); 
						rewind(stderr);
						break;
		}
	stav++;

}





int  inode_write(int file,int dev_addr,int lenght){

		unsigned char buff[2];
		struct i2c_msg msg[2];
		
		struct i2c_ioctl_rdwr_data {
	
	 		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
	    	int nmsgs;             /* number of messages to exchange */ 
		} msgst;
	
		
		m_ind.timestamp=time(NULL);
		m_ind.data_len=lenght;





		//bar();

		made_address(START_ADDR,buff);
		
			
		msg[0].addr = dev_addr;
		msg[0].flags = 0;
		msg[0].len = 2;
		msg[0].buf = buff;
	
	
		msg[1].addr = dev_addr;
		msg[1].flags = I2C_M_NOSTART;
		msg[1].len = sizeof(struct mini_inode);
		msg[1].buf = (char *) &m_ind;

	

		msgst.msgs = msg;	
		msgst.nmsgs = 2;
	
		
		if (ioctl(file,I2C_RDWR,&msgst) < 0){

				fprintf(stderr,                                                             
	                  "Error: Transaction failed: %s\n",      
	                              strerror(errno)); 

			return 1;
	                                                                                
		}

       return 0;
	
}



int inode_read(int file,int dev_addr,void *p_inode ){

	
	#define  POK  32
	int ln;
	char buff[2]; //={0x0,0x0};
	
	struct i2c_msg msg[2];
		
	struct i2c_ioctl_rdwr_data {
	
	 		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
	    	int nmsgs;             /* number of messages to exchange */ 
	} msgst;
	
	made_address(START_ADDR,buff);
	
	ln=0;
	//bar();	
	
	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buff;
	
	
	msg[1].addr = dev_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = sizeof(struct mini_inode);
	msg[1].buf = p_inode;

	
	

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	

	if ((ln = ioctl(file, I2C_RDWR, &msgst)) < 0) {

			fprintf(stderr,                                                             
	                  "Error: Read error:%d\n",ln);
	   return ln;                     
	}


	
	return ln;

}


void pheader(int file,int dev_addr){

	struct tm *p_tm;
	char time_buf[15],*p_buf;

	p_buf=time_buf;
	inode_read(file,dev_addr,p_ind );
	p_tm=localtime(&p_ind->timestamp);
	strftime(p_buf,sizeof(time_buf),"%Y%m%d%H%M%S",p_tm);
	printf("LEN=%d,TIME=%s\n",p_ind->data_len,p_buf);
	return;
}




#ifdef WARNINC
void warn(void)
{

	fprintf(stderr,"\n\n!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf(stderr,"This program is intended for use on eeproms\nusing external busses such as i2c-pport.\n");
	fprintf(stderr,"Do not use this on your SDRAM DIMM EEPROMS\nunless you REALLY REALLY know what you are\ndoing!!! Doing so will render your SDRAM\nUSELESS and leave your system UNBOOTABLE!!!\n"); 
	fprintf(stderr,"To disable this warning use -force\n");
	fprintf(stderr,"\n\nPress  ENTER  to continue or hit Control-C NOW !!!!\n\n\n");                                 

	getchar();
}
#endif
