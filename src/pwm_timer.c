/*
 @autor:       Alexandr Haidamaka
 @file:        pwm_timer.c
 @description: PWM_timer functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define PWM_BASE_RATE_US     (50)
#define TMR_PRESCALER_16     (4)
#define TMR_RELOAD_VALUE_80  (250 - 8)
#define TMR_RELOAD_VALUE_160 (250 - 4)
#define FRC1_ENABLE_TIMER    (1 << 7)
//#define ESP_PWM_DEBUG

/*******************************************************************************
 *** STRUCTURES
 ******************************************************************************/
struct pwm_info
{
	unsigned int pin :8;
	unsigned int val :1;
	uint32_t cnt;
	uint32_t period;
	uint32_t duty;
};

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
static int s_num_pwms;
static struct pwm_info *s_pwms;
static uint32_t s_pwm_timer_reload_value = TMR_RELOAD_VALUE_80;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void pwm_timer_callback(void *arg);

//------------------------------------------------------------------------------
static struct pwm_info *find_or_create_pwm_info(uint8_t pin, int create)
{
	int i;
	struct pwm_info *p = NULL;
	for (i = 0; i < s_num_pwms; i++)
	{
		if (s_pwms[i].pin == pin)
		{
			p = s_pwms + i;
			break;
		}
	}
	if (p != NULL || !create)
		return p;
	ETS_FRC1_INTR_DISABLE();
	p = (struct pwm_info *) realloc(s_pwms, sizeof(*p) * (s_num_pwms + 1));
	if (p != NULL)
	{
		s_pwms = p;
		p = s_pwms + s_num_pwms;
		s_num_pwms++;
		p->pin = pin;
		p->cnt = p->duty = 0;
	}
	ETS_FRC1_INTR_ENABLE();
	return p;
}

//------------------------------------------------------------------------------
static void remove_pwm_info(struct pwm_info *p)
{
	int i = p - s_pwms;
	ETS_FRC1_INTR_DISABLE();
	memmove(p, p + 1, (s_num_pwms - 1 - i) * sizeof(*p));
	s_num_pwms--;
	if (s_num_pwms > 0)
	{
		s_pwms = realloc(s_pwms, sizeof(*p) * s_num_pwms);
	}
	else
	{
		free(s_pwms);
		s_pwms = 0;
	}
	ETS_FRC1_INTR_ENABLE();
}

//------------------------------------------------------------------------------
inline static void pwm_configure_timer(void)
{
	if (system_get_cpu_freq() == SYS_CPU_80MHZ)
		s_pwm_timer_reload_value = TMR_RELOAD_VALUE_80;
	else
		s_pwm_timer_reload_value = TMR_RELOAD_VALUE_160;

	ETS_FRC_TIMER1_INTR_ATTACH(pwm_timer_callback, NULL);
	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, s_pwm_timer_reload_value);
	RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
	TMR_PRESCALER_16 | FRC1_ENABLE_TIMER);
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
}

//------------------------------------------------------------------------------
int pwm_set(int pin, int period, int duty)
{
	struct pwm_info *p;

	if (pin != 16 && get_gpio_info(pin) == NULL)
		return 0;

	if (period != 0
			&& (period < PWM_BASE_RATE_US * 2 || duty < 0 || duty > period))
		return 0;

	period /= PWM_BASE_RATE_US;
	duty /= PWM_BASE_RATE_US;

	p = find_or_create_pwm_info(pin, (period > 0 && duty >= 0));
	if (p == NULL)
		return 0;

	if (period == 0)
	{
		if (p != NULL)
		{
			remove_pwm_info(p);
			pwm_configure_timer();
			mgos_gpio_write(pin, 0);
		}
		return 1;
	}

	if (p->period == (uint32_t) period && p->duty == (uint32_t) duty)
		return 1;

	mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT);
#ifdef ESP_PWM_DEBUG
	mgos_gpio_set_mode(D7, MGOS_GPIO_MODE_OUTPUT);
#endif

	ETS_FRC1_INTR_DISABLE();
	p->period = period;
	p->duty = duty;
	if (p->cnt == 0 || p->cnt > (uint32_t) period)
	{
		p->val = 1;
		p->cnt = p->duty;
		mgos_gpio_write(pin, p->val);
	}
	ETS_FRC1_INTR_ENABLE();

	if (duty == 0 || period == duty)
		mgos_gpio_write(pin, (period == duty));

	pwm_configure_timer();
	return 1;
}

//------------------------------------------------------------------------------
IRAM NOINSTR void pwm_timer_callback(void *arg)
{
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, s_pwm_timer_reload_value);
	int i;
	uint32_t v, set = 0, clear = 0;
	(void) arg;
#ifdef ESP_PWM_DEBUG
	{
		v = GPIO_REG_READ(GPIO_OUT_ADDRESS);
		v ^= 1 << D7;
		GPIO_REG_WRITE(GPIO_OUT_ADDRESS, v);
	}
#endif
	for (i = 0; i < s_num_pwms; i++)
	{
		struct pwm_info *p = s_pwms + i;
		if (--(p->cnt) > 0)
			continue;
		/* Edge cases were handled during setup. */
		if (p->duty == 0 || p->duty == p->period)
			continue;
		if (p->pin != 16)
		{ /* GPIO 16 is controlled via a different register */
			uint32_t bit = ((uint32_t) 1) << p->pin;
			if (p->val)
			{
				clear |= bit;
			}
			else
			{
				set |= bit;
			}
		}
		else
		{
			uint32_t v = READ_PERI_REG(RTC_GPIO_OUT) & 0xfffffffe;
			v |= ~p->val;
			WRITE_PERI_REG(RTC_GPIO_OUT, v);
		}
		p->val ^= 1;
		p->cnt = (p->val ? p->duty : (p->period - p->duty));
	}
	if (set || clear)
	{
		v = GPIO_REG_READ(GPIO_OUT_ADDRESS);
		v |= set;
		v &= ~clear;
		GPIO_REG_WRITE(GPIO_OUT_ADDRESS, v);
	}
	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
}
