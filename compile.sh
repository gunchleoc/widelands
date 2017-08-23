#!/bin/sh
echo " "
echo "###########################################################"
echo "#     Script to simplify the compilation of Widelands     #"
echo "###########################################################"
echo " "
echo "  Because of the many different systems Widelands might be"
echo "  compiled on, we unfortunately can not provide a simple"
echo "  way to prepare your system for compilation. To ensure"
echo "  that your system is ready, best check"
echo "  http://wl.widelands.org/wiki/BuildingWidelands"
echo " "
echo "  You will often find helpful hands at our"
echo "  * IRC Chat: http://wl.widelands.org/webchat/"
echo "  * Forums: http://wl.widelands.org/forum/"
echo "  * Mailing List: http://wl.widelands.org/wiki/MailLists/"
echo " "
echo "  Please post your bug reports and feature requests at:"
echo "  https://bugs.launchpad.net/widelands"
echo " "
echo "  For instructions on how to adjust options and build with"
echo "  CMake, please take a look at"
echo "  https://wl.widelands.org/wiki/BuildingWidelands/."
echo " "
echo "###########################################################"
echo " "

## Option to avoid building and linking website-related executables etc.
BUILDTYPE="Debug"
BUILD_TRANSLATIONS="ON"
BUILD_UTILS="ON"
BUILD_WEBSITE="ON"
PRINT_HELP=0
while [ "$1" != "" ]; do
  if [ "$1" = "--help" ]; then
    PRINT_HELP=1
  elif [ "$1" = "--minimal" -o "$1" = "-m" ]; then
    BUILDTYPE="Release"
    BUILD_TRANSLATIONS="OFF"
    BUILD_UTILS="OFF"
    BUILD_WEBSITE="OFF"
  elif [ "$1" = "--release" -o "$1" = "-r" ]; then
    BUILDTYPE="Release"
  elif [ "$1" = "--no-translations" -o "$1" = "-t" ]; then
    BUILD_TRANSLATIONS="OFF"
  elif [ "$1" = "--no-utils" -o "$1" = "-u" ]; then
    BUILD_UTILS="OFF"
  elif [ "$1" = "--no-website" -o "$1" = "-w" ]; then
    BUILD_WEBSITE="OFF"
  fi
  shift
done
if [ $PRINT_HELP = 1 ]; then
  echo "  You can build Widelands as a debug or as a release build."
  echo "  The debug build is the default option; in order to create"
  echo "  a release build, call"
  echo ""
  echo "    ./compile.sh -r"
  echo ""
  echo "  The following options are available for speeding up the"
  echo "  build:"
  echo " "
  echo "    -m or --minimal:"
  echo "          Create the fastest build possible. This is a"
  echo "          release build with no translations or additional"
  echo "          executables."
  echo " "
  echo "    -u or --no-utils:"
  echo "          Omit building and linking the utility"
  echo "          executables (wl_create_spritemap)"
  echo " "
  echo "    -w or --no-website:"
  echo "          Omit building and linking the website-related"
  echo "          executables (wl_map_info, wl_map_object_info)"
  echo " "
  echo "    -t or --no-translations:"
  echo "          Omit building translations"
  echo " "
  echo "###########################################################"
  echo " "
  exit 0
else
  echo "  Building Widelands with the following options:"
  echo ""
  if [ $BUILDTYPE = "Release" ]; then
    echo "    * RELEASE build"
  else
    echo "    * DEBUG build"
  fi
  if [ $BUILD_TRANSLATIONS = "ON" ]; then
    echo "    * Translations"
  else
    echo "    * NO translations"
  fi
  if [ $BUILD_UTILS = "ON" ]; then
    echo "    * Utility executables"
  else
    echo "    * NO utility executables"
  fi
  if [ $BUILD_WEBSITE = "ON" ]; then
    echo "    * Website executables"
  else
    echo "    * NO website executables"
  fi
fi

echo " "
echo "  Call ./compile.sh --help for further options"
echo " "
echo "###########################################################"
echo " "

######################################
# Definition of some local variables #
######################################
buildtool="" #Use ninja by default, fall back to make if that is not available.
######################################


######################################
#    Definition of some functions    #
######################################
  # Check basic stuff
  basic_check () {
    # Check whether the script is run in a widelands main directory
    if ! [ -f src/wlapplication.cc ] ; then
      echo "  This script must be run from the main directory of the widelands"
      echo "  source code."
      exit 1
    fi
    return 0
  }

  set_buildtool () {
    #Defaults to ninja, but if that is not found, we use make instead
    if [ `command -v ninja` ] ; then
      buildtool="ninja"
    #On some systems (most notably Fedora), the binary is called ninja-build
    elif [ `command -v ninja-build` ] ; then
      buildtool="ninja-build"
    #... and some systems refer to GNU make as gmake
    elif [ `command -v gmake` ] ; then
      buildtool="gmake"
    else
      buildtool="make"
    fi
  }

  # Check if directories / links already exists and create / update them if needed.
  prepare_directories_and_links () {
    test -d build/locale || mkdir -p build/locale
    test -e data/locale || ln -s ../build/locale data/locale
    return 0
  }

  # Compile Widelands
  compile_widelands () {
    if [ $buildtool = "ninja" ] || [ $buildtool = "ninja-build" ] ; then
      cmake -G Ninja .. -DCMAKE_BUILD_TYPE=$BUILDTYPE -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE -DOPTION_BUILD_UTILITIES=$BUILD_UTILS -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS
    else
      cmake .. -DCMAKE_BUILD_TYPE=$BUILDTYPE -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE -DOPTION_BUILD_UTILITIES=$BUILD_UTILS -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS
    fi

    $buildtool
    return 0
  }

  # Remove old and move newly compiled files
  move_built_files () {
    rm  -f ../VERSION || true
    rm  -f ../widelands || true

    rm  -f ../wl_map_object_info || true
    rm  -f ../wl_map_info || true
    rm  -f ../wl_create_spritemap || true

    mv VERSION ../VERSION
    mv src/widelands ../widelands

    if [ $BUILD_UTILS = "ON" ]; then
        mv ../build/src/utils/wl_create_spritemap ../wl_create_spritemap
    fi
    if [ $BUILD_WEBSITE = "ON" ]; then
        mv ../build/src/website/wl_map_object_info ../wl_map_object_info
        mv ../build/src/website/wl_map_info ../wl_map_info
    fi
    return 0
  }

  create_update_script () {
    # First check if this is an bzr checkout at all - only in that case,
    # creation of a script makes any sense.
    if ! [ -f .bzr/branch-format ] ; then
      echo "You don't appear to be using Bazaar. An update script will not be created"
      return 0
    fi
      rm -f update.sh || true
      cat > update.sh << END_SCRIPT
#!/bin/sh
echo "################################################"
echo "#            Widelands update script.          #"
echo "################################################"
echo " "

set -e
if ! [ -f src/wlapplication.cc ] ; then
  echo "  This script must be run from the main directory of the widelands"
  echo "  source code."
  exit 1
fi

bzr pull
cd build
$buildtool
rm  ../VERSION || true
rm  ../widelands || true
mv VERSION ../VERSION
mv src/widelands ../widelands
cd ..

echo " "
echo "################################################"
echo "#      Widelands was updated successfully.     #"
echo "# You should be able to run it via ./widelands #"
echo "################################################"
END_SCRIPT
      chmod +x ./update.sh
      echo "  -> The update script has successfully been created."
  }
######################################



######################################
#    Here is the "main" function     #
######################################
set -e
basic_check
set_buildtool
prepare_directories_and_links
mkdir -p build
cd build
compile_widelands
move_built_files
cd ..
create_update_script
echo " "
echo "###########################################################"
echo "# Congratulations! Widelands has been built successfully  #"
echo "# with the following settings:                            #"
echo "#                                                         #"
if [ $BUILDTYPE = "Release" ]; then
  echo "# - Release build                                         #"
else
  echo "# - Debug build                                           #"
fi
if [ $BUILD_TRANSLATIONS = "ON" ]; then
  echo "# - Translations                                          #"
else
  echo "# - No translations                                       #"
fi

if [ $BUILD_WEBSITE = "ON" ]; then
  echo "# - Website-related executables                           #"
else
  echo "# - No website-related executables                        #"
fi
echo "#                                                         #"
echo "# You should now be able to run Widelands via             #"
echo "# typing ./widelands + ENTER in your terminal             #"
echo "#                                                         #"
echo "# You can update Widelands via running ./update.sh        #"
echo "# in the same directory that you ran this script in.      #"
echo "###########################################################"
######################################
