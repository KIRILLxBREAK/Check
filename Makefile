# This file is generated by tools/dmake, do not edit.

# To compile with rules, use 'make HAVE_RULES=yes'
ifndef HAVE_RULES
    HAVE_RULES=no
endif

# folder where lib/*.cpp files are located
ifndef SRCDIR
    SRCDIR=lib
endif

ifdef CFGDIR
    CFG=-DCFGDIR=\"$(CFGDIR)\"
else
    CFG=
endif

RDYNAMIC=-rdynamic
# Set the CPPCHK_GLIBCXX_DEBUG flag. This flag is not used in release Makefiles.
# The _GLIBCXX_DEBUG define doesn't work in Cygwin or other Win32 systems.
ifndef COMSPEC
    ifdef ComSpec
        #### ComSpec is defined on some WIN32's.
        COMSPEC=$(ComSpec)
    endif # ComSpec
endif # COMSPEC

ifdef COMSPEC
    #### Maybe Windows
    ifndef CPPCHK_GLIBCXX_DEBUG
        CPPCHK_GLIBCXX_DEBUG=
    endif # !CPPCHK_GLIBCXX_DEBUG

    ifeq ($(MSYSTEM),MINGW32)
        LDFLAGS=-lshlwapi
    else
        RDYNAMIC=-lshlwapi
    endif
else # !COMSPEC
    uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

    ifeq ($(uname_S),Linux)
        ifndef CPPCHK_GLIBCXX_DEBUG
            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG
        endif # !CPPCHK_GLIBCXX_DEBUG
    endif # Linux

    ifeq ($(uname_S),GNU/kFreeBSD)
        ifndef CPPCHK_GLIBCXX_DEBUG
            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG
        endif # !CPPCHK_GLIBCXX_DEBUG
    endif # GNU/kFreeBSD

endif # COMSPEC

# Set the UNDEF_STRICT_ANSI flag to address compile time warnings
# with tinyxml2 and Cygwin.
ifdef COMSPEC
    uname_S := $(shell uname -s)

    ifneq (,$(findstring CYGWIN,$(uname_S)))
        UNDEF_STRICT_ANSI=-U__STRICT_ANSI__
    endif # CYGWIN
endif # COMSPEC

ifndef CXX
    CXX=g++
endif

ifndef CXXFLAGS
    CXXFLAGS=-include lib/cxx11emu.h -pedantic -Wall -Wextra -Wabi -Wcast-qual -Wfloat-equal -Winline -Wmissing-declarations -Wmissing-format-attribute -Wno-long-long -Wpacked -Wredundant-decls -Wshadow -Wsign-promo -Wno-missing-field-initializers -Wno-missing-braces -Wno-sign-compare -Wno-multichar $(CPPCHK_GLIBCXX_DEBUG) -g
endif

ifeq ($(CXX), g++)
    override CXXFLAGS += -std=c++0x
else ifeq ($(CXX), clang++)
    override CXXFLAGS += -std=c++0x
else ifeq ($(CXX), c++)
    ifeq ($(shell uname -s), Darwin)
        override CXXFLAGS += -std=c++0x
    endif
endif

ifeq ($(HAVE_RULES),yes)
    override CXXFLAGS += -DHAVE_RULES -DTIXML_USE_STL $(shell pcre-config --cflags)
    ifdef LIBS
        LIBS += $(shell pcre-config --libs)
    else
        LIBS=$(shell pcre-config --libs)
    endif
endif

ifndef PREFIX
    PREFIX=/home/kirill/Загрузки/cppcheck-master
endif

ifndef INCLUDE_FOR_LIB
    INCLUDE_FOR_LIB=-Ilib -Iexternals/tinyxml
endif

ifndef INCLUDE_FOR_CLI
    INCLUDE_FOR_CLI=-Ilib -Iexternals/tinyxml
endif

BIN=$(DESTDIR)$(PREFIX)/bin

# For 'make man': sudo apt-get install xsltproc docbook-xsl docbook-xml on Linux
DB2MAN?=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl
XP=xsltproc -''-nonet -''-param man.charmap.use.subset "0"
MAN_SOURCE=man/cppcheck.1.xml


###### Object Files

LIBOBJ =      $(SRCDIR)/astutils.o \
              $(SRCDIR)/check.o \
              $(SRCDIR)/check64bit.o \
              $(SRCDIR)/checkassert.o \
              $(SRCDIR)/checkautovariables.o \
              $(SRCDIR)/checkbool.o \
              $(SRCDIR)/checkboost.o \
              $(SRCDIR)/checkbufferoverrun.o \
              $(SRCDIR)/checkclass.o \
              $(SRCDIR)/checkcondition.o \
              $(SRCDIR)/checkexceptionsafety.o \
              $(SRCDIR)/checkfunctions.o \
              $(SRCDIR)/checkgost.o \
              $(SRCDIR)/checkinternal.o \
              $(SRCDIR)/checkio.o \
              $(SRCDIR)/checkleakautovar.o \
              $(SRCDIR)/checkmemoryleak.o \
              $(SRCDIR)/checknullpointer.o \
              $(SRCDIR)/checkother.o \
              $(SRCDIR)/checkpostfixoperator.o \
              $(SRCDIR)/checksizeof.o \
              $(SRCDIR)/checkstl.o \
              $(SRCDIR)/checkstring.o \
              $(SRCDIR)/checktype.o \
              $(SRCDIR)/checkuninitvar.o \
              $(SRCDIR)/checkunusedfunctions.o \
              $(SRCDIR)/checkunusedvar.o \
              $(SRCDIR)/checkvaarg.o \
              $(SRCDIR)/cppcheck.o \
              $(SRCDIR)/errorlogger.o \
              $(SRCDIR)/library.o \
              $(SRCDIR)/mathlib.o \
              $(SRCDIR)/path.o \
              $(SRCDIR)/preprocessor.o \
              $(SRCDIR)/settings.o \
              $(SRCDIR)/suppressions.o \
              $(SRCDIR)/symboldatabase.o \
              $(SRCDIR)/templatesimplifier.o \
              $(SRCDIR)/timer.o \
              $(SRCDIR)/token.o \
              $(SRCDIR)/tokenize.o \
              $(SRCDIR)/tokenlist.o \
              $(SRCDIR)/valueflow.o

CLIOBJ =      cli/cmdlineparser.o \
              cli/cppcheckexecutor.o \
              cli/filelister.o \
              cli/main.o \
              cli/pathmatch.o \
              cli/threadexecutor.o \
              cli/process.o

ifndef TINYXML
    TINYXML = externals/tinyxml/tinyxml2.o
endif


EXTOBJ += $(TINYXML)
.PHONY: all clean instal


###### Targets

cppcheck: $(LIBOBJ) $(CLIOBJ) $(EXTOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o cppcheck $(CLIOBJ) $(LIBOBJ) $(EXTOBJ) $(LIBS) $(LDFLAGS) $(RDYNAMIC)

all:	cppcheck

check:	all
	./testrunner -g -q

checkcfg:	cppcheck
	./test/cfg/runtests.sh

clean:
	rm -f build/*.o lib/*.o cli/*.o externals/tinyxml/*.o cppcheck cppcheck.1

man:	man/cppcheck.1

man/cppcheck.1:	$(MAN_SOURCE)

	$(XP) $(DB2MAN) $(MAN_SOURCE)

tags:
	ctags -R --exclude=doxyoutput --exclude=test/cfg cli externals gui lib test

install: cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}
	install htmlreport/cppcheck-htmlreport ${BIN}
ifdef CFGDIR 
	install -d ${DESTDIR}${CFGDIR}
	install -m 644 cfg/* ${DESTDIR}${CFGDIR}
endif


###### Build

$(SRCDIR)/astutils.o: lib/astutils.cpp lib/cxx11emu.h lib/astutils.h lib/symboldatabase.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/astutils.o $(SRCDIR)/astutils.cpp

$(SRCDIR)/check.o: lib/check.cpp lib/cxx11emu.h lib/check.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/check.o $(SRCDIR)/check.cpp

$(SRCDIR)/check64bit.o: lib/check64bit.cpp lib/cxx11emu.h lib/check64bit.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/check64bit.o $(SRCDIR)/check64bit.cpp

$(SRCDIR)/checkassert.o: lib/checkassert.cpp lib/cxx11emu.h lib/checkassert.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkassert.o $(SRCDIR)/checkassert.cpp

$(SRCDIR)/checkautovariables.o: lib/checkautovariables.cpp lib/cxx11emu.h lib/checkautovariables.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkautovariables.o $(SRCDIR)/checkautovariables.cpp

$(SRCDIR)/checkbool.o: lib/checkbool.cpp lib/cxx11emu.h lib/checkbool.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/astutils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkbool.o $(SRCDIR)/checkbool.cpp

$(SRCDIR)/checkboost.o: lib/checkboost.cpp lib/cxx11emu.h lib/checkboost.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkboost.o $(SRCDIR)/checkboost.cpp

$(SRCDIR)/checkbufferoverrun.o: lib/checkbufferoverrun.cpp lib/cxx11emu.h lib/checkbufferoverrun.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/astutils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkbufferoverrun.o $(SRCDIR)/checkbufferoverrun.cpp

$(SRCDIR)/checkclass.o: lib/checkclass.cpp lib/cxx11emu.h lib/checkclass.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkclass.o $(SRCDIR)/checkclass.cpp

$(SRCDIR)/checkcondition.o: lib/checkcondition.cpp lib/cxx11emu.h lib/checkcondition.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/astutils.h lib/checkother.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkcondition.o $(SRCDIR)/checkcondition.cpp

$(SRCDIR)/checkexceptionsafety.o: lib/checkexceptionsafety.cpp lib/cxx11emu.h lib/checkexceptionsafety.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkexceptionsafety.o $(SRCDIR)/checkexceptionsafety.cpp

$(SRCDIR)/checkfunctions.o: lib/checkfunctions.cpp lib/cxx11emu.h lib/checkfunctions.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkfunctions.o $(SRCDIR)/checkfunctions.cpp

$(SRCDIR)/checkgost.o: lib/checkgost.cpp lib/cxx11emu.h lib/checkgost.h lib/check.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkgost.o $(SRCDIR)/checkgost.cpp
	
$(SRCDIR)/checkinternal.o: lib/checkinternal.cpp lib/cxx11emu.h lib/checkinternal.h lib/check.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkinternal.o $(SRCDIR)/checkinternal.cpp

$(SRCDIR)/checkio.o: lib/checkio.cpp lib/cxx11emu.h lib/checkio.h lib/check.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkio.o $(SRCDIR)/checkio.cpp

$(SRCDIR)/checkleakautovar.o: lib/checkleakautovar.cpp lib/cxx11emu.h lib/checkleakautovar.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/checkmemoryleak.h lib/symboldatabase.h lib/astutils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkleakautovar.o $(SRCDIR)/checkleakautovar.cpp

$(SRCDIR)/checkmemoryleak.o: lib/checkmemoryleak.cpp lib/cxx11emu.h lib/checkmemoryleak.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/astutils.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkmemoryleak.o $(SRCDIR)/checkmemoryleak.cpp

$(SRCDIR)/checknullpointer.o: lib/checknullpointer.cpp lib/cxx11emu.h lib/checknullpointer.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checknullpointer.o $(SRCDIR)/checknullpointer.cpp

$(SRCDIR)/checkother.o: lib/checkother.cpp lib/cxx11emu.h lib/checkother.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/astutils.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkother.o $(SRCDIR)/checkother.cpp

$(SRCDIR)/checkpostfixoperator.o: lib/checkpostfixoperator.cpp lib/cxx11emu.h lib/checkpostfixoperator.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkpostfixoperator.o $(SRCDIR)/checkpostfixoperator.cpp

$(SRCDIR)/checksizeof.o: lib/checksizeof.cpp lib/cxx11emu.h lib/checksizeof.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checksizeof.o $(SRCDIR)/checksizeof.cpp

$(SRCDIR)/checkstl.o: lib/checkstl.cpp lib/cxx11emu.h lib/checkstl.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/checknullpointer.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkstl.o $(SRCDIR)/checkstl.cpp

$(SRCDIR)/checkstring.o: lib/checkstring.cpp lib/cxx11emu.h lib/checkstring.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkstring.o $(SRCDIR)/checkstring.cpp

$(SRCDIR)/checktype.o: lib/checktype.cpp lib/cxx11emu.h lib/checktype.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checktype.o $(SRCDIR)/checktype.cpp

$(SRCDIR)/checkuninitvar.o: lib/checkuninitvar.cpp lib/cxx11emu.h lib/checkuninitvar.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/astutils.h lib/checknullpointer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkuninitvar.o $(SRCDIR)/checkuninitvar.cpp

$(SRCDIR)/checkunusedfunctions.o: lib/checkunusedfunctions.cpp lib/cxx11emu.h lib/checkunusedfunctions.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkunusedfunctions.o $(SRCDIR)/checkunusedfunctions.cpp

$(SRCDIR)/checkunusedvar.o: lib/checkunusedvar.cpp lib/cxx11emu.h lib/checkunusedvar.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkunusedvar.o $(SRCDIR)/checkunusedvar.cpp

$(SRCDIR)/checkvaarg.o: lib/checkvaarg.cpp lib/cxx11emu.h lib/checkvaarg.h lib/config.h lib/check.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/checkvaarg.o $(SRCDIR)/checkvaarg.cpp

$(SRCDIR)/cppcheck.o: lib/cppcheck.cpp lib/cxx11emu.h lib/cppcheck.h lib/config.h lib/settings.h lib/library.h lib/mathlib.h lib/standards.h lib/errorlogger.h lib/suppressions.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/tokenize.h lib/tokenlist.h lib/preprocessor.h lib/path.h lib/version.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/cppcheck.o $(SRCDIR)/cppcheck.cpp

$(SRCDIR)/errorlogger.o: lib/errorlogger.cpp lib/cxx11emu.h lib/errorlogger.h lib/config.h lib/suppressions.h lib/path.h lib/cppcheck.h lib/settings.h lib/library.h lib/mathlib.h lib/standards.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/tokenize.h lib/tokenlist.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/errorlogger.o $(SRCDIR)/errorlogger.cpp

$(SRCDIR)/library.o: lib/library.cpp lib/cxx11emu.h lib/library.h lib/config.h lib/mathlib.h lib/standards.h lib/errorlogger.h lib/suppressions.h lib/path.h lib/tokenlist.h lib/token.h lib/valueflow.h lib/symboldatabase.h lib/astutils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/library.o $(SRCDIR)/library.cpp

$(SRCDIR)/mathlib.o: lib/mathlib.cpp lib/cxx11emu.h lib/mathlib.h lib/config.h lib/errorlogger.h lib/suppressions.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/mathlib.o $(SRCDIR)/mathlib.cpp

$(SRCDIR)/path.o: lib/path.cpp lib/cxx11emu.h lib/path.h lib/config.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/path.o $(SRCDIR)/path.cpp

$(SRCDIR)/preprocessor.o: lib/preprocessor.cpp lib/cxx11emu.h lib/preprocessor.h lib/config.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/token.h lib/valueflow.h lib/mathlib.h lib/path.h lib/settings.h lib/library.h lib/standards.h lib/timer.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/preprocessor.o $(SRCDIR)/preprocessor.cpp

$(SRCDIR)/settings.o: lib/settings.cpp lib/cxx11emu.h lib/settings.h lib/config.h lib/library.h lib/mathlib.h lib/standards.h lib/errorlogger.h lib/suppressions.h lib/timer.h lib/preprocessor.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/settings.o $(SRCDIR)/settings.cpp

$(SRCDIR)/suppressions.o: lib/suppressions.cpp lib/cxx11emu.h lib/suppressions.h lib/config.h lib/path.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/suppressions.o $(SRCDIR)/suppressions.cpp

$(SRCDIR)/symboldatabase.o: lib/symboldatabase.cpp lib/cxx11emu.h lib/symboldatabase.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/symboldatabase.o $(SRCDIR)/symboldatabase.cpp

$(SRCDIR)/templatesimplifier.o: lib/templatesimplifier.cpp lib/cxx11emu.h lib/templatesimplifier.h lib/config.h lib/mathlib.h lib/token.h lib/valueflow.h lib/tokenize.h lib/errorlogger.h lib/suppressions.h lib/tokenlist.h lib/settings.h lib/library.h lib/standards.h lib/timer.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/templatesimplifier.o $(SRCDIR)/templatesimplifier.cpp

$(SRCDIR)/timer.o: lib/timer.cpp lib/cxx11emu.h lib/timer.h lib/config.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/timer.o $(SRCDIR)/timer.cpp

$(SRCDIR)/token.o: lib/token.cpp lib/cxx11emu.h lib/token.h lib/config.h lib/valueflow.h lib/mathlib.h lib/errorlogger.h lib/suppressions.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/token.o $(SRCDIR)/token.cpp

$(SRCDIR)/tokenize.o: lib/tokenize.cpp lib/cxx11emu.h lib/tokenize.h lib/errorlogger.h lib/config.h lib/suppressions.h lib/tokenlist.h lib/mathlib.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/path.h lib/symboldatabase.h lib/templatesimplifier.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/tokenize.o $(SRCDIR)/tokenize.cpp

$(SRCDIR)/tokenlist.o: lib/tokenlist.cpp lib/cxx11emu.h lib/tokenlist.h lib/config.h lib/token.h lib/valueflow.h lib/mathlib.h lib/path.h lib/preprocessor.h lib/settings.h lib/library.h lib/standards.h lib/errorlogger.h lib/suppressions.h lib/timer.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/tokenlist.o $(SRCDIR)/tokenlist.cpp

$(SRCDIR)/valueflow.o: lib/valueflow.cpp lib/cxx11emu.h lib/valueflow.h lib/config.h lib/astutils.h lib/errorlogger.h lib/suppressions.h lib/mathlib.h lib/settings.h lib/library.h lib/standards.h lib/timer.h lib/symboldatabase.h lib/token.h lib/tokenlist.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o $(SRCDIR)/valueflow.o $(SRCDIR)/valueflow.cpp

cli/cmdlineparser.o: cli/cmdlineparser.cpp lib/cxx11emu.h cli/cmdlineparser.h lib/cppcheck.h lib/config.h lib/settings.h lib/library.h lib/mathlib.h lib/standards.h lib/errorlogger.h lib/suppressions.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/tokenize.h lib/tokenlist.h cli/cppcheckexecutor.h cli/filelister.h lib/path.h cli/threadexecutor.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/cmdlineparser.o cli/cmdlineparser.cpp

cli/process.o: cli/process.cpp cli/process.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/process.o cli/process.cpp

cli/cppcheckexecutor.o: cli/cppcheckexecutor.cpp lib/cxx11emu.h cli/cppcheckexecutor.h cli/process.h lib/errorlogger.h lib/config.h lib/suppressions.h cli/cmdlineparser.h lib/cppcheck.h lib/settings.h lib/library.h lib/mathlib.h lib/standards.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/tokenize.h lib/tokenlist.h cli/filelister.h lib/path.h cli/pathmatch.h lib/preprocessor.h cli/threadexecutor.h lib/utils.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/cppcheckexecutor.o cli/cppcheckexecutor.cpp

cli/filelister.o: cli/filelister.cpp lib/cxx11emu.h cli/filelister.h lib/path.h lib/config.h cli/pathmatch.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/filelister.o cli/filelister.cpp

cli/main.o: cli/main.cpp lib/cxx11emu.h cli/cppcheckexecutor.h lib/errorlogger.h lib/config.h lib/suppressions.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/main.o cli/main.cpp

cli/pathmatch.o: cli/pathmatch.cpp lib/cxx11emu.h cli/pathmatch.h lib/path.h lib/config.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/pathmatch.o cli/pathmatch.cpp

cli/threadexecutor.o: cli/threadexecutor.cpp lib/cxx11emu.h cli/threadexecutor.h lib/errorlogger.h lib/config.h lib/suppressions.h lib/cppcheck.h lib/settings.h lib/library.h lib/mathlib.h lib/standards.h lib/timer.h lib/check.h lib/token.h lib/valueflow.h lib/tokenize.h lib/tokenlist.h cli/cppcheckexecutor.h
	$(CXX) ${INCLUDE_FOR_CLI} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o cli/threadexecutor.o cli/threadexecutor.cpp

externals/tinyxml/tinyxml2.o: externals/tinyxml/tinyxml2.cpp lib/cxx11emu.h externals/tinyxml/tinyxml2.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o externals/tinyxml/tinyxml2.o externals/tinyxml/tinyxml2.cpp

tools/dmake.o: tools/dmake.cpp lib/cxx11emu.h cli/filelister.h cli/pathmatch.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o tools/dmake.o tools/dmake.cpp

tools/reduce.o: tools/reduce.cpp lib/cxx11emu.h
	$(CXX) ${INCLUDE_FOR_LIB} $(CPPFLAGS) $(CFG) $(CXXFLAGS) $(UNDEF_STRICT_ANSI) -c -o tools/reduce.o tools/reduce.cpp
