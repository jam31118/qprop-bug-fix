optimargs=-O3 -ffast-math -march=native
gslargs=-I/usr/include -L/usr/lib -lgsl -lgslcblas
mpiargs=-I/usr/include -L/usr/lib -lmpi -lmpi_cxx -DHAVE_MPI
# for Ubuntu this seems to work for Open MPI
mpiargs=-I/usr/include/openmpi -L/usr/lib -lmpi -lmpi_cxx -DHAVE_MPI
