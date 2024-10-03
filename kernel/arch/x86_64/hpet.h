#ifndef HPET_H
# define HPET_H
# include "kernel/types.h"

# define HPET_GENERAL_CAPABILITIES 0x0
# define HPET_GENERAL_CONFIGURATION 0x10
# define HPET_MAIN_COUNTER_VALUE 0xf0

# define HPET_CAP_COUNTER_CLOCK_OFFSET 32
# define HPET_CONFIGURATION_ON 1
# define HPET_CONFIGURATION_OFF 0

void	hpet_init(void);
void	hpet_sleep(u32 ms);

#endif