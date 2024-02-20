#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <math.h>

#include "wav_parse.h"
#include "utils.h"

#define OUTPUT_SAMPLES 10000
#define METADATA_BUFFER_SIZE 80000

#define SKIP_TO_SECOND 50
#define MAX_FREQ 750

int main(int argc, char **argv)
{
	int res = 44100;

	double complex *data = malloc(sizeof(double complex) * res);
	double complex *dftOut = malloc(sizeof(double complex) * MAX_FREQ);

	sinWave(data, 440, 1, 0, res);
	dft(dftOut, data, res, MAX_FREQ);

	printf("[");

	for (int i = 0; i < MAX_FREQ; i++)
	{
		printf("%0.4f, ", creal(dftOut[i]));
	}

	printf("]\n");

	free(data);
	free(dftOut);
	return 0;
}

int main2(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Invalid args.\n");
		return -1;
	}

	char *fileName = argv[1];
	char *outputName = argv[2];

	printf("Reading from %s\n", fileName);

	return parseWavFile(fileName, outputName);
}

int parseWavFile(char *fileName, char *outputName)
{
	srand(time(NULL));

	int headerSize = sizeof(RiffChunk) + sizeof(FmtChunk) + sizeof(DataChunk);

	char *fname = "wav_parse.parseWavFile()";

	char *buffer = malloc(headerSize);

	printf("Opening %s\n", fileName);

	FILE *file = fopen(fileName, "rb");
	FILE *output = fopen(outputName, "wb");
	if (!file)
	{
		printf("Error reading %s: %s\n", fileName, strerror(errno));
		return -1;
	}

	if (fread(buffer, 1, headerSize, file) != headerSize)
	{
		printf("Warning, buffer not filled...\n");
	}

	fwrite(buffer, 1, headerSize, output);

	RiffChunk *riffChunk;
	FmtChunk *fmtChunk;
	DataChunk *dataChunk;

	riffChunk = (RiffChunk *)buffer;
	fmtChunk = (FmtChunk *)(&(buffer[sizeof(RiffChunk)]));
	dataChunk = (DataChunk *)(&(buffer[sizeof(RiffChunk) + sizeof(FmtChunk)]));

	dataChunk->chunkSize = OUTPUT_SAMPLES * fmtChunk->blockAlign;

	printf("====================================================================\n");
	printf("Metadata for %s:\n", fileName);
	printf("Chunk 1 ID: %s\n"
		   "File Size: %u B\n"
		   "Format: %s\n"
		   "\n"
		   "Chunk 2 ID: %s\n"
		   "Chunk 2 Size: %u B\n"
		   "Audio Format: %d%s\n"
		   "Number of Channels: %u\n"
		   "Sample Rate: %dHz\n"
		   "Byte Rate: %d B / s\n"
		   "Block Align (Frame Size): %d B\n"
		   "Bits per Sample: %u b\n"
		   "\n"
		   "Chunk 3 ID: %s\n"
		   "Data Size: %u B\n"
		   "Samples per Second: %d\n"
		   "I Real: %0.1f\n"
		   "I Imag: %0.1f\n",
		   riffChunk->chunkId == RIFF
			   ? "RIFF"
			   : "NOT RIFF!",
		   riffChunk->chunkSize + 8,
		   riffChunk->format == WAVE ? "WAVE" : "NOT WAVE!",
		   fmtChunk->chunkId == FMT ? "fmt " : "NOT fmt !",
		   fmtChunk->chunkSize,
		   fmtChunk->audioFormat,
		   fmtChunk->audioFormat == 1 ? " (PCM)" : "",
		   fmtChunk->nChannels,
		   fmtChunk->sampleRate,
		   fmtChunk->byteRate,
		   fmtChunk->blockAlign,
		   fmtChunk->bitsPerSample,
		   dataChunk->chunkId == DATA ? "data" : "NOT DATA!",
		   dataChunk->chunkSize,
		   fmtChunk->byteRate / fmtChunk->blockAlign,
		   creal(I),
		   cimag(I));

	printf(
		"Audio Size: %d B\n"
		"Length: %0.2f s\n",
		dataChunk->chunkSize,
		((dataChunk->chunkSize) + 0.0) / ((fmtChunk->byteRate) + 0.0));

	// Number of samples in a single second
	int samplesPerSecond = fmtChunk->byteRate / fmtChunk->blockAlign;

	int32_t *aData = malloc(sizeof(int32_t) * samplesPerSecond);
	int32_t *bData = malloc(sizeof(int32_t) * samplesPerSecond);

	double complex *cData = malloc(sizeof(double complex) * samplesPerSecond);
	double complex *cOutput = malloc(sizeof(double complex) * MAX_FREQ);

	int16_t random;

	fseek(file, fmtChunk->sampleRate * fmtChunk->blockAlign * SKIP_TO_SECOND, SEEK_CUR);

	for (int i = 0; i < samplesPerSecond; i++)
	{
		random = (int16_t)rand();

		if (fread(buffer, 1, fmtChunk->blockAlign, file) != fmtChunk->blockAlign)
		{
			printf("Error reading data!");
			return -1;
		}

		aData[i] = *((int32_t *)&(buffer[0]));
		bData[i] = *((int32_t *)&(buffer[fmtChunk->blockAlign / 2]));
		aData[i] = (aData[i] << 8) >> 8;
		bData[i] = (bData[i] << 8) >> 8;

		cData[i] = CMPLX(map(i, 0, samplesPerSecond, 0.0, 1.0), map(aData[i], -(2 << 23), (2 << 23) - 1, -1.0, 1.0));

		fwrite(buffer, 1, fmtChunk->blockAlign, output);
	}

	dft(cOutput, cData, samplesPerSecond, MAX_FREQ);

	printf("Real Data:\n[\n\n");

	for (int i = 0; i < MAX_FREQ; i++)
	{
		printf("%0.4f, ", creal(cOutput[i]));
	}

	if (fclose(file))
	{
		printf("Error closing %s\n", fileName);
		return -1;
	}
	if (fclose(output))
	{
		printf("Error closing %s\n", outputName);
		return -1;
	}

	printf("]\n");

	printf("====================================================================\n");

	free(buffer);
	free(aData);
	free(bData);
	free(cData);
	free(cOutput);

	return 0;
}

int dft(double complex *out, double complex *data, long numSamples, long maxFreq)
{
	double complex w = cexp(-I * 2 * M_PI / numSamples);
	double complex sum;

	for (int i = 0; i < maxFreq; i++)
	{
		sum = CMPLX(0, 0);

		for (int j = 0; j < numSamples; j++)
		{
			sum += cpow(w, CMPLX(i * j, 0)) * data[j];
		}

		out[i] = sum;
	}

	return 0;
}
