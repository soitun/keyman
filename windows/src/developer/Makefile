#
# Keyman dev Makefile
#


TARGETS=kmcmpdll kmcomp kmconvert tike samples setup inst
MANIFESTS=kmcomp tike setup

!include ..\Header.mak

# ----------------------------------------------------------------------

imsample:
    cd $(ROOT)\src\developer\imsample
    $(MAKE) $(TARGET)

kmcmpdll:
    cd $(ROOT)\src\developer\kmcmpdll
    $(MAKE) $(TARGET)

kmcomp: kmcmpdll
    cd $(ROOT)\src\developer\kmcomp
    $(MAKE) $(TARGET)

kmconvert:
    cd $(ROOT)\src\developer\kmconvert
    $(MAKE) $(TARGET)

tike: kmcmpdll
    cd $(ROOT)\src\developer\tike
    $(MAKE) $(TARGET)

samples:
    cd $(ROOT)\src\developer\samples
    $(MAKE) $(TARGET)

setup:
    cd $(ROOT)\src\developer\setup
    $(MAKE) $(TARGET)

inst:
    cd $(ROOT)\src\developer\inst
    $(MAKE) $(TARGET)

!include ..\Target.mak

# ----------------------------------------------------------------------
# EOF
# ----------------------------------------------------------------------
