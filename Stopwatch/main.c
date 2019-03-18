#ifndef MAIN_C
#define MAIN_C

// Register to switch clocks on/off for all GPIO ports.
#define RCC_AHBENR		(*(unsigned int *)(0x40021000 + 0x14))

// General Purpose Input/Output Port A.

// Register to switch the pin mode (input, output, etc.).
#define GPIOA_MODER		(*(unsigned int *)(0x48000000 + 0x00))
// Register to write bits to output pins.
#define GPIOA_ODR		(*(unsigned int *)(0x48000000 + 0x14))
// Register to enable float, pull-up or pull-down on input pin.
#define GPIOA_PUPDR		(*(unsigned int *)(0x48000000 + 0x0C))
// Read-only register to read bits from input pins.
#define GPIOA_IDR		(*(unsigned int *)(0x48000000 + 0x10))
// Register to configure the clock speeds of pins.
#define GPIOA_OSPEEDR	(*(unsigned int *)(0x48000000 + 0x08))
// Register to configure output type.
#define GPIOA_OTYPER	(*(unsigned int *)(0x48000000 + 0x04))
// Register to atomically write to GPIO pins.
#define GPIOA_BSRR		(*(unsigned int *)(0x48000000 + 0x18))

// General Purpose Input/Output Port B

// Register to switch the pin mode (input, output, etc.).
#define GPIOB_MODER		(*(unsigned int *)(0x48000400 + 0x00))
// Register to write bits to output pins.
#define GPIOB_ODR		(*(unsigned int *)(0x48000400 + 0x14))
// Register to enable float, pull-up or pull-down on input pin.
#define GPIOB_PUPDR		(*(unsigned int *)(0x48000400 + 0x0C))
// Read-only register to read bits from input pins.
#define GPIOB_IDR		(*(unsigned int *)(0x48000400 + 0x10))
// Register to atomically write to GPIO pins.
#define GPIOB_BSRR		(*(unsigned int *)(0x48000400 + 0x18))

// Register to switch clocks on/off for timers.
#define RCC_APB1ENR		(*(unsigned int*)(0x40021000 + 0x1C))

// Timer 3.

// Control register to configure the timer.
#define TIM3_CR1		(*(unsigned int*)(0x40000400 + 0x00))
// Status register for capture/compare and interrupt flags.
#define TIM3_SR			(*(unsigned int*)(0x40000400 + 0x10))
// Event generation register to generate events on capture/compare channels.
#define TIM3_EGR		(*(unsigned int*)(0x40000400 + 0x14))
// Register that contains the counted value.
#define TIM3_CNT		(*(unsigned int*)(0x40000400 + 0x24))
// Precaler register to scale the timers count speed.
#define TIM3_PSC		(*(unsigned int*)(0x40000400 + 0x28))
// Auto-reload register to set the value to be loaded in the auto-reload register.
#define TIM3_ARR		(*(unsigned int*)(0x40000400 + 0x2C))
// capture/compare register 1.
#define TIM3_CCR1		(*(unsigned int*)(0x40000400 + 0x34))

// Timer 6.

// Control register to configure the timer.
#define TIM6_CR1		(*(unsigned int*)(0x40001000 + 0x00))
// Event generation register to generate events on capture/compare channels.
#define TIM6_EGR		(*(unsigned int*)(0x40001000 + 0x14))
// Register that contains the counted value.
#define TIM6_CNT		(*(unsigned int*)(0x40001000 + 0x24))
// Precaler register to scale the timers count speed.
#define TIM6_PSC		(*(unsigned int*)(0x40001000 + 0x28))

// Real-time clock.

// PWR power control register.
#define PWR_CR      	(*(unsigned int *) (0x40007000 + 0x00))
// RTC domain control register.
#define RCC_BDCR    	(*(unsigned int *) (0x40021000 + 0x20))
// RTC write protection register.
#define RTC_WPR     	(*(unsigned int *) (0x40002800 + 0x24))
// RTC initialization and status register.
#define RTC_ISR	    	(*(unsigned int *) (0x40002800 + 0x0C))
// RTC date register.
#define RTC_DR	    	(*(unsigned int *) (0x40002800 + 0x04))
// RTC time register.
#define RTC_TR	    	(*(unsigned int *) (0x40002800 + 0x00))
// RTC prescaler register.
#define RTC_PRER		(*(unsigned int *) (0x40002800 + 0x10))

// How long must button be hold down for RTX reset?
const unsigned short buttonHoldTime = 1000; // milliseconds.
// Default delay in microseconds.
const unsigned short delayUs = 5; // microseconds.
// Display middle colon on/off.
const unsigned short colon = 1;
// Is the real time clock running?
unsigned short rtcRunning = 0;

// Button debouncing.
volatile char buttonPressed = 0;
volatile char buttonPressedConfidenceLevel = 0;
volatile char buttonReleasedConfidenceLevel = 0;
const int confidenceTreshold = 20;

// Segment data with default state (00:00).
unsigned char display0 = 0x3F;
unsigned char display1 = 0xBF;
unsigned char display2 = 0x3F;
unsigned char display3 = 0x3F;

// Segment digit map.
const unsigned char segmentMap[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d,
		0x07, 0x7f, 0x6f };

void calculate_segment_data() {
	display0 = segmentMap[(RTC_TR & 0xF000) >> 12];
	display1 = segmentMap[(RTC_TR & 0xF00) >> 8];
	display2 = segmentMap[(RTC_TR & 0xF0) >> 4];
	display3 = segmentMap[(RTC_TR & 0xF) >> 0];

	if (colon)
		display1 |= 1 << 7;
}

void start_real_time_clock() {
	PWR_CR |= (1 << 8);
	RCC_BDCR |= (1 << 0);
	PWR_CR &= ~(1 << 8);
	rtcRunning = 1;
}

void pause_real_time_clock() {
	PWR_CR |= (1 << 8);
	RCC_BDCR &= ~(1 << 0);
	PWR_CR &= ~(1 << 8);
	rtcRunning = 0;
}

void reset_real_time_clock() {
	RCC_APB1ENR |= (1 << 28);
	PWR_CR |= (1 << 8);
	RCC_BDCR |= (1 << 8);
	RCC_BDCR |= (1 << 0);
	RCC_BDCR |= (1 << 15);

	RTC_WPR = 0xCA;
	RTC_WPR = 0x53;
	RTC_ISR |= (1 << 7);
	while ((RTC_ISR & (1 << 6)) == 0)
		;
	RTC_DR = 0x0;
	RTC_TR = 0x0;
	RTC_ISR &= ~(1 << 7);
	RTC_WPR = 0xFE;
	RTC_WPR = 0x64;
	RCC_BDCR &= ~(1 << 0);
	PWR_CR &= ~(1 << 8);

	rtcRunning = 0;
}

void configure() {
	// GPIO port A clock on.
	RCC_AHBENR |= 1 << 17;

	// Set pin A5 to output.
	GPIOA_MODER |= 1 << 10;
	// Set pin A7 to output.
	GPIOA_MODER |= 1 << (7 * 2);

	// Set pin A5 to pull-up.
	GPIOA_PUPDR |= 1 << (5 * 2);
	// Set pin A7 to pull-up.
	GPIOA_PUPDR |= 1 << (7 * 2);

	// GPIO port B clock on.
	RCC_AHBENR |= 1 << 18;

	// Button input pull up
	GPIOB_PUPDR |= 1 << (2 * 8);

	// Set pin B3 to output.
	GPIOB_MODER |= 1 << (3 * 2);
	// Set pin B4 to output.
	GPIOB_MODER |= 1 << (4 * 2);
	// Set pin B5 to output.
	GPIOB_MODER |= 1 << (5 * 2);
	// Set pin B6 to output.
	GPIOB_MODER |= 1 << (6 * 2);

	// Enable TIM3 clock.
	RCC_APB1ENR |= 1 << 1;

	// Enable TIM6 clock.
	RCC_APB1ENR |= 1 << 4;
}

void check_button() {
	// Released state is 1 by pull-up.
	// Check pin 8B input register.
	if ((GPIOB_IDR & 0x100) == 0) {
		if (buttonPressed == 0) {
			if (buttonPressedConfidenceLevel > confidenceTreshold) {
				buttonPressed = 1;

				// Re-initialize timer.
				TIM6_EGR |= 1 << 0;
				// Set prescaler to 122 so we count in seconds.
				TIM6_PSC = 7999/*+1*/;
				// Enable timer.
				TIM6_CR1 |= 1 << 0;
			} else {
				buttonPressedConfidenceLevel++;
				buttonReleasedConfidenceLevel = 0;
			}
		}
	} else {
		if (buttonPressed == 1) {
			if (buttonReleasedConfidenceLevel > confidenceTreshold) {
				buttonPressed = 0;

				// Disable timer.
				TIM6_CR1 &= ~(1 << 0);

				if (TIM6_CNT < buttonHoldTime) {
					if (!rtcRunning) {
						start_real_time_clock();
					} else {
						pause_real_time_clock();
					}
				} else {
					reset_real_time_clock();
				}
			} else {
				buttonReleasedConfidenceLevel++;
				buttonPressedConfidenceLevel = 0;
			}
		}
	}
}

void delay_milliseconds(unsigned short milliseconds) {
	// Re-initialize timer.
	TIM3_EGR |= 1 << 0;
	// Set prescaler to 122 so we count in seconds.
	TIM3_PSC = 7999/*+1*/;
	// Enable timer.
	TIM3_CR1 |= 1 << 0;
	// Set amount timer should count to.
	TIM3_ARR = milliseconds - 1;
	TIM3_CCR1 = milliseconds;
	// Clear update event bit.
	TIM3_SR &= ~(1 << 1);

	// Wait till bit is set again by timer hardware.
	while (!(TIM3_SR >> 1 & 1)) {
		check_button();
	}
}

void delay_microseconds(unsigned short microseconds) {
	// Re-initialize timer.
	TIM3_EGR |= 1 << 0;
	// Set prescaler to 8 so we count in microseconds.
	TIM3_PSC = 7/*+1*/;
	// Enable timer.
	TIM3_CR1 |= 1 << 0;
	// Set amount timer should count to.
	TIM3_ARR = microseconds - 1;
	TIM3_CCR1 = microseconds;
	// Clear update event bit.
	TIM3_SR &= ~(1 << 1);

	// Wait till bit is set again by timer hardware.
	while (!(TIM3_SR >> 1 & 1)) {
		check_button();
	}
}

void start_communication_with_display() {
	GPIOA_ODR |= (1 << 5);		// Clock high
	GPIOA_ODR |= (1 << 7);		// Dio high
	delay_microseconds(delayUs);
	GPIOA_ODR &= ~(1 << 7);		// Dio Low
	delay_microseconds(delayUs);
	GPIOA_ODR &= ~(1 << 5);		// Clock Low
	delay_microseconds(delayUs);
}

void stop_communication_with_display() {
	GPIOA_ODR &= ~(1 << 5);	// Clock Low
	GPIOA_ODR &= ~(1 << 7);		// Dio Low
	delay_microseconds(delayUs);
	GPIOA_ODR |= (1 << 5);		// Clock high
	delay_microseconds(delayUs);
	GPIOA_ODR |= (1 << 7);		// Dio high
	delay_microseconds(delayUs);
}

void acknowledge_display_received_command() {
	GPIOA_ODR &= ~(1 << 5);		// Clock Low
	GPIOA_ODR &= ~(1 << 7);		// Dio Low
	delay_microseconds(delayUs);
	GPIOA_ODR |= (1 << 5);		// Clock high
	delay_microseconds(delayUs);
	GPIOA_ODR &= ~(1 << 5);		// Clock Low
	delay_microseconds(delayUs);
}

void write_byte_to_display(unsigned char byte) {
	for (int i = 0; i < 8; ++i) {
		GPIOA_ODR &= ~(1 << 5);		// Clock Low
		if (byte & 0x01) {
			GPIOA_ODR |= (1 << 7);	// Dio high
		} else {
			GPIOA_ODR &= ~(1 << 7);	// Dio Low
		}
		delay_microseconds(delayUs);
		byte >>= 1;
		GPIOA_ODR |= (1 << 5);		// Clock high
		delay_microseconds(delayUs);
		GPIOA_ODR &= ~(1 << 5);		// Clock Low
	}
}

void configure_display() {
	start_communication_with_display();

	/*	Data command.
	 * 	01000FK0
	 * 	F = Must address of segment display NOT increment after each segment data command?
	 * 	K = Must data be written to display (0) or from keyboard (1)?
	 * */
	write_byte_to_display(0x40);
	acknowledge_display_received_command();

	stop_communication_with_display();
	start_communication_with_display();

	/*	Display control command.
	 * 	1000SBBB
	 * 	S = Must display be turned on?
	 * 	B = Brightness range: 0..7.
	 * */
	write_byte_to_display(0x8A);
	acknowledge_display_received_command();

	stop_communication_with_display();
}

void update_display(unsigned char display0, unsigned char display1,
		unsigned char display2, unsigned char display3) {
	start_communication_with_display();

	/*	Address command.
	 * 	11000AAA
	 * 	A = Address range: 0..5.
	 * */
	write_byte_to_display(0xC0);
	acknowledge_display_received_command();

	/*	Segment data command.
	 * 	SGFEDCBA
	 * 	S = Turn colon of selected display on/off.
	 * 	A..G = Turn correspondig segment of selected display on/off.
	 * */
	write_byte_to_display(display0);
	acknowledge_display_received_command();

	write_byte_to_display(display1);
	acknowledge_display_received_command();

	write_byte_to_display(display2);
	acknowledge_display_received_command();

	write_byte_to_display(display3);
	acknowledge_display_received_command();

	stop_communication_with_display();
}

int main() {
	configure();
	reset_real_time_clock();
	configure_display();

	while (1) {
		calculate_segment_data();
		update_display(display0, display1, display2, display3);
	}
}

#endif // MAIN_C

