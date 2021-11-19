#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := sample_project
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/esp-rainmaker/components $(PROJECT_PATH)/esp-rainmaker/common
include $(IDF_PATH)/make/project.mk
