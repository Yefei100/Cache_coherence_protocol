Cache_coherence_protocol
========================

A C++ implementation of cache coherence protocol, including MSI, MESI and Dragon.

Configuration command:
make clean
make
./smp_cache cacheSize assoc blockSize numProc protocol traceFile

where numProc set to 4, and protocol = {0: MSI, 1:MESI, 2:Dragon}
traceFile is ./Validation/canneal.04t.longTrace(debug)


