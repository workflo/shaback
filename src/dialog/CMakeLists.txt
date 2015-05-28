cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH CMakeModules)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

#CHECK_FUNCTION_EXISTS (fstat64           HAVE_FSTAT64)
#CHECK_FUNCTION_EXISTS (stat64            HAVE_STAT64)
#CHECK_FUNCTION_EXISTS (lstat64           HAVE_LSTAT64)

IF (NCURSES_FOUND)
	set(shaback_dialog_SRCS
		argv.cpp
		arrows.cpp
		buttons.cpp
		calendar.cpp
		checklist.cpp
		columns.cpp
		dialog.cpp
		dlg_keys.cpp
		editbox.cpp
		formbox.cpp
		fselect.cpp
		gauge.cpp
		help.cpp
		inputbox.cpp
		inputstr.cpp
		menubox.cpp
		mixedform.cpp
		mixedgauge.cpp
		mouse.cpp
		mousewget.cpp
		msgbox.cpp
		pause.cpp
		prgbox.cpp
		progressbox.cpp
		rc.cpp
		tailbox.cpp
		textbox.cpp
		timebox.cpp
		trace.cpp
		ui_getc.cpp
		util.cpp
		version.cpp
		yesno.cpp
    )
ELSE (NCURSES_FOUND)
	set(shaback_dialog_SRCS
		.empty.cpp
	)
ENDIF (NCURSES_FOUND)

add_library(shabackdialog STATIC ${shaback_dialog_SRCS})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "dlg_config.h.in"
    "dlg_config.h"
)