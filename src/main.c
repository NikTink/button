
/*
 * Author: Saaga Hartikainen 2023
 * Adapted from Nordic Semiconductor ASA button-sample
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

/*
GLOBAL VARIABLES
*/

uint32_t switch_delay = 1000;			//delay before changing logical state
bool led_animation_mode = false;		//led animation mode (false = blink, true = spin)
uint32_t last_tick = 0;					//last tick the logical state was changed
uint8_t current_led = 1;				//current led selected (1-4, 0 = default)



/*
 * Get button configuration for sw0, 1, 2 and 3.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static struct gpio_callback button1_cb_data;

#define SW1_NODE	DT_ALIAS(sw1)
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,
							      {0});
static struct gpio_callback button2_cb_data;

#define SW2_NODE	DT_ALIAS(sw2)
#if !DT_NODE_HAS_STATUS(SW2_NODE, okay)
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios,
							      {0});
static struct gpio_callback button3_cb_data;

#define SW3_NODE	DT_ALIAS(sw3)
#if !DT_NODE_HAS_STATUS(SW3_NODE, okay)
#error "Unsupported board: sw3 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios,
							      {0});
static struct gpio_callback button4_cb_data;


/*
 * LED configuration for LED 1, 2, 3 and 4
 */
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});
							 
static struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
						     {0});
							 
static struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios,
						     {0});
							 
static struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios,
						     {0});
							 


/*
 BUTTON PRESS EVENTHANDLER
 */
void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
		 //test callback pinmask against button masks. We want to only look for single-button presses.
			if (cb -> pin_mask == button1_cb_data.pin_mask)	//change anim mode
			{
				led_animation_mode ^= 1; 
				printk("current mode: ");
				if (led_animation_mode)
				{
					printk("spin\n");
				} else {
					printk("blink\n");
				}
				return;
			}				
			
			if (cb -> pin_mask == button2_cb_data.pin_mask)	//increase switch delay
			{				
				if (switch_delay < 5000)
				{
					switch_delay = switch_delay + 100; 
					printk("current delay: %i\n", switch_delay);
				}
				return;
			}
			
			if (cb -> pin_mask == button3_cb_data.pin_mask)	//unused
			{				
				
				return;
			}
			
			if (cb -> pin_mask == button4_cb_data.pin_mask)	// decrease switch delay
			{				
				if (switch_delay > 100)
				{
					switch_delay = switch_delay - 100;
					printk("current delay: %i\n", switch_delay);
				}
				return;
			}

		
}

/*
 *Function: 
 *	up_led_state
 *Parameters: 
 *	None
 *summary:
 *	control LED state
 *description:
 *	Update LED states based on animation-state and delay and progress the logical state.
 *	Watch timer for when to operate. Called every cycle.
 */

void up_led_state( void )
{
	if ((k_uptime_get_32()-last_tick)>switch_delay)
	{
		if (led_animation_mode)
		{
			current_led++;
			switch(current_led)
				{
					case 1:
						gpio_pin_set_dt(&led1, 0);
						gpio_pin_set_dt(&led2, 1);
						break;
					
					case 2:
						gpio_pin_set_dt(&led2, 0);
						gpio_pin_set_dt(&led4, 1);
						break;
					
					case 3:
						gpio_pin_set_dt(&led4, 0);
						gpio_pin_set_dt(&led3, 1);
						break;
					
					case 4:
						gpio_pin_set_dt(&led3, 0);
						gpio_pin_set_dt(&led1, 1);
						current_led = 0;
						break;
					default:
						gpio_pin_set_dt(&led1, 0);
						gpio_pin_set_dt(&led2, 1);
						current_led = 1;
						break; 

				}
		} else {
			switch(current_led)
				{
					case 1:
						gpio_pin_set_dt(&led1, 0);
						gpio_pin_toggle_dt(&led2);
						break;
					
					case 2:
						gpio_pin_set_dt(&led2, 0);
						gpio_pin_toggle_dt(&led4);
						break;
					
					case 3:
						gpio_pin_set_dt(&led4, 0);
						gpio_pin_toggle_dt(&led3);
						break;
					
					case 4:
						gpio_pin_set_dt(&led3, 0);
						gpio_pin_toggle_dt(&led1);
						break;

					default:
						gpio_pin_set_dt(&led1, 0);
						gpio_pin_toggle_dt(&led2);
						break;
				}
			
			
		}
		last_tick = k_uptime_get_32();
	}
}

void main(void)
{
	
	int ret = 0;

	/*
		CONFIGURE BUTTONS
	*/

	if (!device_is_ready(button1.port)) {
		printk("Error: button1 device %s is not ready\n",
		       button1.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button1.port->name, button1.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button1,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button1.port->name, button1.pin);
		return;
	}

	gpio_init_callback(&button1_cb_data, button_pressed, BIT(button1.pin));
	gpio_add_callback(button1.port, &button1_cb_data);
	printk("Set up button1 at %s pin %d\n", button1.port->name, button1.pin);
	

	if (!device_is_ready(button2.port)) {
		printk("Error: button2 device %s is not ready\n",
		       button2.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button2.port->name, button2.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button2,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button2.port->name, button2.pin);
		return;
	}

	gpio_init_callback(&button2_cb_data, button_pressed, BIT(button2.pin));
	gpio_add_callback(button2.port, &button2_cb_data);
	printk("Set up button2 at %s pin %d\n", button2.port->name, button2.pin);
	

	if (!device_is_ready(button3.port)) {
		printk("Error: button3 device %s is not ready\n",
		       button3.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button3.port->name, button3.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button3,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button3.port->name, button3.pin);
		return;
	}

	gpio_init_callback(&button3_cb_data, button_pressed, BIT(button3.pin));
	gpio_add_callback(button3.port, &button3_cb_data);
	printk("Set up button3 at %s pin %d\n", button3.port->name, button3.pin);
	

	if (!device_is_ready(button4.port)) {
		printk("Error: button4 device %s is not ready\n",
		       button4.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button4.port->name, button4.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button4,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button4.port->name, button4.pin);
		return;
	}

	gpio_init_callback(&button4_cb_data, button_pressed, BIT(button4.pin));
	gpio_add_callback(button4.port, &button4_cb_data);
	printk("Set up button4 at %s pin %d\n", button4.port->name, button4.pin);

	/*
		CONFIGURE LEDS
	*/

	if (led1.port && !device_is_ready(led1.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led1.port->name);
		led1.port = NULL;
	}
	if (led1.port) {
		ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led1.port->name, led1.pin);
			led1.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led1.port->name, led1.pin);
		}
	}

	if (led2.port && !device_is_ready(led2.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led2.port->name);
		led2.port = NULL;
	}
	if (led2.port) {
		ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led2.port->name, led2.pin);
			led2.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led2.port->name, led2.pin);
		}
	}

	if (led3.port && !device_is_ready(led3.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led3.port->name);
		led3.port = NULL;
	}
	if (led3.port) {
		ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led3.port->name, led3.pin);
			led3.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led3.port->name, led3.pin);
		}
	}

	if (led4.port && !device_is_ready(led4.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led4.port->name);
		led4.port = NULL;
	}
	if (led4.port) {
		ret = gpio_pin_configure_dt(&led4, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led4.port->name, led4.pin);
			led4.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led4.port->name, led4.pin);
		}
	}

	/*
		ENTER OPERATION
	*/

	//start with all LEDs off
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);
	gpio_pin_set_dt(&led3, 0);
	gpio_pin_set_dt(&led4, 0);
	if (led1.port && led2.port && led3.port && led4.port) {
		while (1) {
			/* Operate state machine*/
			up_led_state();				//update led-state based on the current logical state
		}
	}
}
