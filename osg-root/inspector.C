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

