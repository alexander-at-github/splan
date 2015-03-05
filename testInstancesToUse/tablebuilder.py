#!/usr/bin/python

import re, sys

def mySort(aList):
    #print("mySort")
    #print(aList)
    try:
        aList = sorted(list(map(int, aList)))
    except:
        #print("Exception")
        pass
    #print(aList)
    return aList

def first(list):
    if list and all( type(i) is int for i in list): # and len(list) > 0:
        return list[0]
    return '-'

def last(list):
    if list and all( type(i) is int for i in list): # and len(list) > 0:
        return list[-1]
    return '-'

def med(list):
    if not list or not all( type(i) is int for i in list): # or len(list) < 1:
        return '-'
    listLenH = int(len(list) / 2)
    if len(list) % 2 == 0:
        med = (list[listLenH - 1] + list[listLenH]) / 2
    else:
        med = list[listLenH]
    return med




if len(sys.argv) < 2:
    if len(sys.argv) < 1:
        sys.argv[0] = 'tablebuilder.py'
    print('Usage: ./{:s} <file>'.format(sys.argv[0]))
    raise SystemExit

with open(sys.argv[1]) as file:

    # Remove first two lines
    file.readline()
    file.readline()

    domain = ''
    numInst = 0
    numSol = 0
    ks = [] # a list of solution length for solved
    es = [] # a list of nodes expanded for solved
    ku = [] # a list of search depth for unsolved
    eu = [] # a list of nodes expanded for unsolved
    for line in file.readlines(): #lines:
        #if not line:
        #    continue
        #print(line)

        strs = line.split(' ')
        currDomain = strs[0].split('/')[8]
        #print(strs[0].split('/'))
        #print("currDomain: " + currDomain)

        if domain == '':
            domain = currDomain # Take care for first iteration
        elif currDomain != domain:
            print()
            #print("### currDomain != domain")
            #print(ks)
            #print(es)
            #print(ku)
            #print(eu)

            resultStr = "{:s} & " + \
                        "{:d} & " + \
                        "{:d} & " + \
                        "{:s} \\newline {:s} \\newline {:s} & " + \
                        "{:s} \\newline {:s} \\newline {:s} & " + \
                        "{:d} & " + \
                        "{:s} \\newline {:s} \\newline {:s} & " + \
                        "{:s} \\newline {:s} \\newline {:s} \\\\ \hline"
            ks = mySort(ks) #sorted(list(map(int, ks))) if ks else ks
            #print(ks)
            es = mySort(es) #sorted(list(map(int, es))) if es else es
            ku = mySort(ku) #sorted(list(map(int, ku))) if ku else ku
            eu = mySort(eu) #sorted(list(map(int, eu))) if eu else eu
            resultStr = resultStr.format(domain,
                             numInst,
                             numSol,
                             str(first(ks)), str(med(ks)), str(last(ks)),
                             str(first(es)), str(med(es)), str(last(es)),
                             numInst - numSol,
                             str(first(ku)), str(med(ku)), str(last(ku)),
                             str(first(eu)), str(med(eu)), str(last(eu)))
            print(resultStr)
            # Reset all the variables, cause we are looking at a new domain
            domain = currDomain
            numInst = 0
            numSol = 0
            ks = []
            es = []
            ku = []
            eu = []

        kk = strs[9]
        ee = strs[6]

        numInst += 1
        if strs[2] == 'S': # If solved
            numSol += 1
            ks.append(kk)
            es.append(ee)
        else:
            # Not solved
            ku.append(kk)
            eu.append(ee)

