Algorithms for BangleJS
=======================

Algorithms for step counting and heart rate for the Bangle JS.

Compile:

```bash
make
```

Run:

```bash
./build/benchmarkSC ../tests/ references.csv results.csv
```

Where `../tests` is the folder where the csv files are located, `references.csv` is the file with the reference step count and `results.csv` is the filename of the output file. Change accordingly.