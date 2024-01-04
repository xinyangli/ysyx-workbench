#/usr/bin/env bash
#
# Environment Variables:
#   NEMU_HOME
set -x

STUID=ysyx_22040000
STUNAME=李心杨

TRACER=tracer-ysyx
GITFLAGS="-q --author=$TRACER<tracer@ysyx.org> --no-verify --allow-empty"

YSYX_HOME=$NEMU_HOME/..
WORK_BRANCH=$(git rev-parse --abbrev-ref HEAD)
WORK_INDEX=$YSYX_HOME/.git/index.${WORK_BRANCH}
TRACER_BRANCH=$TRACER

LOCK_DIR=$YSYX_HOME/.git/

git_soft_checkout () {
    git checkout --detach -q && git reset --soft $1 -q -- && git checkout $1 -q -- ;
}

git_commit () {
    # create tracer branch if not existent
    git branch $TRACER_BRANCH -q 2>/dev/null || true
    # backup git index
    cp -a .git/index $WORK_INDEX
    # switch to tracer branch
    git_soft_checkout "$TRACER_BRANCH"
    # add files to commit
    git add . -A --ignore-errors
    # generate commit msg, commit changes in tracer branch
    printf "> $1 \n $STUID $STUNAME \n $(uname -a) \n $(uptime)\n" | git commit -F - $GITFLAGS
    git_soft_checkout "$WORK_BRANCH"
    mv $WORK_INDEX .git/index
}

git_commit $1

# TIMEOUT=2
# LOCKFILE=$YSYX_HOME/.git/index.lock
# exec {FD}<>$LOCKFILE

# if ! flock -x -w $TIMEOUT $FD; then
#   echo "Failed to obtain a lock within $TIMEOUT seconds"
#   echo "Another instance of `basename $0` is probably running."
#   exit 1
# else
#   echo "Lock acquired"
#   git_commit $1
#   rm -f $WORK_INDEX
#   exit 0
# fi

