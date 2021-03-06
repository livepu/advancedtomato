# Makefile for to build a gcc/uClibc toolchain
#
# Copyright (C) 2002-2003 Erik Andersen <andersen@uclibc.org>
# Copyright (C) 2004 Manuel Novoa III <mjn3@uclibc.org>
# Copyright (C) 2005-2006 Felix Fietkau <nbd@openwrt.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include $(TOPDIR)/rules.mk

PKG_NAME:=gcc
PKG_VERSION:=$(strip $(subst ",, $(CONFIG_GCC_VERSION)))#"))

ifeq ($(PKG_VERSION),3.4.6)
  PKG_MD5SUM:=4a21ac777d4b5617283ce488b808da7b
endif
ifeq ($(PKG_VERSION),4.1.2)
  PKG_MD5SUM:=a4a3eb15c96030906d8494959eeda23c
endif
ifeq ($(PKG_VERSION),4.2.4)
  PKG_MD5SUM:=d79f553e7916ea21c556329eacfeaa16
endif
ifeq ($(PKG_VERSION),4.3.5)
  PKG_MD5SUM:=e588cfde3bf323f82918589b94f14a15
endif
ifeq ($(PKG_VERSION),4.4.6)
  PKG_MD5SUM:=ab525d429ee4425050a554bc9247d6c4
endif

PATCH_DIR=./patches/$(PKG_VERSION)
PKG_SOURCE_URL:=ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-$(PKG_VERSION) \
	http://mirrors.rcn.net/pub/sourceware/gcc/releases/gcc-$(PKG_VERSION) \
	ftp://ftp.gnu.org/gnu/gcc/releases/gcc-$(PKG_VERSION)
PKG_SOURCE:=gcc-$(PKG_VERSION).tar.bz2
STAGING_DIR_HOST:=$(TOOLCHAIN_DIR)
BUILD_DIR_HOST:=$(BUILD_DIR_TOOLCHAIN)

include $(INCLUDE_DIR)/host-build.mk

STAMP_BUILT:=$(TOOLCHAIN_DIR)/stamp/.gcc-initial_installed
PKG_BUILD_DIR1:=$(PKG_BUILD_DIR)-initial
PKG_BUILD_DIR2:=$(PKG_BUILD_DIR)-final

SEP:=,
TARGET_LANGUAGES:="c$(if $(CONFIG_INSTALL_LIBSTDCPP),$(SEP)c++)$(if $(CONFIG_INSTALL_LIBGCJ),$(SEP)java)"

EXTRA_TARGET=$(if $(CONFIG_EXTRA_TARGET_ARCH),--enable-biarch --enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-uclibc)

export libgcc_cv_fixed_point=no

GCC_CONFIGURE:= \
	SHELL="$(BASH)" \
	$(PKG_BUILD_DIR)/configure \
		--prefix=$(TOOLCHAIN_DIR) \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--with-gnu-ld \
		--enable-target-optspace \
		--disable-libgomp \
		--disable-libmudflap \
		--disable-multilib \
		--disable-nls \
		--disable-__cxa_atexit \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(call qstrip,$(CONFIG_EXTRA_GCC_CONFIG_OPTIONS)) \

ifneq ($(CONFIG_GCC_VERSION_4_3)$(CONFIG_GCC_VERSION_4_4),)
  GCC_BUILD_TARGET_LIBGCC:=y
  GCC_CONFIGURE+= \
		--with-gmp=$(TOPDIR)/staging_dir/host \
		--with-mpfr=$(TOPDIR)/staging_dir/host \
		--disable-decimal-float
endif

ifneq ($(CONFIG_GCC_VERSION_4_4),)
  ifneq ($(CONFIG_mips)$(CONFIG_mipsel),)
    GCC_CONFIGURE += --with-mips-plt
  endif
endif

ifneq ($(CONFIG_SSP_SUPPORT),)
  GCC_CONFIGURE+= \
		--enable-libssp
else
  GCC_CONFIGURE+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
  GCC_CONFIGURE+= \
		--enable-biarch \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

GCC_CONFIGURE_STAGE1:= \
	$(GCC_CONFIGURE) \
		--with-newlib \
		--with-sysroot=$(BUILD_DIR_HOST)/uClibc_dev/ \
		--enable-languages=c \
		--disable-shared \
		--disable-threads \

GCC_CONFIGURE_STAGE2:= \
	$(GCC_CONFIGURE) \
		--enable-languages=$(TARGET_LANGUAGES) \
		--enable-shared \
		--enable-threads \

ifneq ($(CONFIG_TLS_SUPPORT),)
  GCC_CONFIGURE_STAGE2+= \
		--enable-tls
else
  GCC_CONFIGURE_STAGE2+= \
		--disable-tls
endif

GCC_MAKE:= \
	export SHELL="$(BASH)"; \
	$(MAKE) \
		CFLAGS_FOR_TARGET="$(TARGET_CFLAGS)" \
		CXXFLAGS_FOR_TARGET="$(TARGET_CFLAGS)"

define Build/SetToolchainInfo
	$(SED) 's,TARGET_CROSS=.*,TARGET_CROSS=$(REAL_GNU_TARGET_NAME)-,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,GCC_VERSION=.*,GCC_VERSION=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
endef


define Stage1/Configure
	mkdir -p $(PKG_BUILD_DIR1)
	(cd $(PKG_BUILD_DIR1); rm -f config.cache; \
		$(GCC_CONFIGURE_STAGE1) \
	);
endef

define Stage1/Compile
	$(GCC_MAKE) -C $(PKG_BUILD_DIR1) all-gcc \
	    $(if $(GCC_BUILD_TARGET_LIBGCC),all-target-libgcc)
endef

define Stage1/Install
	$(GCC_MAKE) -C $(PKG_BUILD_DIR1) install-gcc \
	    $(if $(GCC_BUILD_TARGET_LIBGCC),install-target-libgcc)

	# XXX: glibc insists on linking against libgcc_eh
	( cd $(TOOLCHAIN_DIR)/lib/gcc/$(REAL_GNU_TARGET_NAME)/$(PKG_VERSION) ; \
	    [ -e libgcc_eh.a ] || ln -sf libgcc.a libgcc_eh.a ; \
	    cp libgcc.a libgcc_initial.a; \
	)
endef

define Stage2/Configure
	mkdir -p $(PKG_BUILD_DIR2) $(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)
	# Important!  Required for limits.h to be fixed.
	rm -rf $(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/sys-include
	ln -sf ../include $(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/sys-include
	rm -rf $(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/lib
	ln -sf ../lib $(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/lib
	(cd $(PKG_BUILD_DIR2); rm -f config.cache; \
		$(GCC_CONFIGURE_STAGE2) \
	);
endef

define Stage2/Compile
	$(GCC_MAKE) -C $(PKG_BUILD_DIR2) all
endef

define SetupExtraArch
	for app in $(TOOLCHAIN_DIR)/bin/$(OPTIMIZE_FOR_CPU)*-{gcc,gcc-*,g++}; do \
		[ -e $$$$app ] || continue; \
		old_base=$$$$(basename $$$$app); \
		new_base=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-$$$${old_base##$(OPTIMIZE_FOR_CPU)-}; \
		sed -e "s/@CC_BASE@/$$$$old_base/" \
			-e 's/@EXTRA_ARCH_OPTS@/$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_OPTS))/' \
			 ./files/alternate-arch-cc.in > \
			 $(TOOLCHAIN_DIR)/bin/$$$$new_base; \
		chmod a+x $(TOOLCHAIN_DIR)/bin/$$$$new_base; \
	done
endef

define Stage2/Install
	$(GCC_MAKE) -C $(PKG_BUILD_DIR2) install
	# Set up the symlinks to enable lying about target name.
	set -e; \
	(cd $(TOOLCHAIN_DIR); \
		ln -sf $(REAL_GNU_TARGET_NAME) $(GNU_TARGET_NAME); \
		cd bin; \
		for app in $(REAL_GNU_TARGET_NAME)-* ; do \
			ln -sf $$$${app} \
		   	$(GNU_TARGET_NAME)$$$${app##$(REAL_GNU_TARGET_NAME)}; \
		done; \
	);
	$(if $(CONFIG_EXTRA_TARGET_ARCH),$(call SetupExtraArch))
	$(RSTRIP) $(TOOLCHAIN_DIR)/libexec/gcc/$(REAL_GNU_TARGET_NAME)/$(PKG_VERSION)
	-$(RSTRIP) $(TOOLCHAIN_DIR)/libexec/gcc/$(REAL_GNU_TARGET_NAME)/$(PKG_VERSION)/install-tools
endef



define Build/Prepare
	$(call Build/Prepare/Default)
	$(call Build/SetToolchainInfo)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(PKG_BUILD_DIR)/
	$(SED) 's,^MULTILIB_OSDIRNAMES,# MULTILIB_OSDIRNAMES,' $(PKG_BUILD_DIR)/gcc/config/*/t-*
	#(cd $(PKG_BUILD_DIR)/libstdc++-v3; autoconf;);
	$(SED) 's,gcc_no_link=yes,gcc_no_link=no,' $(PKG_BUILD_DIR)/libstdc++-v3/configure
endef

define Build/Configure
	$(call Stage1/Configure)
endef

define Build/Compile
	$(call Stage1/Compile)
	$(call Stage1/Install)
endef

define Build/Install
	$(call Stage2/Configure)
	$(call Stage2/Compile)
	$(call Stage2/Install)
endef

define Build/Clean
	rm -rf \
		$(PKG_BUILD_DIR) \
		$(PKG_BUILD_DIR1) \
		$(PKG_BUILD_DIR2) \
		$(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME) \
		$(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-gc* \
		$(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-c*
endef

$(eval $(call HostBuild))
