#!/bin/bash
# This script downloads parrot and sets it up to work with the ATLAS CVMFS

wget http://stash.osgconnect.net/parrot/parrot-sl6.tar.gz
tar -xvzf parrot-sl6.tar.gz
export HTTP_PROXY="squid.osgconnect.net:3128;http://uct2-grid1.uchicago.edu:3128;DIRECT"
export PARROT_HELPER="parrot/lib/libparrot_helper.so"
wget http://stash.osgconnect.net/keys/cern.ch.pub
./parrot/bin/parrot_run -r atlas.cern.ch:url=http://cvmfs.racf.bnl.gov:8000/opt/atlas,pubkey=cern.ch.pub,quota_limit=1000 /bin/bash -c 'source environment.sh; make; ./inspector ROOT-FILE'
hostname
