SOURCE_DIRS	:=	$(realpath .)

SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))
