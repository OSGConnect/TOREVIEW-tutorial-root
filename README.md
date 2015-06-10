OSG Connect ROOT Tutorial
=========================

Overview
--------
This application example will cover the use of the ROOT data analysis framework on OSG Connect. In this example, we'll use Parrot in order to access CVMFS on any worker node, regardless of whether or not it is natively mounted.
Background
----------
[ROOT](http://root.cern.ch/drupal/) is a piece of software commonly used in high energy physics. We'll use a sample piece of code (shamelessly stolen from [Ilija Vukotic](http://ivukotic.web.cern.ch/ivukotic/)) that prints all of the TTrees and their branches for a given ROOT file.
Testing ROOT on the submit host
-------------------------------
For this example, we're going to use ROOT in a manner similar to a typical ATLAS job. The first thing to do is set up our working directory for the tutorial, or simply run 'tutorial root'.
	[username@login01 ~]$ mkdir -p osg-root; cd osg-root
We'll need to run a few scripts to get the ROOT environment set up properly. This will add ROOT to our PATH and point LD_LIBRARY_PATH at the correct libraries. First, create the script *environment.sh*:
	#!/bin/bash
	export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
	source $ATLAS_LOCAL_ROOT_BASE/user/atlasLocalSetup.sh
	localSetupROOT --skipConfirm
Let's try running ROOT. We'll use the '-l' flag because we don't want ROOT's splash screen:
	[username@login01 root]$ source environment.sh
	Setting up gcc
	Setting up ROOT
	Setting up xRootD
	[username@login01 root]$ root -l
	*** DISPLAY not set, setting it to 10.150.25.138:0.0
	root [0]
There are some complaints about DISPLAY, but that's alright because we don't plan to do anything requiring X11 graphics. You can quit out of root with '.q'
	root [0] .q
	[username@login01 root]$
Running some ROOT code
----------------------
We're going to need some ROOT code, as well as a Makefile to compile it. Here is the ROOT code, in file *inspector.C*:
	// This piece of code has to get tree names, tree sizes, branch names, branch sizes.
	// Should be compiled.
	 
	#include <stdlib.h>
	 
	#include "Riostream.h"
	#include "TROOT.h"
	#include "TFile.h"
	#include "TNetFile.h"
	#include "TTree.h"
	#include "TTreeCache.h"
	#include "TBranch.h"
	#include "TClonesArray.h"
	#include "TStopwatch.h"
	#include "TKey.h"
	#include "TEnv.h"
	 
	#include <iostream>
	#include <fstream>
	#include <sstream>
	 
	using namespace std;
	 
	class mTree{
	public:
	    mTree(TTree *t){
	        name=t->GetName();
	        entries=(long)t->GetEntries();
	        totSize=t->GetZipBytes();
	        leaves=t->GetListOfBranches()->GetEntriesFast();
	        for (int i=0; i<leaves; i++) {
	            TBranch* branch = (TBranch*)t->GetListOfBranches()->UncheckedAt(i);
	            branch->SetAddress(0);
	            // cout <<i<<"\t"<<branch->GetName()<<"\t BS: "<< branch->GetBasketSize()<<"\t size: "<< branch->GetTotalSize()<< "\ttotbytes: "<<branch->GetTotBytes() << endl;
	            branchSizes.insert(std::pair<string,long>(branch->GetName(),branch->GetZipBytes()));
	        }
	    }
	    string name;
	    long entries;
	    long totSize;
	    int leaves;
	    map<string,long> branchSizes;// this is value of ZIPPED SIZES collected from all the files
	    void print(){
	        cout<<name<<":"<<entries<<":"<<totSize<<":"<<branchSizes.size()<<endl;
	        for(map<string,long>::iterator it = branchSizes.begin(); it != branchSizes.end(); it++){
	            cout<<it->first<<"\t"<<it->second<<endl;
	        }
	    }
	};
	 
	int main(int argc, char **argv){
	    if (argc<2) {
	        cout<<"usage: inpector <filename> "<<endl;
	        return 0;
	    }
	 
	    vector<mTree> m_trees;
	 
	    string fn = argv[1];
	    TFile *f = TFile::Open(fn.c_str());
	 
	    TIter nextkey( f->GetListOfKeys() );
	    TKey *key;
	    while ( (key = (TKey*)nextkey())) {
	        TObject *obj = key->ReadObj();
	        if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
	            TTree *tree = (TTree*)f->Get(obj->GetName());
	            int exist=0;
	            for(vector<mTree>::iterator i=m_trees.begin();i!=m_trees.end();i++)
	                if (obj->GetName()==(*i).name) exist=1;
	            if (!exist) m_trees.push_back(mTree(tree));
	        }
	    }
	    cout<<m_trees.size()<<endl;
	    for (vector<mTree>::iterator it = m_trees.begin();it != m_trees.end(); it++)
	        it->print();
	f->Close();
	    return 0;
	}
Here's the Makefile:

	RC     := root-config
	ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
	MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
	endif
	ifneq ($(MKARCH),)
	include $(MKARCH)
	else
	#include $(ROOTSYS)/test/Makefile.arch
	endif
	ALIBS = $(LIBS) -lTreePlayer
	#------------------------------------------------------------------------------
	INSPO       = inspector.$(ObjSuf)
	INSPS       = inspector.$(SrcSuf)
	INSP        = inspector$(ExeSuf)
	OBJS          = $(INSPO)
	PROGRAMS      = $(INSP)
	#------------------------------------------------------------------------------
	.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)
	all:            $(PROGRAMS)
	$(INSP):      $(INSPO)
	        $(LD) $(LDFLAGS) $^ $(ALIBS) $(OutPutOpt)$@
	        $(MT_EXE)
	        @echo "$@ done"
 
You can run "make" to create the executable inspector code.
	[username@login01 root]$ make
	g++ -O2 -Wall -fPIC -pthread -m64 -I/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.18-x86_64-slc6-gcc4.7/include   -c -o inspector.o inspector.C
	g++ -O2 -m64 inspector.o -L/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.18-x86_64-slc6-gcc4.7/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic  -lTreePlayer -o inspector
	inspector done
To try the code out, we'll first need an example ROOT file. A ROOT file is available at http://stash.osgconnect.net/+jenkins/ex1.root. Run the following command to retrieve it:
	[username@login01 root]$ wget -q --no-check-certificate http://stash.osgconnect.net/+jenkins/ex1.root
Now let's try our code out. We're going to be remotely reading data from the XRootD filesystem.

	[username@login01 root]$ ./inspector ex1.root | head -n10
	2
	CollectionTree:6290:132131:6
	EventNumber	17671
	RunNumber	248
	StreamAOD_ref	9131
	StreamESD_ref	50859
	StreamRDO_ref	9131
	Token		45091
	physics:6290:714067858:8543
	EF_2b35_loose_3j35_a4tchad_4L1J15	3498
Accessing software anywhere using Parrot
----------------------------------------
Suchandra has spoken a bit about Parrot for data access. I've written a bit of shell code to do that for you in the script *wrapper.sh*, shown below. Just like in the previous example, **ROOT-FILE** will need to be replaced with the location of a ROOT file. 
	#!/bin/bash
	# This script downloads parrot and sets it up to work with the ATLAS CVMFS
	 
	wget http://stash.osgconnect.net/parrot/parrot-sl6.tar.gz
	tar -xvzf parrot-sl6.tar.gz
	export HTTP_PROXY="squid.osgconnect.net:3128;http://uct2-grid1.uchicago.edu:3128;DIRECT"
	export PARROT_HELPER="parrot/lib/libparrot_helper.so"
	wget http://stash.osgconnect.net/keys/cern.ch.pub
	./parrot/bin/parrot_run -r atlas.cern.ch:url=http://cvmfs.racf.bnl.gov:8000/opt/atlas,pubkey=cern.ch.pub,quota_limit=1000 /bin/bash -c 'source environment.sh; make; ./inspector ROOT-FILE'
	hostname
Building an HTCondor Job
------------------------
Creating a job submit file for this code is pretty straightforward. The wrapper script does the bulk of the heavy lifting, we just have to make sure we are transferring the appropriate files. The requirements line is optional here, but I've included it because I'd like to see my job run on OSG. The source for *root.submit* follows:
	executable     = wrapper.sh
	universe       = vanilla
	 
	Error   = log/err.$(Cluster).$(Process)
	Output  = log/out.$(Cluster).$(Process)
	Log     = log/log.$(Cluster).$(Process)
	 
	transfer_executable = True
	transfer_input_files=inspector.C,Makefile,environment.sh
	when_to_transfer_output = ON_EXIT
	 
	requirements =isUndefined(GLIDECLIENT_Name) == FALSE
	 
	queue 1
Let's submit the code:
	[username@login01 root]$ condor_submit root.submit
	Submitting job(s).
	1 job(s) submitted to cluster 85995.
We can see that it's running:
	[username@login01 root]$ condor_q username
	
	
	-- Submitter: login01.osgconnect.net : <192.170.227.195:42546> : login01.osgconnect.net
	 ID      OWNER            SUBMITTED     RUN_TIME ST PRI SIZE CMD
	85995.0   username        5/20 13:16   0+00:00:05 R  0   0.0  wrapper.sh
	
	This code puts all of its output on stdout, so let's check the output:
	
	[username@login01 root]$ tail -n10 log/out.85995.0
	vx_m		  465260
	vx_n		  16759
	vx_nTracks	  223060
	vx_px		  480170
	vx_py		  480150
	vx_pz		  481488
	vx_sumPt	  465725
	vx_x		  481620
	vx_y		  444255
	vx_z		  499665
	c-110-34.aglt2.org
Success!
