/*
 * This header provides constants for binding intel,x86-pinctrl.
 */

#ifndef _DT_BINDINGS_GPIO_X86_GPIO_H
#define _DT_BINDINGS_GPIO_X86_GPIO_H

#include <dt-bindings/gpio/gpio.h>

#define GPIO_MODE_NATIVE	0
#define GPIO_MODE_GPIO		1

#define GPIO_MODE_FUNC0	0
#define GPIO_MODE_FUNC1	1
#define GPIO_MODE_FUNC2	2
#define GPIO_MODE_FUNC3	3
#define GPIO_MODE_FUNC4	4
#define GPIO_MODE_FUNC5	5
#define GPIO_MODE_FUNC6	6

#define PIN_INPUT	0
#define PIN_OUTPUT	1

#define PIN_INPUT_NOPULL	0
#define PIN_INPUT_PULLUP	1
#define PIN_INPUT_PULLDOWN	2

#define PULL_STR_2K		0
#define PULL_STR_20K	2

#define ROUTE_SCI	0
#define ROUTE_SMI	1

#define OWNER_ACPI	0
#define OWNER_GPIO	1

#define PIRQ_APIC_MASK	0
#define PIRQ_APIC_ROUTE	1

#define TRIGGER_EDGE	0
#define TRIGGER_LEVEL	1

#endif
