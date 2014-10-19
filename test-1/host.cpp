#include "prereqs.hpp"

#include <libgen.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <e-hal.h>

#define HELP_TEXT \
		"test-1-host <y> <x>\n" \
		"\n" \
		"\toptions:\n" \
		"\t\ty    target core y coordinate\n" \
		"\t\tx    target core x coordinate\n"

int main(int argc, char** args) {
	e_platform_t platform;
	e_epiphany_t dev;
	u32 y, x;

	char* hostExecutable = strdup(args[0]);
	char* epiphanyExecutable = (char*) malloc(sizeof(char) * (strlen(hostExecutable) + strlen(E_EXECUTABLE) + 1 + 1));
	sprintf(epiphanyExecutable, "%s/%s", dirname(hostExecutable), E_EXECUTABLE);

	if (argc < 3) {
		printf(HELP_TEXT);
		free(hostExecutable);
		free(epiphanyExecutable);
		exit(0);
	}

	y = (u32) atoi(args[1]);
	x = (u32) atoi(args[2]);

	// initalize epiphany device
	e_init(nullptr);
	e_reset_system();
	e_get_platform_info(&platform);
	//e_set_loader_verbosity(L_D3);
	e_open(&dev, 0, 0, platform.rows, platform.cols); //open all cores

	UserInterrupt init = ReadyInterrupt;
	e_write((void*) &dev, y, x, 0x24, &init, sizeof(init));

	e_load_group(epiphanyExecutable, &dev, y, x, 1, 1, E_TRUE);

	bool quit = false;
	bool started = false;
	bool ready = false;

	u32 previousValue = 123456789;
	u32 value;

	while (quit == false) {
		UserInterrupt what;
		e_read((void*) &dev, y, x, 0x24, &what, sizeof(what));
		switch (what) {
		case ReadyInterrupt:
			if (ready == false) {
				printf("epiphany ready.\n");
				ready = true;
			}
			break;
		case StartedInterrupt:
			if (started == false) {
				u32 x_, y_;
				e_read((void*) &dev, y, x, 0x40, &x_, sizeof(x_));
				e_read((void*) &dev, y, x, 0x44, &y_, sizeof(y_));

				void *a, *b, *c, *d;

				e_read((void*) &dev, y, x, 0x4c, &a, sizeof(void*));
				e_read((void*) &dev, y, x, 0x50, &b, sizeof(void*));
				e_read((void*) &dev, y, x, 0x54, &c, sizeof(void*));
				e_read((void*) &dev, y, x, 0x58, &d, sizeof(void*));

				printf("%p, %p, %p, %p\n", a, b, c, d);

				if (x != x_) {
					printf("wrong x: %d != %d\n", x, x_);
					quit = true;
				}

				if (y != y_) {
					printf("wrong y: %d != %d\n", y, y_);
					quit = true;
				}

				printf("epiphany started.\n");
				started = true;
			}
			break;
		case FinishInterrupt:
			printf("epiphany wants to quit. quitting.\n");
			quit = true;
			break;
		default:
			printf("unknown interrupt value %x. aborting.\n", what);
			quit = true;
			break;
		}

		e_read((void*) &dev, y, x, 0x48, &value, sizeof(value));
		if (value != previousValue) {
			// printf("value %d\n", value);
			previousValue = value;
		}
	}

	u8 buf[0xbd0];
	e_read((void*) &dev, y, x, 0x1430, buf, 0xbd0);

	for (int i = 0; i < 0xbd0; i += 1) {
		printf("%x", buf[i]);
	}
	printf("\n");

	e_close(&dev);
	e_finalize();

	free(hostExecutable);
	free(epiphanyExecutable);

	exit(EXIT_SUCCESS);
}
