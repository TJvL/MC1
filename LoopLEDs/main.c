/*
 * main.c
 *
 * Made by: Thomas van Leeuwen 2072837
 */

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

// Register to switch clocks on/off of all GPIO ports.
#define RCC_AHBENR (*(unsigned int *)(0x40021000 + 0x14))

// General Purpose Input/Output Port B
#define GPIOB_MODER (*(unsigned int *)(0x48000400 + 0x00)) // Register to switch the pin mode (input, output, etc.).
#define GPIOB_ODR (*(unsigned int *)(0x48000400 + 0x14)) // Register to write bits to output pins.
#define GPIOB_PUPDR (*(unsigned int *)(0x48000400 + 0x0C)) // Register to enable float, pull-up or pull-down on input pin.
#define GPIOB_IDR (*(unsigned int *)(0x48000400 + 0x10)) // Read-only register to read bits from input pins.
#define GPIOB_BSRR (*(unsigned int *)(0x48000400 + 0x18)) // Register to atomically write to GPIO pins.

const short ledAmount = 4; // Amount of LEDs soldered on shield.
short reverse = 0; // Reverse the LED sequence.

void shieldConfig() {
	BIT_SET(RCC_AHBENR, 18);
	//RCC_AHBENR |= (1 << 18);  // Enable clock for GPIO port B

	BIT_SET(GPIOB_MODER, 6);
	BIT_SET(GPIOB_MODER, 8);
	BIT_SET(GPIOB_MODER, 10);
	BIT_SET(GPIOB_MODER, 12);

//	GPIOB_MODER |= (1 << (3 * 2));   // Set GPIO port B pin 3 to output.
//	GPIOB_MODER |= (1 << (4 * 2));   // Set GPIO port B pin 4 to output.
//	GPIOB_MODER |= (1 << (5 * 2));   // Set GPIO port B pin 5 to output.
//	GPIOB_MODER |= (1 << (6 * 2));   // Set GPIO port B pin 6 to output.

	// GPIO port B pin 8 is input by default.
	// We do want to set input mode to pull up because the button is connected to ground.
	//GPIOB_PUPDR |= (1 << (8 * 2));
	BIT_SET(GPIOB_PUPDR, 16);
}

void write_bit(int num, int on) {
	if (on) {
		BIT_SET(GPIOB_BSRR, num);
	} else {
		BIT_CLEAR(GPIOB_BSRR, num + 16);
	}
}

void ledWrite(int num, int on) {
	switch (num) { // Map the num parameter to a bit on the register.
	case 0: // When a non mapped number is passed the default is LED0.
	default:
		write_bit(3, on);
		break;
	case 1:
		write_bit(5, on);
		break;
	case 2:
		write_bit(4, on);
		break;
	case 3:
		write_bit(6, on);
		break;
	}
}

void checkButtonState() {
	if (((GPIOB_IDR >> 8) & 1) == 0) {
		reverse ^= 1;
	}
}

void delay_us() {
	for (int i = 0; i < 50000; i++) {
		asm("nop");
	}
}

void ledSequence() {
	short ledNum = reverse ? ledAmount - 1 : 0;
	while (ledNum >= 0 && ledNum < ledAmount) {
		GPIOB_ODR = 0x0;
		ledWrite(ledNum, 1);
		delay_us();
		ledNum = reverse ? ledNum - 1 : ledNum + 1;
	}
}

int main() {
	shieldConfig();

	while (1) {
		checkButtonState();
		ledSequence();
	}
}

