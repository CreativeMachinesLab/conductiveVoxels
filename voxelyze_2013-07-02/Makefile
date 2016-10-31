# compiler flags
# -g --Enable debugging
# -Wall --Turn on all warnings
# -D_USE_FIXED_PROTOTYPES_
# --Force the compiler to use the correct headers
# -ansi --Don't use GNU ext; do use ansi standard.
#
# may have to run "source /opt/intel/composerxe/bin/compilervars.sh intel64" to get intel compiler?

# CXX=icpc
# CC=icpc
CXX=g++
CC=g++
#INCLUDE=-I./zlib-1.2.5 -I./Voxelyze -I./TinyXML
# INCLUDE= -I./Voxelyze -I./Voxelyze/Utils -l./Voxelyze/Utils/zlib
INCLUDE= -I./Voxelyze -I./Voxelyze/Utils
# CFLAGS= -xhost -O3 -Wall $(INCLUDE)
CFLAGS= -O3 -Wall $(INCLUDE)
CPPFLAGS= $(CFLAGS)

Vox_src=\
Voxelyze/Utils/XML_Rip.cpp\
Voxelyze/VXS_Voxel.cpp\
Voxelyze/VXS_Bond.cpp\
Voxelyze/VX_Sim.cpp\
Voxelyze/VX_SimGA.cpp\
Voxelyze/VX_Object.cpp\
Voxelyze/VX_MeshUtil.cpp\
Voxelyze/VX_FRegion.cpp\
Voxelyze/VX_FEA.cpp\
Voxelyze/VX_Environment.cpp\
Voxelyze/Utils/Mesh.cpp\
Voxelyze/main.cpp

tinyxml_src=\
Voxelyze/Utils/tinyxml.cpp\
Voxelyze/Utils/tinyxmlerror.cpp\
Voxelyze/Utils/tinyxmlparser.cpp


# zlib_src=\
# zlib-1.2.5/adler32.c\
# zlib-1.2.5/compress.c\
# zlib-1.2.5/crc32.c\
# zlib-1.2.5/deflate.c\
# zlib-1.2.5/gzclose.c\
# zlib-1.2.5/gzlib.c\
# zlib-1.2.5/gzread.c\
# zlib-1.2.5/gzwrite.c\
# zlib-1.2.5/infback.c\
# zlib-1.2.5/inffast.c\
# zlib-1.2.5/inflate.c\
# zlib-1.2.5/inftrees.c\
# zlib-1.2.5/trees.c\
# zlib-1.2.5/uncompr.c\
# zlib-1.2.5/zutil.c

OBJECT_FILES=\
Voxelyze/Utils/XML_Rip.o\
Voxelyze/VXS_Voxel.o\
Voxelyze/VXS_Bond.o\
Voxelyze/VX_Sim.o\
Voxelyze/VX_SimGA.o\
Voxelyze/VX_Object.o\
Voxelyze/VX_MeshUtil.o\
Voxelyze/VX_FRegion.o\
Voxelyze/VX_FEA.o\
Voxelyze/VX_Environment.o\
Voxelyze/Utils/Mesh.o\
Voxelyze/main.o\
Voxelyze/Utils/tinyxml.o\
Voxelyze/Utils/tinyxmlerror.o\
Voxelyze/Utils/tinyxmlparser.o\
# zlib-1.2.5/adler32.o\
# zlib-1.2.5/compress.o\
# zlib-1.2.5/crc32.o\
# zlib-1.2.5/deflate.o\
# zlib-1.2.5/gzclose.o\
# zlib-1.2.5/gzlib.o\
# zlib-1.2.5/gzread.o\
# zlib-1.2.5/gzwrite.o\
# zlib-1.2.5/infback.o\
# zlib-1.2.5/inffast.o\
# zlib-1.2.5/inflate.o\
# zlib-1.2.5/inftrees.o\
# zlib-1.2.5/trees.o\
# zlib-1.2.5/uncompr.o\
# zlib-1.2.5/zutil.o




all: voxelize

# zlib-1.2.5/%.o : zlib-1.2.5/%.c
# 	icc -c $(CFLAGS) $< -o $@

voxelize: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

#vox.o: $(Vox_src)#
#	$(CC) $(CFLAGS) $(INCLUDE) $(Vox_src)

#zlib.o: $(zlib_src)
#	$(CC) $(CFLAGS) $(INCLUDE) $(zlib_src)

clean:
	rm -rf *.o voxelize */*.o */Utils/*.o
