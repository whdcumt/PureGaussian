#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <conio.h>
#include <malloc.h>
#define SQRT_2PI 2.506628274631

#pragma pack(push,2)
typedef struct
{
	uint16_t bfType;
	uint32_t bfileSize;
	uint32_t bfReserved;
	uint32_t bOffBits;
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount; // глубина цвета
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} bmp_header_t;
#pragma pack(pop)

typedef struct
{
	bmp_header_t header;
	char *data;
} bmp_t;

int win_size(double);
double Gauss(double, double);
double *GaussAlgorithm(int, double);
void bmp_free(bmp_t *);
bmp_t *bmp_open(FILE *);
int bmp_write(bmp_t*, FILE*);
bmp_t *GaussBlur(bmp_t*, double);

int main(int argc, char** argv)
{
	char *InputName, *OutputName;
	FILE *InputFile, *OutputFile;
	bmp_t *bmp = NULL, *blur = NULL;

	InputName = "input.bmp"; OutputName = "output.bmp";
	if (0 != fopen_s(&InputFile, InputName, "rb"))
	{
		printf("File not found\n");
		_getch();
		return 1;
	}
	bmp = bmp_open(InputFile);
	fclose(InputFile);

	OutputFile = fopen(OutputName, "wb");
	if (OutputFile == NULL) {
		fprintf(stderr, "Can't open %s\n", OutputName);
		return EXIT_FAILURE;
	}

	blur = GaussBlur(bmp, 4.0);
	bmp_write(blur, OutputFile);  
	fclose(OutputFile);
	bmp_free(bmp);
	free(blur);
	getchar();

	return 0;
}

bmp_t *GaussBlur(bmp_t *src, double sigma) 
{
	int	row, col, col_r, col_g, col_b, winsize, halfsize, k, count, rows, count1, count2, count3;
	int width, height;
	double  row_g, row_b, row_r, col_all;
	unsigned char  r_r, r_b, r_g, c_all;
	char *tmp;
	double *algorithm;

	count=0;
	width = 3*src->header.biWidth; height = src->header.biHeight;

	if ((width % 4) != 0) width += (4 - (width % 4)); 
	bmp_t *blur;
	blur = (bmp_t*)malloc(sizeof(bmp_t));
	blur->header = src->header;
	blur->header.biWidth = src->header.biWidth;
	blur->header.biHeight = src->header.biHeight;
	blur->header.biSizeImage = width * blur->header.biHeight;
	blur->data = (char*)malloc(blur->header.biSizeImage);

	winsize = win_size(sigma);
	algorithm = GaussAlgorithm(winsize, sigma); 
	winsize *= 3; 
	halfsize = winsize / 2;

	tmp = (char*)calloc(width * height, sizeof(char)); 
	
	for (row = 0; row < height; row++)
	{
		col_r = 0;
		col_g = 1;
		col_b = 2;
		for (rows = 0; rows < width; rows += 3)
		{
			row_r = row_g = row_b = 0.0;
			count1 = count2 = count3 = 0;
		
			for (k = 1; k < winsize; k += 3)
			{
				if ((k + col_r - halfsize >= 0) && (k + col_r - halfsize < width))
				{
					r_r = *(src->data + row * width + col_r + k - halfsize);
					row_r += (int)(r_r)* algorithm[count1];
					count1++;
				}
				if ((k + col_g - halfsize >= 0) && (k + col_g - halfsize < width))
				{
					r_g = *(src->data + row * width + col_g + k - halfsize);
					row_g += (int)(r_g)* algorithm[count2];
					count2++;
				}

				if ((k + col_b - halfsize >= 0) && (k + col_b - halfsize < width))
				{
					r_b = *(src->data + row * width + col_b + k - halfsize);
					row_b += (int)(r_b)* algorithm[count3];
					count3++;
				}
			}

			*(tmp + row * width + col_r) = (unsigned char)(ceil(row_r));
			*(tmp + row * width + col_g) = (unsigned char)(ceil(row_g));
			*(tmp + row * width + col_b) = (unsigned char)(ceil(row_b));
			col_r += 3;
			col_g += 3;
			col_b += 3;
		}
	}
	
	winsize /= 3;
	halfsize = winsize / 2;
	for (col = 0; col < width; col++)
		for (row = 0; row < height; row++)
		{
		col_all = 0.0;
		for (k = 0; k < winsize; k++)
			if ((k + row - halfsize >= 0) && (k + row - halfsize < height))
			{
			c_all = *(tmp + (row + k - halfsize) * width + col);
			col_all += ((int)c_all) * algorithm[k];
			}
		*(blur->data + row * width + col) = (unsigned char)(ceil(col_all));
		}
	
	free(tmp);
	free(algorithm);

	return blur;
}

double* GaussAlgorithm(int win_size, double sigma)
{
	int wincenter, x;
	double *kern, sum = 0.0;
	wincenter = win_size / 2;
	kern = (double*)calloc(win_size, sizeof(double));

	for (x = 0; x < wincenter + 1; x++)
	{
		kern[wincenter - x] = kern[wincenter + x] = Gauss(sigma, x);
		sum += kern[wincenter - x] + ((x != 0) ? kern[wincenter + x] : 0.0);
	}
	for (x = 0; x < win_size; x++)
		kern[x] /= sum;

	return kern;
}

double Gauss(double sigma, double x)
{
	return exp(-(x * x) / (2.0 * sigma * sigma)) / (sigma * SQRT_2PI);
}

int win_size(double sigma)
{
	return (1 + (((int)ceil(3 * sigma)) * 2)); 
}

bmp_t *bmp_open(FILE *f) {
	bmp_t *bmp;
	bmp = (bmp_t *)malloc(sizeof(bmp_t));
	bmp->data = NULL;

	if (fread(&(bmp->header), sizeof(bmp_header_t), 1, f)) {
		bmp->data = (char*)malloc(bmp->header.biSizeImage);
		if (fread(bmp->data, bmp->header.biSizeImage, 1, f))
			return bmp;
	}
	fprintf(stderr, "Error reading file");
	bmp_free(bmp);
	return NULL;
}

int bmp_write(bmp_t *bmp, FILE *out) {
	return fwrite(&(bmp->header), sizeof(bmp_header_t), 1, out)
		&& fwrite(bmp->data, bmp->header.biSizeImage, 1, out);
}

void bmp_free(bmp_t *bmp)
{
	if (bmp == NULL) return;
	if (bmp->data != NULL) free(bmp->data);
	free(bmp);
}
