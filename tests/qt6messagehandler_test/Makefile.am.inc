qt6messagehandler_test_cppflags = \
	@QT6_CFLAGS@
qt6messagehandler_test_ldflags = \
	@QT6_LIBS@

qt6messagehandler_test_LDADD += $(top_builddir)/liblog4cplusqt6debugappender.la
qt6messagehandler_test_CPPFLAGS += $(qt6messagehandler_test_cppflags)
qt6messagehandler_test_LDFLAGS += $(qt6messagehandler_test_ldflags)
if BUILD_WITH_WCHAR_T_SUPPORT
qt6messagehandler_testU_CPPFLAGS += $(qt6messagehandler_test_cppflags)
qt6messagehandler_testU_LDFLAGS += $(qt6messagehandler_test_ldflags)
qt6messagehandler_testU_LDADD += $(top_builddir)/liblog4cplusqt6debugappenderU.la
endif # BUILD_WITH_WCHAR_T_SUPPORT
