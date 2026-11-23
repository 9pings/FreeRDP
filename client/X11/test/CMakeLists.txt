find_program(XVFB_RUN_EXECUTABLE xvfb-run)
if(XVFB_RUN_EXECUTABLE AND WITH_XRANDR AND X11_Xrandr_FOUND)
  add_executable(TestX11Monitor TestX11Monitor.c)
  target_link_libraries(TestX11Monitor PRIVATE xfreerdp-client ${X11_Xrandr_LIB})
  add_test(NAME TestX11Monitor COMMAND ${XVFB_RUN_EXECUTABLE} -a -s "-screen 0 3120x1920x24 -nolisten tcp"
                                       $<TARGET_FILE:TestX11Monitor>
  )
endif()

add_executable(TestX11DeferredAlt TestX11DeferredAlt.c)
target_link_libraries(TestX11DeferredAlt PRIVATE xfreerdp-client)
add_test(NAME TestX11DeferredAlt COMMAND TestX11DeferredAlt)

add_executable(TestX11CmdLine TestX11CmdLine.c)
target_link_libraries(TestX11CmdLine PRIVATE xfreerdp-client)
add_test(NAME TestX11CmdLine COMMAND TestX11CmdLine)

find_program(XVFB_RUN_EXECUTABLE xvfb-run)
add_executable(TestX11RailRestore TestX11RailRestore.c)
target_link_libraries(TestX11RailRestore PRIVATE xfreerdp-client)
if(XVFB_RUN_EXECUTABLE)
  add_test(NAME TestX11RailRestore COMMAND ${XVFB_RUN_EXECUTABLE} -a $<TARGET_FILE:TestX11RailRestore>)
  set_tests_properties(TestX11RailRestore PROPERTIES SKIP_RETURN_CODE 77)
endif()
