## SOURCE_DIRS	:=	applications/$(APP)
##
## SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))

SOURCES	+=	applications/$(APP)/$(APP).c