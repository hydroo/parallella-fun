#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <e-hal.h>
#include <e-loader.h>

static void e_check_test(void* dev, unsigned row, unsigned col, int* status);
static void usage();

int main(int argc, char** args) {
	e_loader_diag_t e_verbose;
	e_platform_t platform;
	e_epiphany_t dev;
	int row0, col0, rows, cols, para;
	char elfFile[4096];
	int status = 1; // pass
	int i, j;

	if (argc < 5) {
		usage();
		status = 0;
	} else {
		row0 = atoi(args[1]);
		col0 = atoi(args[2]);
		rows = atoi(args[3]);
		cols = atoi(args[4]);
		para = atoi(args[5]);
		strcpy(elfFile, args[6]);

		// initalize epiphany device
		e_init(nullptr);
		e_reset_system();
		e_get_platform_info(&platform);
		// e_set_loader_verbosity(L_D3);
		e_open(&dev, 0, 0, platform.rows, platform.cols); //open all cores

		// load program one at a time, checking one a time
		if(para) {
			printf("running in parallel\n");
			for (i = row0; i < row0 + rows; i += 1) {
				for (j = col0; j < col0 + cols; j += 1) {
					e_load_group(elfFile, &dev, i, j, 1, 1, E_TRUE);
				}
			}
		} else {
			e_load_group(elfFile, &dev, row0, col0, (row0+rows), (col0+cols), E_TRUE);
		}

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

void usage() {
	printf(
			"matrix-multiplication-host <row> <col> <rows> <cols> <srec>\n"
			"\n"
			"\texample:\n"
			"\t\te-test 0 0 4 4 1 my.srec\n"
			"\n"
			"\toptions:\n"
			"\t\trow     target core start row coordinate\n"
			"\t\tcol     target core start column coordinate\n"
			"\t\trows    number of rows to test\n"
			"\t\tcols    number of columns to test\n"
			"\t\tpara    run test in parallel\n"
			"\t\tsrec    path to srec file\n"
			);
}
