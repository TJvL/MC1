// Register to switch clocks on/off of all GPIO ports.
#define RCC_AHBENR (*(unsigned int *)(0x40021000 + 0x14))

// General Purpose Input/Output Port B
#define GPIOB_MODER (*(unsigned int *)(0x48000400 + 0x00)) // Register to switch the pin mode (input, output, etc.).
#define GPIOB_ODR (*(unsigned int *)(0x48000400 + 0x14)) // Register to write bits to output pins.
#define GPIOB_PUPDR (*(unsigned int *)(0x48000400 + 0x0C)) // Register to enable float, pull-up or pull-down on input pin.
#define GPIOB_IDR (*(unsigned int *)(0x48000400 + 0x10)) // Read-only register to read bits from input pins.

// Register to switch clocks on/off for timers.
#define RCC_APB1ENR		(*(unsigned int*)(0x40021000 + 0x1C))

// Timer 3

// Control register to configure the timer.
#define TIM3_CR1		(*(unsigned int*)(0x40000400 + 0x00))
// Event generation register to generate events on capture/compare channels.
#define TIM3_EGR		(*(unsigned int*)(0x40000400 + 0x14))
// Register that contains the counted value.
#define TIM3_CNT		(*(unsigned int*)(0x40000400 + 0x24))
// Precaler register to scale the timers count speed.
#define TIM3_PSC		(*(unsigned int*)(0x40000400 + 0x28))
// Status register for capture/compare and interrupt flags.
#define TIM3_SR			(*(unsigned int*)(0x40000400 + 0x10))
// Auto-reload register to set the value to be loaded in the auto-reload register.
#define TIM3_ARR		(*(unsigned int*)(0x40000400 + 0x2C))
// capture/compare register 1.
#define TIM3_CCR1		(*(unsigned int*)(0x40000400 + 0x34))

const short Tim3UsDivision = 7992; // Amount the counter needs to count to so 1ms has passed.
const short ledAmount = 4; // Amount of LEDs soldered on shield.

void configLbiShield() {
	RCC_AHBENR |= (1 << 18);  // Enable clock for GPIO port B

	GPIOB_MODER |= (1 << (3 * 2));   // Set GPIO port B pin 3 to output.
	GPIOB_MODER |= (1 << (4 * 2));   // Set GPIO port B pin 4 to output.
	GPIOB_MODER |= (1 << (5 * 2));   // Set GPIO port B pin 5 to output.
	GPIOB_MODER |= (1 << (6 * 2));   // Set GPIO port B pin 6 to output.

	RCC_APB1ENR ^= (1 << 1); // Enable TIM3 clock
	TIM3_CR1 = 0x1; // Enable counter (H15.4.1)
}

void timerDelayTest(int milliseconds){
	// This function uses timer 3 to generate a blocking
	// delay of <milliseconds>. Software constantly checks
	// the value in TIM3_CNT to determine if the time
	// (in milliseconds) has expired
	TIM3_EGR |= 1 << 0;
	TIM3_CR1 |= 1 << 0;

	int msPassed = 0;
	while (msPassed < milliseconds) {
		if (TIM3_CNT > Tim3UsDivision) {
			msPassed++;
			TIM3_EGR |= 1 << 0;
		}
	}
}

void timerDelay(int milliseconds) {
	// This function uses timer 3 to generate a blocking
	// delay of <milliseconds>. It uses one of the
	// capture/compare registers to check if the time
	// (in milliseconds) has expired.
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
	while (!(TIM3_SR >> 1 & 1))
		;
}

void ledWrite(int num, int on) {
	switch (num) { // Map the num parameter to a bit on the register.
	case 0: // When a non mapped number is passed the default is LED0.
	default:
		GPIOB_ODR ^= (on << 3);
		break;
	case 1:
		GPIOB_ODR ^= (on << 5);
		break;
	case 2:
		GPIOB_ODR ^= (on << 4);
		break;
	case 3:
		GPIOB_ODR ^= (on << 6);
		break;
	}
}

void ledSequence() {
	const short ms = 250;
	for (short i = 0; i < ledAmount; i++) {
		GPIOB_ODR = 0x0;
		ledWrite(i, 1);
		timerDelay(ms);
		//timerDelayTest(ms);
	}
}

int main() {
	configLbiShield();

	while (1) {
		ledSequence();
	}
}

