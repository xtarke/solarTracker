/*
 * main.c
 *
 * Pulses test for Eppley Solar Tracker
 *
 *  Created on: October 11, 2018
 *      Author: Renan Augusto Starke
 *      Instituto Federal de Santa Catarina
 */

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stdlib.h>

#include "lib/avr_usart.h"
#include "lib/avr_timer.h"
#include "lib/bits.h"
#include "pulso.h"

#include "display/lcd.h"

#include "buttons.h"

void f_idle_state(void);
void f_manual_state(void);
void f_solar_tracking_state(void);
void f_azmanual_state(void);
void f_zemanual_state(void);
void f_homing_state(void);
void f_offing_state(void);

void timer1_hardware_init();

void move_azimuth(int16_t pulses);
void move_zenith(int16_t pulses);

static inline void start_timer_1();
static inline void stop_timer_1();

volatile uint16_t tables_index = 0;

const uint16_t zenith_table[] PROGMEM = {468,478,487,495,504,513,522,530,539,548,557,566,574,583,592,601,609,
	618,627,636,645,653,662,671,680,
	688,697,706,715,724,732,741,750,759,767,776,785,794,802,811,820,829,837,846,855,863,872,881,890,898,
	907,916,924,933,942,950,959,968,976,985,994,1002,1011,1020,1028,1037,1046,1054,1063,1071,1080,1089,
	1097,1106,1114,1123,1131,1140,1148,1157,1165,1174,1183,1191,1199,1208,1216,1225,1233,1242,1250,1259,
	1267,1275,1284,1292,1301,1309,1317,1326,1334,1342,1351,1359,1367,1375,1384,1392,1400,1408,1417,1425,
	1433,1441,1449,1458,1466,1474,1482,1490,1498,1506,1514,1522,1530,1538,1546,1554,1562,1570,1578,1586,
	1594,1602,1610,1618,1625,1633,1641,1649,1657,1664,1672,1680,1688,1695,1703,1711,1718,1726,1733,1741,
	1749,1756,1764,1771,1779,1786,1794,1801,1808,1816,1823,1830,1838,1845,1852,1860,1867,1874,1881,1888,
	1896,1903,1910,1917,1924,1931,1938,1945,1952,1959,1966,1972,1979,1986,1993,2000,2006,2013,2020,2026,
	2033,2040,2046,2053,2059,2066,2072,2079,2085,2091,2098,2104,2110,2117,2123,2129,2135,2141,2147,2153,
	2159,2165,2171,2177,2183,2189,2195,2201,2206,2212,2218,2223,2229,2235,2240,2246,2251,2257,2262,2267,
	2273,2278,2283,2288,2294,2299,2304,2309,2314,2319,2324,2329,2333,2338,2343,2348,2352,2357,2362,2366,
	2371,2375,2379,2384,2388,2392,2397,2401,2405,2409,2413,2417,2421,2425,2429,2433,2437,2440,2444,2448,
	2451,2455,2458,2462,2465,2468,2472,2475,2478,2481,2484,2487,2490,2493,2496,2499,2502,2505,2507,2510,
	2512,2515,2517,2520,2522,2524,2527,2529,2531,2533,2535,2537,2539,2541,2543,2545,2546,2548,2550,2551,
	2553,2554,2555,2557,2558,2559,2560,2561,2562,2563,2564,2565,2566,2567,2567,2568,2569,2569,2570,2570,
	2570,2571,2571,2571,2571,2571,2571,2571,2571,2571,2571,2570,2570,2570,2569,2569,2568,2568,2567,2566,
	2565,2564,2564,2563,2562,2561,2559,2558,2557,2556,2554,2553,2551,2550,2548,2547,2545,2543,2541,2540,
	2538,2536,2534,2532,2529,2527,2525,2523,2520,2518,2515,2513,2510,2508,2505,2502,2500,2497,2494,2491,
	2488,2485,2482,2479,2476,2472,2469,2466,2462,2459,2455,2452,2448,2445,2441,2437,2434,2430,2426,2422,
	2418,2414,2410,2406,2402,2398,2393,2389,2385,2380,2376,2372,2367,2363,2358,2353,2349,2344,2339,2334,
	2330,2325,2320,2315,2310,2305,2300,2295,2290,2284,2279,2274,2269,2263,2258,2252,2247,2241,2236,2230,
	2225,2219,2213,2208,2202,2196,2190,2185,2179,2173,2167,2161,2155,2149,2143,2137,2130,2124,2118,2112,
	2106,2099,2093,2087,2080,2074,2067,2061,2054,2048,2041,2035,2028,2021,2015,2008,2001,1995,1988,1981,
	1974,1967,1960,1954,1947,1940,1933,1926,1919,1912,1905,1897,1890,1883,1876,1869,1862,1854,1847,1840,
	1832,1825,1818,1810,1803,1796,1788,1781,1773,1766,1758,1751,1743,1736,1728,1720,1713,1705,1698,1690,
	1682,1674,1667,1659,1651,1643,1636,1628,1620,1612,1604,1596,1588,1581,1573,1565,1557,1549,1541,1533,
	1525,1517,1509,1501,1493,1484,1476,1468,1460,1452,1444,1436,1428,1419,1411,1403,1395,1386,1378,1370,
	1362,1353,1345,1337,1328,1320,1312,1303,1295,1287,1278,1270,1262,1253,1245,1236,1228,1219,1211,1203,
	1194,1186,1177,1169,1160,1152,1143,1135,1126,1117,1109,1100,1092,1083,1075,1066,1058,1049,1040,1032,
	1023,1014,1006,997,989,980,971,963,954,945,937,928,919,911,902,893,885,876,867,858,850,841,832,824,
	815,806,797,789,780,771,762,754,745,736,727,719,710,701,692,684,675,666,657,649,640,631,622,614,605,
	596,587,579,570,561,552,544,535,526,517,509,500,491,482,474,465,456,447,439,430,421,412,404,395,386,
	378,369,360,351,343,334,325,317,308,299,291,282,274,265
};

const uint16_t azmiuth_table[] PROGMEM = {4788,4798,4808,4818,4828,4838,4848,4858,4868,4878,4888,4898,4908,4918,
		4929,4939,4949,4959,4969,4979,4989,4999,5009,5019,5029,5040,5050,5060,5070,5080,5090,5101,5111,5121,5131,
		5142,5152,5162,5172,5183,5193,5203,5213,5224,5234,5244,5255,5265,5276,5286,5296,5307,5317,5328,5338,5349,
		5359,5370,5380,5391,5401,5412,5423,5433,5444,5454,5465,5476,5486,5497,5508,5519,5529,5540,5551,5562,5573,
		5583,5594,5605,5616,5627,5638,5649,5660,5671,5682,5693,5704,5715,5726,5737,5749,5760,5771,5782,5793,5805,
		5816,5827,5839,5850,5861,5873,5884,5896,5907,5919,5930,5942,5954,5965,5977,5988,6000,6012,6024,6035,6047,
		6059,6071,6083,6095,6107,6119,6131,6143,6155,6167,6179,6191,6204,6216,6228,6240,6253,6265,6277,6290,6302,
		6315,6327,6340,6353,6365,6378,6391,6403,6416,6429,6442,6455,6468,6480,6493,6506,6520,6533,6546,6559,6572,
		6585,6599,6612,6625,6639,6652,6666,6679,6693,6707,6720,6734,6748,6761,6775,6789,6803,6817,6831,6845,6859,
		6873,6887,6902,6916,6930,6944,6959,6973,6988,7002,7017,7031,7046,7061,7075,7090,7105,7120,7135,7150,7165,
		7180,7195,7210,7225,7241,7256,7271,7287,7302,7318,7333,7349,7364,7380,7396,7412,7427,7443,7459,7475,7491,
		7507,7523,7540,7556,7572,7588,7605,7621,7638,7654,7671,7687,7704,7721,7738,7754,7771,7788,7805,7822,7839,
		7856,7873,7891,7908,7925,7942,7960,7977,7995,8012,8030,8047,8065,8083,8101,8118,8136,8154,8172,8190,8208,
		8226,8244,8262,8281,8299,8317,8335,8354,8372,8391,8409,8428,8446,8465,8484,8502,8521,8540,8558,8577,8596,
		8615,8634,8653,8672,8691,8710,8729,8748,8768,8787,8806,8825,8845,8864,8883,8903,8922,8941,8961,8980,9000,
		9019,9039,9059,9078,9098,9117,9137,9157,9176,9196,9216,9235,9255,9275,9295,9315,9334,9354,9374,9394,9414,
		9433,9453,9473,9493,9513,9533,9553,9572,9592,9612,9632,9652,9672,9692,9712,9731,9751,9771,9791,9811,9831,
		9850,9870,9890,9910,9930,9949,9969,9989,10009,10028,10048,10068,10087,10107,10127,10146,10166,10185,10205,
		10224,10244,10263,10283,10302,10321,10341,10360,10379,10399,10418,10437,10456,10475,10495,10514,10533,10552,
		10571,10590,10609,10627,10646,10665,10684,10703,10721,10740,10759,10777,10796,10814,10833,10851,10869,10888,
		10906,10924,10943,10961,10979,10997,11015,11033,11051,11069,11087,11104,11122,11140,11158,11175,11193,11210,
		11228,11245,11263,11280,11297,11315,11332,11349,11366,11383,11400,11417,11434,11451,11468,11484,11501,11518,
		11534,11551,11568,11584,11600,11617,11633,11649,11666,11682,11698,11714,11730,11746,11762,11778,11794,11810,
		11825,11841,11857,11872,11888,11903,11919,11934,11950,11965,11980,11995,12011,12026,12041,12056,12071,12086,
		12101,12115,12130,12145,12160,12174,12189,12204,12218,12233,12247,12261,12276,12290,12304,12318,12333,12347,
		12361,12375,12389,12403,12417,12431,12444,12458,12472,12486,12499,12513,12527,12540,12554,12567,12581,12594,
		12607,12621,12634,12647,12660,12673,12687,12700,12713,12726,12739,12752,12764,12777,12790,12803,12816,12828,
		12841,12854,12866,12879,12891,12904,12916,12929,12941,12954,12966,12978,12991,13003,13015,13027,13039,13052,
		13064,13076,13088,13100,13112,13124,13136,13147,13159,13171,13183,13195,13206,13218,13230,13242,13253,13265,
		13276,13288,13299,13311,13322,13334,13345,13357,13368,13379,13391,13402,13413,13425,13436,13447,13458,13470,
		13481,13492,13503,13514,13525,13536,13547,13558,13569,13580,13591,13602,13613,13624,13635,13645,13656,13667,
		13678,13689,13699,13710,13721,13732,13742,13753,13764,13774,13785,13795,13806,13817,13827,13838,13848,13859,
		13869,13880,13890,13901,13911,13921,13932,13942,13953,13963,13973,13984,13994,14004,14015,14025,14035,14046,
		14056,14066,14076,14087,14097,14107,14117,14127,14138,14148,14158,14168,14178,14188,14199,14209,14219,14229,
		14239,14249,14259,14269,14279,14290,14300,14310,14320,14330,14340,14350,14360,14370,14380,14390,14400,14410,
		14420,14430,14440,14450,14460,14470,14480,14490,14500,14510,14520,14530,14540,14549,14559,14569,14579,14589,
		14599,14609,14619,14629,14639,14649,14659
};


/* Definição dos estados */
typedef enum {
	IDLE,
	MANUAL,
	ZE_MANUAL,
	AZ_MANUAL,
	SOLAR_TRACKING,
	HOMING,
	OFFING,
	NUM_STATES
} state_t;

/* Definição da estrutura mantenedora do vetor de estados */
typedef struct {
	state_t myState;
	void (*func)(void);
}fsm_t;

char lcd_debug [][10] = {
		"IDLE   ",
		"MANUAL ",
		"ZE_M   ",
		"AZ_M   ",
		"TRACK  ",
		"HOMING ",
		"OFFING ",
};


/* Mapeamento entre estado e funções */
fsm_t myFSM[] = {
	{ IDLE, f_idle_state },
	{ MANUAL, f_manual_state },
	{ ZE_MANUAL, f_zemanual_state },
	{ AZ_MANUAL, f_azmanual_state },
	{ SOLAR_TRACKING, f_solar_tracking_state },
	{ HOMING, f_homing_state },
	{ OFFING, f_offing_state },

};

/* Estado atual */
volatile state_t curr_state = IDLE;

FILE *debug;
FILE *lcd_stream;

int main(void)
{
	/* Set IO DIR pins */
	EPPLEY_CTRL_PORT->DDR |= SET(AZ_PULSE_PIN) | SET(AZ_DIR_PIN) | SET(ZE_PULSE_PIN) | SET(ZE_DIR_PIN) | SET(PC5);

	lcd_stream = inic_stream();

	inic_LCD_4bits();

	/* Obtem o stream de depuração */
	debug = get_usart_stream();

	/* Inicializa hardware da USART */
	USART_Init(B9600);

	timer1_hardware_init();

	buttons_init();

	sei();

	//0.01875º per step

	_delay_ms(1000);

	SET_BIT(PORTB,PB2);

	while(1)
    {
		cmd_LCD(0x80, 0);
		(*myFSM[curr_state].func)();

		char * disp = lcd_debug[(uint8_t)curr_state];

		escreve_LCD(disp);
		
		_delay_ms(100);
    }
}


void f_idle_state(void){

	static uint8_t old_button = NONE;
	uint8_t button = get_button(old_button);

	stop_timer_1();

	if (button == SELECT)
		curr_state = MANUAL;


	old_button = button;
}

void f_manual_state(void){

	static uint8_t old_button = NONE;

	uint8_t button = get_button(old_button);

	if (button == RIGHT)
		curr_state = SOLAR_TRACKING;

	if (button == UP)
		curr_state = ZE_MANUAL;

	if (button == DOWN)
		curr_state = AZ_MANUAL;


	old_button = button;
}


void f_azmanual_state(void){

	static uint8_t old_button = NONE;
	uint8_t button = get_button(old_button);

	if (button == RIGHT)
		curr_state = IDLE;

	if (button == DOWN){
		SET_BIT(EPPLEY_CTRL_PORT->PORT, AZ_DIR_PIN);
		gera_pulsos(250, AZ_PULSE_PIN);
	}

	if (button == UP){
		CLR_BIT(EPPLEY_CTRL_PORT->PORT, AZ_DIR_PIN);
		gera_pulsos(250, AZ_PULSE_PIN);
	}


	old_button = button;
}

void f_zemanual_state(void){

	static uint8_t old_button = NONE;
	uint8_t button = get_button(old_button);

	if (button == RIGHT)
		curr_state = IDLE;

	if (button == DOWN){
		SET_BIT(EPPLEY_CTRL_PORT->PORT, ZE_DIR_PIN);
		gera_pulsos(250, ZE_PULSE_PIN);
	}

	if (button == UP){
		CLR_BIT(EPPLEY_CTRL_PORT->PORT, ZE_DIR_PIN);
		gera_pulsos(250, ZE_PULSE_PIN);
	}


	old_button = button;
}

volatile uint16_t current_az_pos = 0;
volatile uint16_t current_ze_pos = 0;

void f_solar_tracking_state(void){

	static uint8_t old_button = NONE;
	uint8_t button = get_button(old_button);

	if (button == SELECT)
		curr_state = OFFING;

	if (!TST_BIT(TIMER_1->TCCRB, CS10)){
		start_timer_1();
	}

	uint16_t new_az_pos = pgm_read_word(&azmiuth_table[tables_index]);
	uint16_t new_ze_pos = pgm_read_word(&zenith_table[tables_index]);

	int16_t delta =  new_az_pos - current_az_pos;


	if (delta != 0) {
		move_azimuth(delta);
		while(is_pulsing());
		current_az_pos = new_az_pos;
	}


	delta =  new_ze_pos - current_ze_pos;
	if (delta != 0) {
		move_zenith(delta);
		while(is_pulsing());
		current_ze_pos = new_ze_pos;
	}

	fprintf(debug,"%d %d %d %d\n",tables_index, delta, current_az_pos, current_ze_pos);

	old_button = button;
}

void f_homing_state(void){

	while(is_pulsing());

	int16_t delta =  0 - current_az_pos;
	move_azimuth(delta);
	while(is_pulsing());
	current_az_pos = 0;

	delta = 0 - current_ze_pos;
	move_zenith(delta);
	while(is_pulsing());
	current_ze_pos = 0;

	curr_state = SOLAR_TRACKING;
	tables_index = 0;
}

void f_offing_state(void){

	stop_timer_1();
	while(is_pulsing());

	int16_t delta =  0 - current_az_pos;
	move_azimuth(delta);
	while(is_pulsing());
	current_az_pos = 0;

	delta = 0 - current_ze_pos;
	move_zenith(delta);
	while(is_pulsing());
	current_ze_pos = 0;

	curr_state = IDLE;
	tables_index = 0;

}

/**
  * @brief  Configura hardware do timer1 para IRQ em CTC.
  * @param	Nenhum
  *
  * @retval Nenhum.
  */
void timer1_hardware_init(){

	/* Acesso indireto por struct e bit field: com avr_timer.h */
	TIMER_1->TCCRA = 0;
	/* Modo CTC e prescaler 1024 */
	TIMER_1->TCCRB = SET(WGM12);

	/* Definição do TOP */
	TIMER_1->OCRA = 20535;

	/* Habilitação da IRQ: capture pois o top é OCR1A */
	TIMER_IRQS->TC1.BITS.OCIEA = 1;
}

static inline void start_timer_1(){
	TIMER_1->TCCRB |=  SET(CS10) | SET(CS12);
}

static inline void stop_timer_1(){
	TIMER_1->TCCRB &=  ~(SET(CS10) | SET(CS12));
	TIMER_1->TCNT = 0;
}

ISR(TIMER1_COMPA_vect){

	/* Don't move if it is already moving */
	volatile uint8_t pulsing = is_pulsing();
	if (pulsing)
		return;

	tables_index++;

	if (tables_index == 699){
		stop_timer_1();
		curr_state = HOMING;
	}

}

void move_azimuth(int16_t pulses){

	if (pulses == 0)
		return;
	else if (pulses < 0)
		SET_BIT(EPPLEY_CTRL_PORT->PORT, AZ_DIR_PIN);
	else
		CLR_BIT(EPPLEY_CTRL_PORT->PORT, AZ_DIR_PIN);

	uint16_t abs_pulses = abs(pulses);

	gera_pulsos(abs_pulses, AZ_PULSE_PIN);
}

void move_zenith(int16_t pulses){

	if (pulses == 0)
		return;
	else if (pulses < 0)
		SET_BIT(EPPLEY_CTRL_PORT->PORT, ZE_DIR_PIN);
	else
		CLR_BIT(EPPLEY_CTRL_PORT->PORT, ZE_DIR_PIN);

	uint16_t abs_pulses = abs(pulses);

	gera_pulsos(abs_pulses, ZE_PULSE_PIN);
}




