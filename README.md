# Power Quality Waveform Analyser

C99 command-line tool that reads a CSV log of 3-phase power quality data and writes a summary report.

## Build and run (CLion)
1. Open this folder in CLion.
2. Build the `waveform_analyser` target.
3. Run with the CSV file path as the argument.

## Build and run (command line)
```bash
gcc -std=c99 -Wall -Wextra -o waveform_analyser main.c waveform.c io.c -lm
./waveform_analyser power_quality_log.csv
```

## Output
The program writes `results.txt` to the current directory.

## GitHub repository
https://github.com/b11j2a3/PPFE_COURSEWORK
