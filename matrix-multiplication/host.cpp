#include <libgen.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <e-hal.h>
#include <e-loader.h>

#define HELP_TEXT \
		"matrix-multiplication-host <row> <col> <rows> <cols>\n" \
		"\n" \
		"\texample:\n" \
		"\t\tmatrix-multiplication-host 0 0 4 4 1 my.srec\n" \
		"\n" \
		"\toptions:\n" \
		"\t\trow     target core start row coordinate\n" \
		"\t\tcol     target core start column coordinate\n" \
		"\t\trows    number of rows to test\n" \
		"\t\tcols    number of columns to test\n"

static void e_check_test(void* dev, unsigned row, unsigned col, int* status);

int main(int argc, char** args) {
	e_loader_diag_t e_verbose;
	e_platform_t platform;
	e_epiphany_t dev;
	int row0, col0, rows, cols;
	int status = 1; // pass
	int i, j;

	char* hostExecutable = strdup(args[0]);
	char* epiphanyExecutable = (char*) malloc(sizeof(char) * (strlen(hostExecutable) + strlen(E_EXECUTABLE) + 1 + 1));
	sprintf(epiphanyExecutable, "%s/%s", dirname(hostExecutable), E_EXECUTABLE);

	if (argc < 5) {
		printf(HELP_TEXT);
		free(hostExecutable);
		free(epiphanyExecutable);
		exit(0);
	} else {
		row0 = atoi(args[1]);
		col0 = atoi(args[2]);
		rows = atoi(args[3]);
		cols = atoi(args[4]);

		// initalize epiphany device
		e_init(nullptr);
		e_reset_system();
		e_get_platform_info(&platform);
		// e_set_loader_verbosity(L_D3);
		e_open(&dev, 0, 0, platform.rows, platform.cols); //open all cores

		e_load_group(epiphanyExecutable, &dev, row0, col0, (row0+rows), (col0+cols), E_TRUE);

		// checking the test
		for (i = row0; i < row0 + rows; i += 1) {
			for (j = col0; j < col0 + cols; j += 1) {
				e_check_test(&dev, i, j, &status);
			}
		}

		// close down Epiphany device
		e_close(&dev);
		e_finalize();
	}

	free(hostExecutable);
	free(epiphanyExecutable);

	// self check
	if (status) {
		return EXIT_SUCCESS;
	} else{
		return EXIT_FAILURE;
	}
}

void e_check_test(void* dev, unsigned row, unsigned col, int* status) {
	unsigned int result;
	int wait = 1;

	while(1) {
		e_read(dev, row, col, 0x24, &result, sizeof(unsigned));
		if (result == 0xdeadbeef) {
			printf("core (%d,%d) failed\n",row,col);
			*status = 0;
			break;
		} else if (result==0x12345678) {
			unsigned clr = (unsigned) 0x0;
			e_write(dev, row, col, 0x24, &clr, sizeof(clr));
			printf("core %d,%d passed\n", row, col);
			break;
		} else{
			if (wait){
				usleep(10000);
				printf("core %d,%d waiting...\n", row, col);
				wait = 0;
			} else {
				printf("core %d,%d failed\n", row, col);
				*status = 0;
				break;
			}
		}
	}
}
