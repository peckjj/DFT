int main(int argc, char **argv);
int parseWavFile(char *fileName, char *outputName);
int dft(double complex *out, double complex *data, long numSamples, long maxFreq);

#define RIFF *((uint32_t *)("RIFF"))
#define WAVE *((uint32_t *)("WAVE"))
#define FMT *((uint32_t *)("fmt "))
#define DATA *((uint32_t *)("data"))

typedef struct __attribute__((__packed__)) RiffChunk
{
    uint32_t chunkId;
    uint32_t chunkSize;
    uint32_t format;
} RiffChunk;

typedef struct __attribute__((__packed__)) FmtChunk
{
    uint32_t chunkId;
    uint32_t chunkSize;
    uint16_t audioFormat;
    uint16_t nChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} FmtChunk;

typedef struct __attribute__((__packed__)) DataChunk
{
    uint32_t chunkId;
    uint32_t chunkSize;
} DataChunk;
