#
# source from Ralph Williamsen, 
# see http://stackoverflow.com/questions/4120001/what-is-the-git-equivalent-for-revision-number
#
# Set the source control revision similar to subversion to use in 'c'
# files as a define.
# You must build in the master branch otherwise the build branch will
# be prepended to the revision and/or "dirty" appended. This is to
# clearly ID developer builds. 
REPO_REVISION_:=$(shell git rev-list HEAD --count)
BUILD_BRANCH:=$(shell git rev-parse --abbrev-ref HEAD)
BUILD_REV_ID:=$(shell git rev-parse HEAD)
BUILD_REV_ID_SHORT:=$(shell git describe --long --tags --dirty --always)
ifeq ($(BUILD_BRANCH), master)
REPO_REVISION:=$(REPO_REVISION_)_g$(BUILD_REV_ID_SHORT)
else
REPO_REVISION:=$(BUILD_BRANCH)_$(REPO_REVISION_)_r$(BUILD_REV_ID_SHORT)
endif
export REPO_REVISION
export BUILD_BRANCH
export BUILD_REV_ID

