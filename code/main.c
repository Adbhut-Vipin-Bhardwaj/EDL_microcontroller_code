#include "REGX51.H"

// ranges for times:
//////////////////////////////////////////////////////////////////////
unsigned long int	t_75_to_50_range[2] = {62260, 67040}; // 31.12ms to 33.52ms
unsigned long int	t_50_to_25_range[2] = {106760, 113840}; // 53.38ms to 56.92ms
unsigned long int	op_duration[4] = {4000000, 6000000, 8000000, 10000000}; // operations for dist b/w claps
//																	P1.4   |  P1.5  |  P1.6  |  P1.7
//																	2s  	 |  3s    |  4s		 |  5s
//////////////////////////////////////////////////////////////////////
unsigned char timer0_overflows = 0;
unsigned char timer1_overflows = 0;
unsigned char num_clap = 0;
bit reset_to_start = 0;

// interrupt handlers
void timer0_interrupt() interrupt 1{ // interrupt address = 000B, so number = (11-3)/8
	if(timer0_overflows == 0x01){
		// reset timer0_overflows, stop timer, not in a clap
		timer0_overflows = 0;
		TR0 = 0;
		TH0 = 0; TL0 = 0;
		reset_to_start = 1;
	}else{
		timer0_overflows++;
	}
}
void timer1_interrupt() interrupt 3{ // interrupt address = 001B, so number = (16+11-3)/8
	if(timer1_overflows == 0xB7){
		// reset timer1_overflows, stop timer, not in a clap pair
		timer1_overflows = 0;
		TR1 = 0;
		TH1 = 0; TL1 = 0;
		num_clap = 0;
	}else{
		timer1_overflows++;
	}
}

void delay(int num){
	// when num = 500, 1505021 machine cycles, 0.752 seconds
	int i,j;
	for (i=0; i<num; i++){
		for (j=0; j<500; j++){}
	}
}

void multi_clap_handler_func(unsigned long int clap_gap){
	if ((clap_gap >= (1 * 2000000)) && (clap_gap < op_duration[0])){
		P1_4 = 1;
		delay(500);
		P1_4 = 0;
	}else if ((clap_gap >= op_duration[0]) && (clap_gap < op_duration[1])){
		P1_5 = 1;
		delay(500);
		P1_5 = 0;
	}else if ((clap_gap >= op_duration[1]) && (clap_gap < op_duration[2])){
		P1_6 = 1;
		delay(500);
		P1_6 = 0;
	}else if ((clap_gap >= op_duration[2]) && (clap_gap < op_duration[3])){
		P1_7 = 1;
		delay(500);
		P1_7 = 0;
	}else{}
}

void main(){
	bit t_75_to_50_ok, t_50_to_25_ok;
	unsigned long int t_75_to_50, t_50_to_25;
	unsigned long int clap_gap = 0;
	// set up timers, interrupts
	TMOD = 0x11; // both timers in 16 bit mode
	IE = 0x8A; // enable timer0, timer1 interrupt
	P1 = 0x03; // switch off LEDs initially
	while(1){
		while(!P1_0){}
		// start timer0
		TR0 = 1;
		while(P1_0){
			// 75% - 50% pulse
			if(reset_to_start){
				continue;
			}
		}
		if(reset_to_start){reset_to_start = 0; continue;}
		// stop timer0
		TR0 = 0;
		// store value t_75_to_50
		t_75_to_50 = ((((unsigned long int) timer0_overflows) << 16) & 0x00FF0000);
		t_75_to_50 += ((((unsigned long int) TH0) << 8) & 0x0000FF00);
		t_75_to_50 += (((unsigned long int) TL0) & 0x000000FF);
		timer0_overflows = 0; TH0 = 0; TL0 = 0;
		if(!P1_1){continue;}
		// start timer0
		TR0 = 1;
		while(P1_1){
			// 50% - 25% pulse
			if(reset_to_start){
				continue;
			}
		}
		if(reset_to_start){reset_to_start = 0; continue;}
		// stop timer0
		TR0 = 0;
		// store value t_50_to_25
		t_50_to_25 = ((((unsigned long int) timer0_overflows) << 16) & 0x00FF0000);
		t_50_to_25 += ((((unsigned long int) TH0) << 8) & 0x0000FF00);
		t_50_to_25 += (((unsigned long int) TL0) & 0x000000FF);
		timer0_overflows = 0; TH0 = 0; TL0 = 0;
		
		// check if t_75_to_50, t_50_to_25 in range
		
		t_75_to_50_ok = ((t_75_to_50 >= t_75_to_50_range[0]) && (t_75_to_50 <= t_75_to_50_range[1]));
		t_50_to_25_ok = ((t_50_to_25 >= t_50_to_25_range[0]) && (t_50_to_25 <= t_50_to_25_range[1]));
		
		if (t_75_to_50_ok && t_50_to_25_ok){
			//clap detected
			// check num_clap,
			// 		if 0 => num_clap++, start timer1, start over with P1_0
			// 		if 1 => stop timer1, go_to multi_clap_handler_func; num_clap = 0;
			// send signal single clap detected
			P1_2 = 1;
			delay(500); // 0.752 seconds, 1505021 machine cycles
			P1_2 = 0;
			if(num_clap == 0){
				num_clap++;
				TR1 = 1;
			}else if(num_clap == 1){
				TR1 = 0;
				clap_gap = ((((unsigned long int) timer1_overflows) << 16) & 0x00FF0000);
				clap_gap += ((((unsigned long int) TH1) << 8) & 0x0000FF00);
				clap_gap += (((unsigned long int) TL1) & 0x000000FF);
				clap_gap -= (t_75_to_50 + t_50_to_25 + 1505021);
				multi_clap_handler_func(clap_gap);
				num_clap = 0;
				timer1_overflows = 0;
				TH1 = 0; TL1 = 0;
			}else{}
		}else{
			//clap not detected
			// go back to P1_0
		}
	}
}