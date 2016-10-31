#!/bin/sh -x

cd HyperNEAT_v2_5
rootDir=`pwd`
boost_root=/usr/local/include/boost-1_36
boost_stage=/usr/local/lib
boost_lib_ext=dylib
boost_gxx=xgcc40
tinyXML_root=${rootDir}/tinyxmldll
JGTL_include=${rootDir}/JGTL/include
ode_root=/usr/local


echo Checking if dev-intel07 or dev-intel09 is host name
if [ `hostname` = "dev-intel07" -o `hostname` = "dev-intel09" ]
then
module unload intelcc/11.0.083
module load gnu

CC=gcc 
CXX=g++

BOOST_INCLUDEDIR=/opt/cus/boost/boost-1.36/include/boost-1_36
BOOST_LIBRARYDIR=/opt/cus/boost/boost-1.36/lib

    echo Setting boost_root
    boost_root=/mnt/home/jclune/hyperneat/hyperneat_libs/boost_1_35_0
    boost_root=/opt/cus/boost/boost-1.36/
    boost_stage=/mnt/home/jclune/hyperneat/hyperneat_libs/boost_1_35_0/boost135HyperNEATLibs
    boost_lib_ext=so
    boost_gxx=gcc41
    echo Setting ode_root to /mnt/home/beckma24
    ode_root=/mnt/home/beckma24/ode-0.10.1

    echo Using Intel compiler
#    module unload hpc-defaults; module load sgicluster
#    CXX=/opt/intel/cce/10.0.025/bin/icpc

elif [ `hostname` = "n178.alice.cme.msu.edu" ]
then

CC=gcc
CXX=g++

BOOST_INCLUDEDIR=/usr/include
BOOST_LIBRARYDIR=/usr/lib

    echo Setting boost_root
    boost_stage=/usr/lib
    boost_lib_ext=so
    boost_gxx=gcc41

    echo Setting ode_root to /home/jclune/sourcecode/ode/ode-0.10.1
    ode_root=/home/jclune/sourcecode/ode/ode-0.10.1

elif [ `hostname` = "endlessforms" -o `hostname` = "devendlessforms" ]
then


    CC=gcc
    CXX=g++

    #boost_root=/usr/include/boost
    boost_root=/usr/include

    BOOST_INCLUDEDIR=/usr/include/
    BOOST_LIBRARYDIR=/usr/lib

    echo Setting boost_root
    boost_stage=/usr/lib
    boost_lib_ext=so
    boost_gxx=gcc41


elif [ `hostname` = "rodan.cse.msu.edu" ]
then
    echo Configuring for Rodan...which probably will not work
else
    echo Not using valid host.  Using local boost_root
fi


echo Building required packages
echo

# build TinyXML
if test -e ${tinyXML_root}/out/libtinyxmldll.${boost_lib_ext}
then
    echo TinyXML exists.  Skipping
    echo
else
    echo Building TinyXML
    cd ${rootDir}
    mkdir zlib/build/cygwin_debug
    mkdir zlib/build/cygwin_release
    cd zlib
    cmake .
    make

    cd ..
    mkdir tinyxmldll/build/cygwin_debug
    mkdir tinyxmldll/build/cygwin_release
    cd tinyxmldll
    cmake .
    make
    cd ${rootDir}

    echo Finished making TinyXML
    echo
fi

# build ODE
if [  `hostname` = "endlessforms" -o `hostname` = "devendlessforms" ] #add hosts that should not compile ode here
    then
	Building for shapes project, so ODE not required. Skipping check. 
else    
    if test -e ${ode_root}/lib/libode.a
    then
	echo ODE exists.  Skipping
	echo
    else
	echo ODE does not exist.  Exitting.
	exit -1
    fi
#    echo Building ODE
#    cd ode-0.10.1
#    ./configure --prefix=`pwd` --enable-double-precision --with-drawstuff=none --enable-demos=no --with-trimesh=gimpact --enable-malloc #--enable-ou does not work
#    make
#    make install-exec
#    cd ${rootDir}

#    echo Finished making ODE
#    echo
fi




# build HyperNEAT
if test -e ${rootDir}/HyperNEAT/out/Hypercube_NEAT
then
    echo Hypercube_NEAT exists.  Skipping
    echo
else
    echo Building HyperNEAT
    cd ${rootDir}
    #rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake
    rm -rf hyperneatLibs
    mkdir hyperneatLibs
    cd hyperneatLibs 
    ln -s ${boost_stage}/libboost_thread-gcc41-mt.so libboost_thread-gcc42-mt.so
    ln -s ${boost_stage}/libboost_filesystem-gcc41-mt.so  libboost_filesystem-gcc42-mt.so
    ln -s ${boost_stage}/libboost_system-gcc41-mt.so libboost_system-gcc42-mt.so
    cd ${rootDir}
    cd HyperNEAT
    cmake -DBoost_LIBRARY_DIRS:string=${BOOST_LIBRARYDIR} -DBoost_INCLUDE_DIRS:string=${BOOST_INCLUDEDIR} .
    if [  `hostname` = "endlessforms" -o `hostname` = "devendlessforms" ] #add hosts that should not compile ode here
    then
	cmake -DToODEorNot:string=   #purposefully empty so ode does not get linked in
    else
	cmake -DToODEorNot:string=ode
    fi

    if [  `hostname` = "endlessforms" -o `hostname` = "devendlessforms" ] #add hosts that should not compile ode here
    then
	echo i know i am on host endlessforms.com or devendlessforms

	cmake -DCOMPILE_ODE=OFF -DCMAKE_INSTALL_PREFIX=${rootDir}/out -DJGTL_INCLUDE=${JGTL_include} -DBoost_LIBRARY_DIRS:string=${BOOST_LIBRARYDIR} -DBoost_INCLUDE_DIRS:string=${BOOST_INCLUDEDIR} -DBOOST_ROOT=${boost_root} -DBOOST_STAGE:string=${boost_stage} -DEXECUTABLE_OUTPUT_PATH=${rootDir}/out -DJGTL_INCLUDE=${JGTL_include} -DLIBRARY_OUTPUT_PATH:string=${rootDir}/hyperneatLibs -DTINYXMLDLL_INCLUDE=${tinyXML_root}/include -DTINYXMLDLL_LIB=${tinyXML_root}/out -DCMAKE_COMPILER_IS_GNUCXX:string=/opt/intel/cce/10.0.025/bin/icpc -DENDLESSFORMS=ON
    else
	echo i know my name is simon
	cmake -DCOMPILE_ODE=ON -DCMAKE_INSTALL_PREFIX=${rootDir}/out -DJGTL_INCLUDE=${JGTL_include} -DBoost_LIBRARY_DIRS:string=${BOOST_LIBRARYDIR} -DBoost_INCLUDE_DIRS:string=${BOOST_INCLUDEDIR} -DBOOST_ROOT=${boost_root} -DBOOST_STAGE:string=${boost_stage} -DEXECUTABLE_OUTPUT_PATH=${rootDir}/out -DJGTL_INCLUDE=${JGTL_include} -DLIBRARY_OUTPUT_PATH:string=${rootDir}/hyperneatLibs -DTINYXMLDLL_INCLUDE=${tinyXML_root}/include -DTINYXMLDLL_LIB=${tinyXML_root}/out -DDRAWSTUFF_INCLUDE:string=${rootDir}/ode-0.10.1/include -DODE_LIB:string=${ode_root}/lib -DODE_INCLUDE:string=${ode_root}/include -DCMAKE_COMPILER_IS_GNUCXX:string=/opt/intel/cce/10.0.025/bin/icpc 
    fi
    make $@ #--debug=v
    cd ${rootDir}

    echo Finished making Hypercube_NEAT
    echo
fi
