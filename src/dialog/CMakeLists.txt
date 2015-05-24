cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH CMakeModules)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
#find_package(ZLIB REQUIRED)
#find_package(BZip2 REQUIRED)
#find_package(OpenSSL QUIET)
#find_package(Lzma QUIET)

#CHECK_FUNCTION_EXISTS (fstat64           HAVE_FSTAT64)
#CHECK_FUNCTION_EXISTS (stat64            HAVE_STAT64)
#CHECK_FUNCTION_EXISTS (lstat64           HAVE_LSTAT64)

set(shaback_dialog_SRCS
		argv.c
		arrows.c
		calendar.c
		checklist.c
		columns.c
		dialog.c
		dlg_keys.c
		editbox.c
		formbox.c
		fselect.c
		gauge.c
		help.c
		inputbox.c
		inputstr.c
		menubox.c
		mixedform.c
		mixedgauge.c
		mouse.c
		mousewget.c
		msgbox.c
		pause.c
		prgbox.c
		progressbox.c
		rc.c
		tailbox.c
		textbox.c
		timebox.c
		trace.c
		ui_getc.c
		util.c
		version.c
		yesno.c
		     )


add_library(shabackdialog STATIC ${shaback_dialog_SRCS})

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#  "config.h.in"
#  "config.h"
#  )
