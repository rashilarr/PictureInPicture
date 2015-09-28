#include <stdio.h>
#include <stdlib.h>

#define WIDTH_CIF	352
#define HEIGHT_CIF	288
#define WIDTH_QCIF	176
#define HEIGHT_QCIF	144

typedef struct {
	unsigned short type;
	unsigned long size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned long offsetbits;
} BITMAPFILEHEADER;

typedef struct {
	unsigned long size;
	unsigned long width;
	unsigned long height;
	unsigned short planes;
	unsigned short bitcount;
	unsigned long compression;
	unsigned long sizeimage;
	long xpelspermeter;
	long ypelspermeter;
	unsigned long colorsused;
	unsigned long colorsimportant;
} BITMAPINFOHEADER;

int frames1, frames2;	/* Number of frames to be embedded	*/

/*	ARRAY FOR THE CIF	*/

unsigned char CIF_Y[WIDTH_CIF * HEIGHT_CIF];
unsigned char CIF_CB[(WIDTH_CIF * HEIGHT_CIF) >> 2];
unsigned char CIF_CR[(WIDTH_CIF * HEIGHT_CIF) >> 2];

/*	ARRAY FOR THE QCIF	*/

unsigned char QCIF_Y[WIDTH_QCIF * HEIGHT_QCIF];
unsigned char QCIF_CB[(WIDTH_QCIF * HEIGHT_QCIF) >> 2];
unsigned char QCIF_CR[(WIDTH_QCIF * HEIGHT_QCIF) >> 2];

void convert();
void PIP();

int main(int argc, char *argv[])
{
	FILE *fin1, *fin2, *fout_pip, *fout_converted;
	unsigned int filesize1, filesize2, NoOfFrames, Frames, filesize;
	if((fin1 = fopen(argv[1], "rb")) == NULL)
	{
		printf("Error : Opening first input YUV file.\n");
		return -1;
	}
	if((fin2 = fopen(argv[2], "rb")) == NULL)
	{
		printf("Error : Opening second input YUV file.\n");
		return -1;
	}
	if((fout_pip = fopen(argv[3], "wb")) == NULL)
	{
		printf("Error : Opening output YUV file.\n");
		return -1;
	}
	if((fout_converted = fopen(argv[4], "wb")) == NULL)
	{
		printf("Error : Opening output YUV file.\n");
		return -1;
	}

	fseek(fin1, 0, SEEK_END);
	fseek(fin2, 0, SEEK_END);

	filesize1 = ftell(fin1);
	filesize2 = ftell(fin2);

	rewind(fin1);
	rewind(fin2);

	filesize = (filesize1 < filesize2) ? filesize1 : filesize2;

	NoOfFrames = filesize / ((WIDTH_CIF * HEIGHT_CIF * 3) >> 1);

	printf("Filesize = %d\n", filesize);
	printf("Filesize = %d\n", filesize1);
	printf("Filesize = %d\n", filesize2);
	printf("Number of Frames = %d\n", NoOfFrames);

	for(Frames = 0; Frames < NoOfFrames; Frames++)
	{
		/*	Read the YUV which is to be downsampled	*/
		fread(CIF_Y, sizeof(unsigned char), WIDTH_CIF * HEIGHT_CIF, fin2);
		fread(CIF_CB, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fin2);
		fread(CIF_CR, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fin2);

		/*	Convert the CIF YUV from the second file to QCIF	*/
		convert();
		printf("Processing Frame # %d\n", Frames);

		/*	Read YUV for the main picture	*/
		fread(CIF_Y, sizeof(unsigned char), WIDTH_CIF * HEIGHT_CIF, fin1);
		fread(CIF_CB, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fin1);
		fread(CIF_CR, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fin1);

		/*	Merge the two videos	*/
		PIP();

		/*	Store the downsampled video	*/
		fwrite(QCIF_Y, sizeof(unsigned char), WIDTH_QCIF * HEIGHT_QCIF, fout_converted);
		fwrite(QCIF_CB, sizeof(unsigned char), (WIDTH_QCIF * HEIGHT_QCIF) >> 2, fout_converted);
		fwrite(QCIF_CR, sizeof(unsigned char), (WIDTH_QCIF * HEIGHT_QCIF) >> 2, fout_converted);

		/*	Store the Picture In Picture Video	*/
		fwrite(CIF_Y, sizeof(unsigned char), WIDTH_CIF * HEIGHT_CIF, fout_pip);
		fwrite(CIF_CB, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fout_pip);
		fwrite(CIF_CR, sizeof(unsigned char), (WIDTH_CIF * HEIGHT_CIF) >> 2, fout_pip);
	}

	fclose(fin1);
	fclose(fin2);
	fclose(fout_pip);
	fclose(fout_converted);
	return(0);
}

void convert()
{
	int row, col;
	unsigned char *cif, *qcif;

	/*	First convert the luminance from CIF to QCIF	*/

	cif = (unsigned char*) CIF_Y;
	qcif = (unsigned char*) QCIF_Y;

	for(row = 0; row < HEIGHT_QCIF; row++)
	{
		for(col = 0; col < WIDTH_QCIF; col++)
		{
			*qcif++ = *cif++;
			*cif++;
		}
		cif = cif + WIDTH_CIF;
		
	}

	/*	Next convert the chrominance from CIF to QCIF	*/

	cif = (unsigned char*) CIF_CB;
	qcif = (unsigned char*) QCIF_CB;

	for(row = 0; row < (HEIGHT_QCIF >> 1); row++)
	{
		for(col = 0; col < (WIDTH_QCIF >> 1); col++)
		{
			*qcif++ = *cif++;
			*cif++;
		}
		cif = cif + (WIDTH_CIF >> 1);
		
	}

	cif = (unsigned char*) CIF_CR;
	qcif = (unsigned char*) QCIF_CR;

	for(row = 0; row < (HEIGHT_QCIF >> 1); row++)
	{
		for(col = 0; col < (WIDTH_QCIF >> 1); col++)
		{
			*qcif++ = *cif++;
			*cif++;
		}
		cif = cif + (WIDTH_CIF >> 1);
		
	}
	return;
}

void PIP()
{
	unsigned char *cif;
	unsigned char *qcif;
	int row = 0, col = 0;

	/*	Merge the luminance	*/
	cif = (unsigned char*) (CIF_Y + WIDTH_CIF * HEIGHT_CIF - 1);
	qcif = (unsigned char*) (QCIF_Y + WIDTH_QCIF * HEIGHT_QCIF - 1);

	while(row < HEIGHT_QCIF)
	{
		col = 0;
		while(col < WIDTH_QCIF)
		{
			*cif-- = *qcif--;
			++col;
		}
		cif = cif - WIDTH_CIF + WIDTH_QCIF;
		++row;
	}

	/*	Merge the chrominance CB	*/
	cif = (unsigned char*) (CIF_CB + WIDTH_CIF * (HEIGHT_CIF >> 2) - 1);
	qcif = (unsigned char*) (QCIF_CB + WIDTH_QCIF * (HEIGHT_QCIF >> 2) - 1);

	row = 0;

	while(row < (HEIGHT_QCIF >> 1))
	{
		col = 0;
		while(col < (WIDTH_QCIF >> 1))
		{
			*cif-- = *qcif--;
			++col;
		}
		cif = cif - (WIDTH_CIF >> 1) + (WIDTH_QCIF >> 1);
		++row;
	}

	/*	Merge the chrominane CR	*/
	cif = (unsigned char*) (CIF_CR + WIDTH_CIF * (HEIGHT_CIF >> 2) - 1);
	qcif = (unsigned char*) (QCIF_CR + WIDTH_QCIF * (HEIGHT_QCIF >> 2) - 1);

	row = 0;

	while(row < (HEIGHT_QCIF >> 1))
	{
		col = 0;
		while(col < (WIDTH_QCIF >> 1))
		{
			*cif-- = *qcif--;
			++col;
		}
		cif = cif - (WIDTH_CIF >> 1) + (WIDTH_QCIF >> 1);
		++row;
	}

	return;
}
