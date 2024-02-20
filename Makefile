files := wav_parse.c wav_parse.h utils.h utils.c
targets := wav_parse.c utils.c
output := wp
CC_FLAGS := -lm

CLEAN_OTHER := *Copy.wav *copy.wav *.txt

$(output): $(files)
	gcc -g $(targets) $(CC_FLAGS) -o wp

clean:
	rm ./$(output) $(CLEAN_OTHER) -f

test:
	make && \
	clear && \