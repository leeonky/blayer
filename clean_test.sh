#! /bin/bash

git_repo=$(git remote -v | grep fetch | awk '{sub("git@github.com:", "https://github.com/", $2); print $2}')

sudo docker run --rm -ti daocloud.io/leeonky/ctest runtest https://github.com/leeonky/blayer.git
