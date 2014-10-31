#include "prereqs.hpp"

#include <libgen.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <unistd.h>

#include <e-hal.h>

#define HELP_TEXT \
		"matrix-multiplication-1-core-host <x> <y>\n" \
		"\n" \
		"\toptions:\n" \
		"\t\ty    target core y coordinate\n" \
		"\t\tx    target core x coordinate\n"

static f32* allocateMatrix(int width, int height);
static void fillMatrix(f32* m, int width, int height, f32 value);
static void fillMatrixRandomly(f32* m, int width, int height);
static void copyMatrix(f32* from, f32* to, int width, int height);
static void printMatrix(f32* m, int width, int height, FILE* out, const char* indent);
static void freeMatrix(f32* m);

int main(int argc, char** args) {
	char* hostExecutable = strdup(args[0]);
	char* epiphanyExecutable = (char*) malloc(sizeof(char) * (strlen(hostExecutable) + strlen(E_EXECUTABLE) + 1 + 1));
	sprintf(epiphanyExecutable, "%s/%s", dirname(hostExecutable), E_EXECUTABLE);

	if (argc < 3) {
		printf(HELP_TEXT);
		free(hostExecutable);
		free(epiphanyExecutable);
		exit(0);
	}

	int x = atoi(args[2]);
	int y = atoi(args[1]);

	// initalize epiphany device
	e_platform_t platform;
	e_epiphany_t dev;

	e_init(nullptr);
	e_reset_system();
	e_get_platform_info(&platform);
	// e_set_loader_verbosity(L_D3);
	e_open(&dev, y, x, 1, 1);

	e_load_group(epiphanyExecutable, &dev, 0, 0, 1, 1, E_FALSE);

	UserInterrupt init = UserInterrupt::NotDone;
	e_write(&dev, 0, 0, 0x24, &init, sizeof(init));

	int matrixWidth  = 2;
	int matrixHeight = 2;

	int aw = matrixWidth;   //       +-+
	int ah = matrixHeight;  //    bh |b|
	int bw = ah;            //    aw +-+
	int bh = aw;            // ah +-+
	int cw = ah;            // ch |a| c
	int ch = ah;            // cw +-+

	assert(sizeof(f32) * matrixWidth  * matrixHeight <= 8192);
	assert(sizeof(f32) * matrixHeight * matrixHeight <= 8192);

	auto a = allocateMatrix(aw, ah); 
	auto b = allocateMatrix(bw, bh); 
	auto c = allocateMatrix(cw, ch); 

	// fillMatrix(a, aw, ah, 2);
	fillMatrixRandomly(a, aw, ah);
	// fillMatrix(b, bw, bh, 2);
	fillMatrixRandomly(b, bw, bh);
	fillMatrix(c, cw, ch, 0);

	if (matrixWidth <= 10 && matrixHeight <= 10) {
		printf("matrix a\n");
		printMatrix(a, aw, ah, stdout, "\t");
		printf("matrix b\n");
		printMatrix(b, bw, bh, stdout, "\t");
	}

	// copy matrices
	e_write(&dev, 0, 0, 0x40, &matrixWidth , sizeof(matrixWidth));
	e_write(&dev, 0, 0, 0x44, &matrixHeight, sizeof(matrixHeight));

	e_write(&dev, 0, 0, 0x2000, a, sizeof(f32) * aw * ah);
	e_write(&dev, 0, 0, 0x4000, b, sizeof(f32) * bw * bh);
	e_write(&dev, 0, 0, 0x6000, c, sizeof(f32) * cw * ch);

	// start epiphany program
	e_start_group(&dev);

	// wait until the epiphany is done
	UserInterrupt epiphanyDone = UserInterrupt::NotDone;
	while (epiphanyDone == UserInterrupt::NotDone) {
		e_read(&dev, 0, 0, 0x24, &epiphanyDone, sizeof(epiphanyDone));
		if (epiphanyDone == UserInterrupt::NotDone) { usleep(1000); }
	}
	assert(epiphanyDone == UserInterrupt::Done);

	u32 cycles, fpops;
	e_read(&dev, 0, 0, 0x48, &cycles, sizeof(u32));
	e_read(&dev, 0, 0, 0x4c, &fpops, sizeof(u32));

	fpops *= 2; // they are all fmadd, but are counted as single ones

	printf("it took %d cycles and %d fpops, should take %d and %d fpops. %f fmadd/cycle.\n", cycles, fpops, cw*ch*aw, cw*ch*aw*2, fpops/2 / (f32) cycles);

	for (int k = 0; k < aw; k += 1) { // this order of loops is faster than the others on epiphany
		for (int j = 0; j < ch; j += 1) {
			for (int i = 0; i < cw; i += 1) {
				c[j*cw + i] += a[j*aw + k] * b[k*bw + i];
			}
		}
	}

	if (matrixWidth <= 10 && matrixHeight <= 10) {
		printf("c should be\n");
		printMatrix(c, cw, ch, stdout, "\t");
	}

	auto c_ = allocateMatrix(cw, ch);
	copyMatrix(c, c_, cw, ch);

	e_read(&dev, 0, 0, 0x6000, c, sizeof(f32) * cw * ch);

	if (matrixWidth <= 10 && matrixHeight <= 10) {
		printf("c is\n");
		printMatrix(c, cw, ch, stdout, "\t");
	}

	bool correctResult = true;
	for (int y = 0; y < ch; y += 1) {
		for (int x = 0; x < cw; x += 1) {
			if (c[y*cw + x] != c_[y*cw + x]) {
				correctResult = false;
				// fmadd is allowed to round differently that c + a * b would
				// therefore when using -O2, -O3 or w/e triggers the epiphany to use fmadd rounding is different
				printf("incorrect result matrix c. position x %d, y %d, value should %f, value is %f\n", x, y, c_[y*cw + x], c[y*cw + x]);
				break;
			}
		}

		if (correctResult == false) { break; }
	}

	if (correctResult == true) {
		printf("correct result matrix c\n");
	}

	freeMatrix(a);
	freeMatrix(b);
	freeMatrix(c);

	// close down Epiphany device
	e_close(&dev);
	e_finalize();

	free(hostExecutable);
	free(epiphanyExecutable);

	return 0;
}

f32* allocateMatrix(int width, int height) {
	f32* m = (f32*) malloc(sizeof(f32) * width * height);
	assert(m != nullptr);
	return m;
}

void fillMatrix(f32* m, int width, int height, f32 value) {
	assert(m != nullptr);
	for (int y = 0; y < height; y += 1) {
		for (int x = 0; x < width; x += 1) {
			m[y*width + x] = value;
		}
	}
}

void fillMatrixRandomly(f32* m, int width, int height) {
	assert(m != nullptr);
	srand(time(nullptr));

	for (int y = 0; y < height; y += 1) {
		for (int x = 0; x < width; x += 1) {
			m[y*width + x] = ((f32) (rand() % 10)) + (1 / (f32) (rand() % 100));
		}
	}
}

void copyMatrix(f32* from, f32* to, int width, int height) {
	assert(from != nullptr);
	assert(to != nullptr);
	memcpy(to, from, sizeof(f32) * width * height);
}

void freeMatrix(f32* m) {
	free(m);
}

void printMatrix(f32* m, int width, int height, FILE* out, const char* indent) {
	assert(m != nullptr);
	for (int y = 0; y < height; y += 1) {
		fprintf(out, "%s", indent);
		for (int x = 0; x < width; x += 1) {
			fprintf(out, "%8.3f ", m[y*width + x]);
		}
		fprintf(out, "\n");
	}
}
