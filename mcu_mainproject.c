/*
 * pj_ljh.c
 *
 * Created: 2020-11-30 오후 4:05:14
 * Author : B815148 jihyeon lee
 */ 

#define F_CPU	16000000//16MHz
#include <avr/io.h>//입출력 헤더파일
#include <avr/interrupt.h>//인터럽트 헤더파일
#include <util/delay.h>//delay함수 헤더파일
#include <string.h>

#define STOP	0
#define START	1
#define INIT	2

#define C1	523		// 도
#define D1	587		// 레
#define E1	659		// 미
#define F1	699		// 파
#define G1	784		// 솔
#define A1	880		// 라
#define B1	988		// 시
#define C2	C1*2	// 도
#define D2	D1*2	// 레
#define E2	E1*2	// 미
#define F2	F1*2	// 파
#define G2	G1*2	// 솔
#define A2	A1*2	// 라
#define B2	B1*2	// 시
#define DLY_0	32				// 온음표*2
#define DLY_1	16				// 온음표
#define DLY_2	8				// 2분 음표
#define DLY_4	4				// 4분 음표
#define DLY_8	2				// 8분 음표
#define DLY_16	1				// 16분 음표			
	
	
//mode selector
unsigned char mode_sel=0;

// display공통
const unsigned char Segment_Data[] =
{0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x27,0x7F,0x6F};
char COLUMN[4]={0,0,0,0};

//clock	
unsigned char count_int=0;
unsigned int Seconds=0, Minutes=0, Hours=0;	

//stopwatch, down_clock
int state=STOP;
int SHOW_NUMBER=0, SHOW_NUMBER12=0, SHOW_NUMBER34=0;

//piano
volatile int Doremi[8]={C1,D1,E1,F1,G1,A1,B1,C2};//"도레미파솔라시도"
volatile int Doremi_length[8]={DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4};
volatile unsigned char TIMERvalue=0xFF;
volatile int freq = 1000, i,j;
//piano2
volatile int school[100]={G1,G1,A1,A1,G1,G1,E1,G1,G1,E1,E1,D1,G1,G1,A1,A1,G1,G1,E1,G1,E1,D1,E1,C1};
volatile int school_length[100]={DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_1,DLY_4,DLY_4,DLY_4,DLY_4,DLY_0,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_4,DLY_1,DLY_4,DLY_4,DLY_4,DLY_4,DLY_0,};

volatile int SanToKi[100]={G1,E1,E1,G1,E1,C1,D1,E1,D1,C1,E1,G1,C2,G1,C2,G1,C2,G1,E1,G1,D1,F1,E1,D1,C1};
volatile int SanToKi_length[100]={DLY_1,DLY_4,DLY_4,DLY_4,DLY_4,DLY_1,DLY_1,DLY_4,DLY_4,DLY_4,DLY_4,DLY_1,DLY_2,DLY_4,DLY_2,DLY_4,DLY_4,DLY_4,DLY_1,DLY_1,DLY_4,DLY_4,DLY_4,DLY_4,DLY_1,};


// display
void Show_Display(unsigned int number);
void Show4Digit(int number);
void ShowDigit(int i, int digit);
// mode
void mode0_clock();
void mode1_stop_watch();
void mode2_piano();
void mode3_down_clock();
void mode4_piano2();
void mode5_uart();
//stopwatch
void Run(void);
// piano
void Shimpyo(int time);
void Cutoff_Play(void);
void piano_up(void);
//count down clock
void down_clock_Run();
//piano2
void schoolsong(void);
void SanToKisong(void);
//uart
void uart0_init(void);
int Putchar(char message);
int Print(char *str, char length);

ISR(TIMER0_OVF_vect)
{
	cli();//전역 인터럽트 비허용
	count_int++;//1초마다 값이 하나 증가
	if(count_int == 244)//244번에 한번씩 second 증가. 244.14Hz/244=1Hz, 즉 1초
	{
		PORTG ^= 0x03;//포트G의 0, 1번 핀인 led를 1초마다 토글.
		Seconds++;//mode0의 digital clock은 계속 작동
		count_int=0;
	}
	sei();
}

ISR(TIMER2_OVF_vect)
{
	if(mode_sel==2)//piano mode
	{
		TCNT2= TIMERvalue;
		PORTB ^= 0x10;//스피커 토글
	}
	if(mode_sel==4)//piano2 mode
	{
		TCNT2= TIMERvalue;
		PORTB ^= 0x10;
	}	
}

ISR(INT0_vect)//외부인터럽트 0. 스위치 0을 누를 때
{
	cli();
	if(++mode_sel>6)
	{
		mode_sel=0;
	}
	sei();
}

ISR(INT1_vect)//외부 스위치1 누를 때
{
	cli();
	if(mode_sel==1)//mode1 일 때
	{
		if(state==STOP) state=START;//start 및 일시정지
		else            state=STOP;
	}
	else if (mode_sel==3)//mode3 일 때
	{
		if(state==STOP) state=START;//시작
		else state=STOP;//멈춤
	}
	sei();
}

ISR(INT3_vect)//외부 스위치3 누를 때
{
	cli();//인터럽트 비허용
	if(mode_sel==1)//mode 1
	{
		state=INIT;//초기화
	}
	else if (mode_sel==3)//mode 3
	{
		state=INIT;//초기화
	}
	sei();//전역 인터럽트 허용
}

ISR(INT4_vect)//mode3에서 외부 스위치4 누를 떼
{
	cli();
	SHOW_NUMBER12++;//초 단위 숫자를 증가
	if(SHOW_NUMBER12>99)
	SHOW_NUMBER12=99;//최대 99초에서부터 타이머 시작
	sei();
}

ISR(INT5_vect)//mode3에서 외부 스위치5 누를 때
{
	cli();
	SHOW_NUMBER12--;//초 단위 숫자 감소
	if(SHOW_NUMBER12<0)
	SHOW_NUMBER12=0;//0초까지 표현할 수 있다
	sei();
}

int main(void)
{
	DDRA = 0xff;//fnd 숫자 출력 레지스터
	DDRB = 0x10;//스피커 출력 레지스터
	DDRC = 0xff;//fnd의 위치 출력 레지스터
	DDRG = 0x03;//led 출력 레지스터
	DDRD = 0x00;//외부인터럽트 포트 입력사용 레지스터
	DDRE = 0x00;//외부인터럽트 연결 포트 입력으로 사용

	TCCR0 = 0x06;//digital clock에 사용되는 레지스터. 분주비를 256으로 설정
	TCNT0 = 0x00;//digital clock에 사용되는 레지스터. 카운터 값 저장
	TCCR2 = 0x04;//stopwatch에 사용되는 레지스터. 분주비를 8로 설정
	TCNT2 = 0x00;//stopwatch에 사용되는 레지스터. 카운터 값 저장
	TIMSK = 0x41;//타이머 0과 2의 출력비교 인터럽트 허용
	
	EICRA=0xFF;//외부 인터럽트 0~3 상승 edge에서 검출
	EICRB=0xFF;//외부 인터럽트 4~7 상승 edge에서 검출
	EIMSK=0xFF;//외부 인터럽트 0~7 모두 허용
			
	sei();//전역 인터럽트 허용

   
    while (1) 
    {
		switch(mode_sel)//모드에 따라 수행하는 내용
		{
			case 0:
				mode0_clock();
			break;
			case 1:
				mode1_stop_watch();
			break;
			case 2:
				mode2_piano();
			break;
			case 3:
				mode3_down_clock();
			break;
			case 4:
				mode4_piano2();
			break;
			case 5:
				mode5_uart();
			break;
			
			
			default:
				mode0_clock();//default 모드(초기 모드)는 mode0
			break;
		}
    }
}

void Show_clock_Display(unsigned int number)//mode0에서 사용
{
	COLUMN[0]   = (Minutes%100)/10;//분단위 값의 십의자리 숫자
	COLUMN[1]   = (Minutes%10);//분단위 값의 일의자리 숫자
	COLUMN[2]   = (Seconds%100)/10;//초단위  값의 십의자리 숫자
	COLUMN[3]   = (Seconds%10);//초단위 값의 일의자리 숫자
	
	for(int i=0;i<4;i++)
	{
		ShowDigit(COLUMN[i],i);//'_분_초'숫자를 fnd에 출력함
		_delay_ms(2); // wait for a second
	}
}

void Show4Digit(int number)//mode1에서 사용
{
	COLUMN[0] = number/1000;//fnd 첫번째자리 숫자
	COLUMN[1] = (number%1000)/100;//fnd 두번째자리 숫자
	COLUMN[2] = (number%100)/10;//fnd 세번째자리 숫자
	COLUMN[3] = (number%10);//fnd 네번째자리 숫자
	for(int i=0;i<4;i++)
	{
		ShowDigit(COLUMN[i],i);//fnd에 '초.밀리초'값을 표시
		_delay_ms(2); // wait for a second
	}
}

void ShowDigit(int i, int digit)
{
	PORTC=~(0x01<<digit);
	if(mode_sel==1||3)//stopwatch 모드 or down_clock 모드
	{
		if(digit==1)
		PORTA = Segment_Data[i]|0x80;//두번째 fnd의 dot을 켜기
		else
		PORTA = Segment_Data[i];//다른 자리의 fnd는 dot을 켜지 않음		
	}
	else PORTA = Segment_Data[i];
}

void mode0_clock()
{
	Show_clock_Display(Seconds);
	if(Seconds>=60)//60초가 지나면
	{
		Seconds=0;//초는 0으로 돌아가고
		Minutes++;//분을 +1한다
	}
	if(Minutes>=60)//60분이 지나면
	{
		Minutes=0;//분은 0으로 돌아가고
		Hours++;//시를 +1한다
	}
	if(Hours>=24)//24시가 지나면
	{
		Hours=0;//0시로 돌아간다
	}
}

void mode1_stop_watch()
{
	Run();
	SHOW_NUMBER=SHOW_NUMBER12*100+SHOW_NUMBER34;
	Show4Digit(SHOW_NUMBER);
}

void mode2_piano()
{
	piano_up();		
	_delay_ms(1000);
}

void mode3_down_clock()
{
	down_clock_Run();
	SHOW_NUMBER=SHOW_NUMBER12*100+SHOW_NUMBER34;
	Show4Digit(SHOW_NUMBER);
}

void mode4_piano2()
{
	switch((PING&0x0c)>>2)
	{
		case 0: break;
		case 1: 	schoolsong(); _delay_ms(1000); break;
		case 2:     SanToKisong(); 	_delay_ms(1000); break;
		case 3: break;
	}
}

void mode5_uart()
{
	char * str="\r\nB815148_LJH";
	char length = strlen(str);
	
	while(1){
		if(PING==0x04){
			uart0_init();
			Print(str, length);
			break;
		}
	}
}

void Run(void)
{
	switch(state)
	{
		case STOP :	break;//STOP상태면 아무 변화가 없다
		case START: SHOW_NUMBER34++;//START상태면 ms단위로 증가
		if(SHOW_NUMBER34>99)//99ms가 넘으면
		{
			SHOW_NUMBER12++;//초가 +1된다
			if(SHOW_NUMBER12>99) SHOW_NUMBER12=0;//99초가 넘으면 0초로 돌아간다
			SHOW_NUMBER34=0;//ms도 0초로 돌아간다
		}
		break;
		case INIT : SHOW_NUMBER12=0, SHOW_NUMBER34=0, state=STOP;//INIT 상태면 모두 0 
		break;
	}
}

//mode2 piano
void piano_up(void)
{
	for(int i=0;i<8;i++)
	{
		freq = Doremi[i];//도레미파솔라시도 연주
		TIMERvalue = 255-(1000000/(8*freq));
		Shimpyo(Doremi_length[i]);//4분음표로 연주
		Cutoff_Play();
	}	
}

//mode2,4 piano
void Shimpyo(int time)//각 음표의 재생 시간
{
	for(int i=0;i<time;i++)
	{
		_delay_ms(50);
	}
}

void Cutoff_Play(void)
{
	_delay_ms(300);
	TIMERvalue=255;
	_delay_ms(20);
}

//mode3 down_clock
void down_clock_Run(void)
{
	switch(state)
	{
		case STOP: //stop상태면 아무것도 안함
		break;
		case START: SHOW_NUMBER34--;//ms단위로 감소
		if(SHOW_NUMBER34<0)//0ms가 되면
		{
			SHOW_NUMBER34=99;//ms는 99로 돌아가고
			SHOW_NUMBER12--;//1초 감소
			if(SHOW_NUMBER12<0) //0초가 되면
			{
				SHOW_NUMBER12=0;//0으로 표시
				
			}
		}
		if(SHOW_NUMBER34==00 && SHOW_NUMBER12==00)//0.00초가 되면
		{
			state = STOP ;//멈춘다
		}
		break;
		case INIT : SHOW_NUMBER12=0, SHOW_NUMBER34=0, state=STOP;//초기화
		break;
	}
}

//mode4_piano2	
void schoolsong(void)
{
	for(int i=0;i<24;i++){
		freq = school[i];
		TIMERvalue = 255-(1000000/(8*freq));
		Shimpyo(school_length[i]);
		Cutoff_Play();
	}
}

void SanToKisong(void)
{
	for(int i=0;i<25;i++){
		freq = SanToKi[i];
		TIMERvalue = 255-(1000000/(8*freq));
		Shimpyo(SanToKi_length[i]);
		Cutoff_Play();
	}
}	

//mode5_uart
void uart0_init(void)
{
	UCSR0B = 0x00;
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	UBRR0L = 0x67;
	UBRR0H = 0x00;
	UCSR0B = 0x18;
}

int Putchar(char message)
{
	while (((UCSR0A>>UDRE0)&0x01) == 0) ;
	UDR0 = message;
	return 0;
}

int Print(char *str, char length)
{
	for(int i=0;i<length;i++)
	{
		Putchar(*str++);
	}
	return 0;
}

