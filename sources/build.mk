SOURCE_DIRS	:=	sources 
SOURCE_DIRS	+=	sources/cores/$(CORE) 
SOURCE_DIRS	+=	sources/soc/$(SOC) 
SOURCE_DIRS	+=	sources/soc/$(SOC)/drivers/timer
SOURCE_DIRS	+=	sources/soc/$(SOC)/drivers/uart
SOURCE_DIRS	+=	sources/soc/$(SOC)/drivers/rtc
SOURCE_DIRS	+=	sources/target/$(TARGET)
SOURCE_DIRS	+=	sources/util

SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))
SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.s))
