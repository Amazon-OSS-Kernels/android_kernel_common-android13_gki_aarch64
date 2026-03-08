mkdir uboot
cd uboot
##
#step 1 Download repos
##

##
#step 1-1 prepare toolchain
##

git clone "ssh://gerrit6.labcollab.net:9418/vendor/mediatek/dtv/prebuilts/mtk_toolchain/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu" ./toolchain
cd toolchain
git checkout remotes/origin/fireos/main/8
cd -


UBOOTTOOL=$(realpath ./toolchain/bin/)
export PATH=$UBOOTTOOL:$PATH

##
#step 1-2 prepare ufbl-features
##

git clone "ssh://gerrit6.labcollab.net:9418/platform/external/libtomcrypt" ./bootable/bootloader/libtomcrypt
cd ./bootable/bootloader/libtomcrypt
git checkout remotes/origin/fireos/main/8
cd -



git clone "ssh://gerrit6.labcollab.net:9418/platform/external/libtommath"  ./bootable/bootloader/libtommath
cd ./bootable/bootloader/libtommath
git checkout remotes/origin/fireos/main/8
cd -

git clone "ssh://gerrit6.labcollab.net:9418/platform/bootable/bootloader/ufbl-features"  ./bootable/bootloader/ufbl-features
cd ./bootable/bootloader/ufbl-features
git checkout remotes/origin/fireos/main/8
cd features/libtomcrypt
rm src
ln -s ../../../libtomcrypt/src/ src
cd -

cd features/libtommath/
rm src
ln -s ../../../libtommath/ src
cd -

cd ../../../

##
#step 1-3 prepare u-boot-2019
##


git clone "ssh://gerrit6.labcollab.net:9418/vendor/mediatek/mt9026/dtv/bootloader/u-boot-2019.04"  
cd u-boot-2019.04
git checkout remotes/origin/fireos/main/8


git clone "ssh://gerrit6.labcollab.net:9418/vendor/mediatek/mt9026/dtv/bootloader/Mediatek-internal" 
cd Mediatek-internal
git checkout remotes/origin/fireos/main/8
cd ..



git clone "ssh://gerrit6.labcollab.net:9418/vendor/mediatek/mt9026/dtv/bootloader/Customization"  ./Customization/Mediatek
cd ./Customization/Mediatek
git checkout remotes/origin/fireos/main/8
cd -


cd amazon/include/

ln -s ../security/unlock_pub_key.h


cd -


##
#step 2 # Build Uboot, start from folder "u-boot-2019.04"
##


##
#step 2-1 # modify UFBL path in Makefile.ufbl 
##


# modify UFBL path in Makefile.ufbl
UFBL_PATH       := $(ANDROID_TOP)/bootable/bootloader/ufbl-features
-->
UFBL_PATH       := $(PWD)/../bootable/bootloader/ufbl-features


##
#step 2-2 # gerente .config
##


make MT5896_defconfig

##
#step 2-3 # modify the .config according to the  Android.bp
##


# CONFIG_DIAG_TRANSITION_DIALOG is not set
-->
CONFIG_DIAG_TRANSITION_DIALOG=y


# CONFIG_ROLLBACK_INDEX_IN_EFUSE is not set
-->
CONFIG_ROLLBACK_INDEX_IN_EFUSE=y

# CONFIG_ROLLBACK_INDEX_IN_RPMB is not set
-->
#CONFIG_ROLLBACK_INDEX_IN_RPMB=y

##
#step 2-4 # make the target
##


make 
