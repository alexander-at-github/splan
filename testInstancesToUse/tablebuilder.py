#!/usr/bin/python

import decimal, re, sys

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

def rmZeros(num):
    string = str(num)
    if string == '-':
        pass
    else:
        #string = str(decimal.Decimal(string).normalize())
        string = string.rstrip('0').rstrip('.') if '.' in string else string
    #print(num)
    #print(string)
    return string

def printResultString(domain, numIst, numSol, vos, ks, es, vou, ku, eu):
    #print(ks)
    #print(es)
    #print(ku)
    #print(eu)

    # domain name
    # total number of instances
    # number of solved instances
    # solution length k
    # variable occurence vo for solved
    # number of expanded nodes for solved
    # number of unsolved instances
    # search depth unsolved
    # variable occurence vo for unsolved
    # number of expanded nodes for unsolved
    resultStr = "{:s} & " + \
                "{:d} & " + \
                "{:d} & " + \
                "{:s} \\newline {:s} \\newline {:s} & " + \
                "{:s} \\newline {:s} \\newline {:s} & " + \
                "{:s} \\newline {:s} \\newline {:s} & " + \
                "{:d} & " + \
                "{:s} \\newline {:s} \\newline {:s} & " + \
                "{:s} \\newline {:s} \\newline {:s} & " + \
                "{:s} \\newline {:s} \\newline {:s} \\\\ \hline"

    vos = mySort(vos)
    ks = mySort(ks) #sorted(list(map(int, ks))) if ks else ks
    #print(ks)
    es = mySort(es) #sorted(list(map(int, es))) if es else es

    # We remove the instances which crashed (out of mem) for the parameter vo.
    # What should we do with it otherwise?
    vou = [v for v in vou if v.isdigit()]

    vou = mySort(vou)
    ku = mySort(ku) #sorted(list(map(int, ku))) if ku else ku
    euFiltered = [i for i in eu if i.isdigit()]
    if len(eu) != len(euFiltered):
        print("\n eu - euFiltered: " + str(len(eu) - len(euFiltered)))
    euFiltered = [i if i.isdigit() else '0' for i in eu]
    euFiltered = mySort(euFiltered) #sorted(list(map(int, eu))) if eu else eu

    #print(vou)
    #print(first(vou))
    #print(med(vou))
    #print(last(vou))

    resultStr = resultStr.format(domain,
                     numInst,
                     numSol,
                     rmZeros(first(ks)), rmZeros(med(ks)), rmZeros(last(ks)),
                     rmZeros(first(vos)), rmZeros(med(vos)), rmZeros(last(vos)),
                     rmZeros(first(es)), rmZeros(med(es)), rmZeros(last(es)),
                     numInst - numSol,
                     rmZeros(first(ku)), rmZeros(med(ku)), rmZeros(last(ku)),
                     rmZeros(first(vou)), rmZeros(med(vou)), rmZeros(last(vou)),
                     rmZeros(first(euFiltered)), rmZeros(med(euFiltered)), rmZeros(last(euFiltered)))
    print()
    print(resultStr)



if len(sys.argv) < 2:
    if len(sys.argv) < 1:
        sys.argv[0] = 'tablebuilder.py'
    print('Usage: ./{:s} <file>'.format(sys.argv[0]))
    raise SystemExit

with open(sys.argv[1]) as file:

    # Write filename to output
    print("% These are the results from " + sys.argv[1])

    # Remove first two lines
    file.readline()
    file.readline()

    domain = ''
    numInst = 0
    numSol = 0
    vos = [] # a list of variable occurence for solved
    ks = [] # a list of solution length for solved
    es = [] # a list of nodes expanded for solved
    vou = [] # a list of variable occurence for unsolved
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
            #print("### currDomain != domain")

            # Print the domain data
            printResultString(domain, numInst, numSol, vos, ks, es, vou, ku, eu)

            # Reset all the variables, cause we are looking at a new domain
            domain = currDomain
            numInst = 0
            numSol = 0
            vos = []
            ks = []
            es = []
            vou = []
            ku = []
            eu = []

        vo = strs[8]
        kk = strs[9]
        ee = strs[6]

        numInst += 1
        if strs[2] == 'S': # If solved
            numSol += 1
            vos.append(vo)
            ks.append(kk)
            es.append(ee)
        else:
            # Not solved
            vou.append(vo)
            ku.append(kk)
            eu.append(ee)

    # Finaly print the last domain
    printResultString(domain, numInst, numSol, vos, ks, es, vou, ku, eu)

